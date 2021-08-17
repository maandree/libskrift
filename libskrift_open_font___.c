/* See LICENSE file for copyright and license details. */
#include "common.h"

#ifdef GUNZIP_PATH
# define IS_GZIP() (size >= 18 && mem8[0] == 0x1f && mem8[1] == 0x8b)
#endif

#define IS_PSF_V1() (size >= 4 && mem8[0] == 0x36 && mem8[1] == 0x04)
#define IS_PSF_V2() (size >= 32 && mem8[0] == 0x72 && mem8[1] == 0xb5 && mem8[2] == 0x4a && mem8[3] == 0x86) 

#if defined(GUNZIP_PATH)
static int
decompress(const char *path, const void *mem, size_t size, void **memp, size_t *sizep)
{
	const char *mem_in = mem;
	char *mem_out = *memp, *new;
	size_t off_in = 0, off_out = 0;
	int i, n, mem_fds[2][2], err_fds[2], saved_errno;
	struct pollfd pfds[2];
	nfds_t npfds;
	int status;
	pid_t pid;
	ssize_t r;

	if (pipe(mem_fds[0]))
		return -1;

	if (pipe(mem_fds[1])) {
		saved_errno = errno;
		close(mem_fds[0][0]);
		close(mem_fds[0][1]);
		errno = saved_errno;
		return -1;
	}

	if (pipe2(err_fds, O_CLOEXEC)) {
		saved_errno = errno;
		close(mem_fds[0][0]);
		close(mem_fds[0][1]);
		close(mem_fds[1][0]);
		close(mem_fds[1][1]);
		errno = saved_errno;
		return -1;
	}

	switch ((pid = fork())) {
	case -1:
		saved_errno = errno;
		close(mem_fds[0][0]);
		close(mem_fds[0][1]);
		close(mem_fds[1][0]);
		close(mem_fds[1][1]);
		close(err_fds[0]);
		close(err_fds[1]);
		errno = saved_errno;
		return -1;

	case 0:
		close(err_fds[0]);
		for (i = 0; i < 2; i++) {
			if (mem_fds[i][i] != i) {
				close(i);
			dup2_again:
				if (dup2(mem_fds[i][i], i) != i) {
					if (errno == EINTR)
						goto dup2_again;
					write(err_fds[1], &errno, sizeof(errno));
					return 255;
				}
				close(mem_fds[i][i]);
			}
			close(mem_fds[i][i ^ 1]);
		}
		execl(path, path, NULL);
		write(err_fds[1], &errno, sizeof(errno));
		return 255;

	default:
		break;
	}

	close(err_fds[1]);
	close(mem_fds[0][0]);
	close(mem_fds[1][1]);

	memset(pfds, 0, sizeof(pfds));
	pfds[0].fd = mem_fds[1][0];
	pfds[1].fd = mem_fds[0][1];
	pfds[0].events = POLLIN;
	pfds[1].events = POLLOUT;
	npfds = 2;
	for (;;) {
		n = poll(pfds, npfds, -1);
		if (n <= 0) {
			if (!n)
				continue;
		poll_loop_fail:
			saved_errno = errno;
			close(err_fds[1]);
			if (mem_fds[0][1] >= 0)
				close(mem_fds[0][1]);
			close(mem_fds[1][0]);
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
			free(mem_out);
			errno = saved_errno;
			return -1;
		}
		if (pfds[0].revents) {
			if (*sizep == off_out) {
				if (*sizep > SIZE_MAX - 4096) {
					errno = ENOMEM;
					goto poll_loop_fail;
				}
				new = realloc(mem_out, *sizep += 4096);
				if (!new)
					goto poll_loop_fail;
				mem_out = new;
			}
		read_again:
			r = read(pfds[0].fd, &mem_out[off_out], *sizep - off_out);
			if (r <= 0) {
				if (!r)
					break;
				if (errno == EINTR)
					goto read_again;
				goto poll_loop_fail;
			}
			off_out += (size_t)r;
		}
		if (npfds > 1 && pfds[1].revents) {
			if (off_in == size) {
				npfds -= 1;
				close(mem_fds[0][1]);
				mem_fds[0][1] = -1;
				continue;
			}
write_again:
			r = write(pfds[1].fd, &mem_in[off_in], MIN(size - off_in, (size_t)1 << 16));
			if (r <= 0) {
				if (r < 0 && errno == EINTR)
					goto write_again;
				goto poll_loop_fail;
			}
			off_in += (size_t)r;
		}
	}

	if (mem_fds[0][1] >= 0)
		close(mem_fds[0][1]);
	close(mem_fds[1][0]);

waitpid_again:
	if (waitpid(pid, &status, 0) != pid) {
		if (errno == EINTR)
			goto waitpid_again;
		saved_errno = errno;
		close(err_fds[0]);
		free(mem_out);
		errno = saved_errno;
	}

	if (WIFEXITED(status) && WEXITSTATUS(status) == 255) {
		if (read(err_fds[1], &errno, sizeof(errno)) == sizeof(errno)) {
			goto child_failed;
		} else {
			errno = EIO;
			goto child_failed;
		}
	} else if (status) {
		errno = WIFEXITED(status) ? EBFONT : EPIPE;
		goto child_failed;
	} else if (off_in < size) {
		errno = EPIPE;
	child_failed:
		saved_errno = errno;
		close(err_fds[0]);
		free(mem_out);
		errno = saved_errno;
		return -1;
	}

	close(err_fds[0]);

	if (*sizep > off_out) {
		new = realloc(mem_out, off_out);
		if (new)
			mem_out = new;
	}

	*memp = mem_out;
	*sizep = off_out;
	return 0;
}
#endif

int
libskrift_open_font___(LIBSKRIFT_FONT **fontp, const void *mem_static, void *mem_free, void *mem_unmap, size_t size)
{
	const void *mem = mem_static ? mem_static : mem_free ? mem_free : mem_unmap;
	const uint8_t *mem8 = mem;
	void *new_mem = NULL;
#if defined(GUNZIP_PATH)
	size_t new_size = 0;
#endif

	*fontp = calloc(1, sizeof(**fontp));
	if (!*fontp)
		return -1;
	(*fontp)->refcount = 1;

#if defined(GUNZIP_PATH)
	if (IS_GZIP()) {
		if (decompress(GUNZIP_PATH, mem, size, &new_mem, &new_size))
			return -1;
		if (mem_free)
			free(mem_free);
		else if (mem_unmap)
			munmap(mem_unmap, size);
		mem_static = NULL;
		mem_unmap = NULL;
		mem8 = mem = mem_free = new_mem;
		size = new_size;
	}
#endif

	if (IS_PSF_V1() || IS_PSF_V2()) { /* TODO */
		errno = EBFONT;
		return -1;
	} else {
		(*fontp)->font_type = FONT_TYPE_SCHRIFT;
		(*fontp)->font.schrift = sft_loadmem(mem, size);
	}

	if (!(*fontp)->font.any) {
		free(*fontp);
		*fontp = NULL;
		return -1;
	}

	if (mem_free) {
		(*fontp)->memory_free = mem_free;
		(*fontp)->memory_size = size;
	} else if (mem_unmap) {
		(*fontp)->memory_unmap = mem_unmap;
		(*fontp)->memory_size = size;
	}

	return 0;
}

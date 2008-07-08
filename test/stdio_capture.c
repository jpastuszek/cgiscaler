/***************************************************************************
 *   Copyright (C) 2007, 2008 by Jakub Pastuszek   *
 *   jpastuszek@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "config.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "stdio_capture.h"
#include "../cgreen/cgreen.h"
#include "asserts.h"

/* some serving specific helpers */

int orginal_stdout = 0;
int orginal_stderr = 0;

/** Captures stdout stream in to pipe.
* @param fd_to_capture file descriptor to be captured in to pipe - this will normally be stdout or stderr
* @return pipe end that will receive stdout stream
* @see restore_stdout()
**/
int capture_fd(int fd_to_capture) {
	int p[2];

	assert_not_equal(pipe(p), -1);

	assert_not_equal(close(fd_to_capture), -1);
	/* use write pipe end as stdout */
	assert_not_equal(dup2(p[1], fd_to_capture), -1);

	/* closing original write pipe end as we have it duplicated */
	assert_not_equal(close(p[1]), -1);

	/* return read pipe end */
	return p[0];
}

/** Restores fd_to_restore with orginal_fd.
* @param fd_to_restore file descriptor that captured_fd is now capturing
* @param orginal_fd file descriptor that was originally used for captured stream
* @see capture_fd()
**/
void restore_fd(int fd_to_restore, int orginal_fd) {
	/* close write pipe end */
	assert_not_equal(close(fd_to_restore), -1);
	assert_not_equal(dup2(orginal_fd, fd_to_restore), -1);
}

/** Intercepts stdout stream to file descriptor.
* Use restore_stdout() to restore stdout to previous state.
* @return file descriptor that will receive stdout stream
* @see restore_stdout()
**/
int capture_stdout() {
	/* saving original stdout */
	orginal_stdout = dup(1);

	return capture_fd(1);
}

/** Restores original stdout stream after capture_stdout() call.
* @see capture_stdout()
**/
void restore_stdout() {
	restore_fd(1, orginal_stdout);
}

/** Intercepts stderr stream to file descriptor.
* Use restore_stderr() to restore stderr to previous state.
* @return file descriptor that will receive stderr stream
* @see restore_stderr()
**/
int capture_stderr() {
	/* saving original stdout */
	orginal_stderr = dup(2);

	return capture_fd(2);
}

/** Restores original stderr stream after capture_stderr() call.
* @see capture_stderr()
**/
void restore_stderr() {
	restore_fd(2, orginal_stderr);
}

/** This function will fork and redirect child's stdout to stdout_fd pipe end.
* stdout_fd needs to be freed with finish_fork() witch will also wait until child exists
* @param stdout_fd provided pointer will receive file discriptor of a pipe that will receive stdout stream
* @return 0 for child and 1 for parent
* @see finish_fork()
**/
int fork_with_stdout_capture(int *stdout_fd) {
	*stdout_fd = capture_stdout();
	if(!fork())
		return 0;

	/* We restore our local stdout */
	restore_stdout();
	return 1;
}

/** Call this after fork_with_stdout_capture() call to close capture file descriptor and wait for child exit.
* @param stdout_fd file descriptor that was assigned by fork_with_stdout_capture() call
* @see fork_with_stdout_capture()
**/
void finish_fork_with_stdout_capture(int stdout_fd) {
	int status;
	char buf[1024];

	/* we read what is left to read so that child will not hang on writing to the pipe */
	while (read(stdout_fd, buf, 1024) > 0);

	/* we will wait for child to finish */
	wait(&status);

	/* and close stdout redirect pipe */
	close(stdout_fd);
}

/** This function will fork and redirect child's stdout and stderr to stdout_fd and stderr_fd file descriptors respectively.
* stdout_fd and stderr_fd needs to be freed with finish_fork_with_stdout_and_stderr_capture() witch will also wait until child exists.
* @param stdout_fd provided pointer will receive file discriptor of a pipe that will receive stdout stream
* @param stderr_fd provided pointer will receive file descriptor of a pipe that will receive stderr stream
* @return 0 for child and 1 for parent
* @see finish_fork()
**/
int fork_with_stdout_and_stderr_capture(int *stdout_fd, int *stderr_fd) {
	*stdout_fd = capture_stdout();
	*stderr_fd = capture_stderr();
	if(!fork())
		return 0;

	/* We restore our local stdout */
	restore_stderr();
	restore_stdout();
	return 1;
}

/** Call this after fork_with_stdout_and_stderr_capture() call to close capture file descriptor and wait for child exit.
* @param stdout_fd stdout_fd file descriptor that was assigned by fork_with_stdout_and_stderr_capture() call
* @param stderr_fd stderr_fd file descriptor that was assigned by fork_with_stdout_and_stderr_capture() call
* @see fork_with_stdout_and_stderr_capture()
**/
void finish_fork_with_stdout_and_stderr_capture(int stdout_fd, int stderr_fd) {
	int status;
	char buf[1024];

	/* we read what is left to read so that child will not hang on writing to the pipe */
	while (1) {
		if (read(stdout_fd, buf, 1024) == 0 && read(stderr_fd, buf, 1024) == 0)
			break;
	}

	/* we will wait for child to finish */
	wait(&status);

	/* and close stdout redirect pipe */
	close(stdout_fd);
	close(stderr_fd);
}

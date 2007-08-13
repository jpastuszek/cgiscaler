/***************************************************************************
 *   Copyright (C) 2007 by Jakub Pastuszek   *
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

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "stdio_capture.h"
#include "../cgreen/cgreen.h"
#include "asserts.h"

/* some serving specific helpers */

int orginal_stdout = 0;

/* Returns stdout associated with file descriptor */
int capture_stdout() {
	int p[2];
	
	/* saving original stdout */
	if (!orginal_stdout)
		orginal_stdout = dup(1);

	assert_not_equal(orginal_stdout, 0);

	assert_not_equal(pipe(p), -1);

	assert_not_equal(close(1), -1);
	/* use write pipe end as stdout */
	assert_not_equal(dup2(p[1], 1), -1);

	/* closing original write pipe end as we have it duplicated */
	assert_not_equal(close(p[1]), -1);

	/* return read pipe end */
	return p[0];
}

/* Brings back normal stdout */
void restore_stdout() {
	/* close write pipe end */
	assert_not_equal(close(1), -1);

	/* and restore form saved */
	if (orginal_stdout)
		assert_not_equal(dup2(orginal_stdout, 1), -1);
}

/* this function will fork and redirect child stdout to stdout_fd pipe end; stdout_ft needs to be freed with finish_fork witch will also wait until child exists */
 int fork_with_stdout_capture(int *stdout_fd) {
	*stdout_fd = capture_stdout();
	if(!fork())
		return 0;

	/* We restore our local stdout */
	restore_stdout();
	return 1;
}

/* make sure you finish_fork after doing fork_with_stdout_capture, between this two calls things will happen simultaneously */
 void finish_fork(int stdout_fd) {
	int status;
	/* we will wait for child to finish */
	wait(&status);

	/* and close stdout redirect pipe */
	close(stdout_fd);
}

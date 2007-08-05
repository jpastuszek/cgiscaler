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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#include "debug.h"

int debug_file_fd;
int debug_on;
pid_t pid;

void debug_start(char *file) {
#ifdef DEBUG
	if (debug_on)
		return;

	if (file)
		debug_file_fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0666);
	else
		debug_file_fd = 1; /* STDOUT */
	if (debug_file_fd < 0) {
		debug_on = 0;
		return;	
	}

	pid = getpid();
	debug_on = 1;

	debug(">>>>", "Starting debug");
#endif
}

void debug_stop() {
#ifdef DEBUG
	if (!debug_on)
		return;

	debug("<<<<", "Stopping debug");

	/* Std* should not be closed */
	if (debug_file_fd > 2)
		close(debug_file_fd);
	debug_on = 0;
#endif
}

void debug(const char *level, const char *fmt, ...) {
#ifdef DEBUG
	int size = 40, msg_len, full_msg_len, full_msg_buff_len = 80;
	char *msg, *new_msg;
	char *full_msg;
	va_list ap;

	if (!debug_on)
		return;

	msg = malloc(size);
	if (!msg)
		exit(66);

	while (1) {
		va_start(ap, fmt);

		msg_len = vsnprintf(msg, size, fmt, ap);
	
		/* it worked, we have less then size */
		if (msg_len > -1 && msg_len < size)
			break;
		
		/* we have more then size */
		if (msg_len > -1)
			size = msg_len + 1;
		else
			size *= 2;
	
		/* re-size to add more space */
		new_msg = realloc(msg, size);
		if (!new_msg)
			exit(66);
	
		msg = new_msg;
		va_end(ap);
	}

/*
	write(debug_file_fd, level, strlen(level));
	write(debug_file_fd, ": ", 2);
	write(debug_file_fd, msg, msg_len);
	write(debug_file_fd, "\n", 1);
*/

	/* we are allocating initial file name buffer */
	full_msg = malloc(full_msg_buff_len);
	if (!full_msg)
		exit(66);

	/* now we will loop until snprintf will return less than our buffer size */
	while (1) {
		full_msg_len = snprintf(full_msg, full_msg_buff_len, "%i %s: %s\n", pid, level, msg);
	
		/* it worked, we have less then full_msg_buff_len */
		if (full_msg_len > -1 && full_msg_len < full_msg_buff_len)
			break;
		
		/* we have more then full_msg_buff_len */
		if (full_msg_len > -1)
			full_msg_buff_len = full_msg_len + 1;
		else
			full_msg_buff_len *= 2;
	
		/* re-size to add more space */
		if ((full_msg = realloc(full_msg, full_msg_buff_len)) == NULL)
			exit(66);
	}

	write(debug_file_fd, full_msg, full_msg_len);

	free(full_msg);

#ifdef DEBUG_SYNC
	fsync(debug_file_fd);
#endif
	free(msg);
#endif
}

void timer_start(struct timer *t) {
#ifdef DEBUG
	struct timeval time;
	gettimeofday(&time, NULL);
	t->start = (double) time.tv_sec + ((double) time.tv_usec / 1000000);
#endif
}

double timer_stop(struct timer *t) {
#ifdef DEBUG
	struct timeval time;
	gettimeofday(&time, NULL);
	t->stop = (double) time.tv_sec + ((double) time.tv_usec / 1000000);
	return t->stop - t->start;
#else
	return 0;
#endif
}

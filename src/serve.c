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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "serve.h"
#include "cache.h"
#include "file_utils.h"
#include "config.h"
#include "debug.h"

void remove_cache_file(char *cache_file_path);

int serve_from_file(char *file_path, char *mime_type) {
	unsigned char *buffer;
	int error_file;
	size_t bytes_read, bytes_written, total_bytes_read, total_bytes_written;
	off_t file_size;

	debug(DEB,"Serving from file: '%s' mime-type: '%s'", file_path, mime_type);

	error_file = open(file_path, O_RDONLY);
	if (error_file == -1) {
		debug(WARN,"Failed to open error file '%s': %s", file_path, strerror(errno));
		return 0;
	}

	/* Getting image size */
	file_size = lseek(error_file, 0, SEEK_END);
	if (file_size == -1) {
		close(error_file);
		debug(WARN,"Failed to get file size: %s", strerror(errno));
		return 0;
	}

	if (lseek(error_file, 0, SEEK_SET) == -1) {
		close(error_file);
		debug(WARN,"Failed to reposition file offset: %s", strerror(errno));
		return 0;
	}

	/* Sending headers */
	printf("Content-Type: %s\n", mime_type);
	printf("Content-Length: %u\n", (unsigned int) file_size);
	printf("\n");

	/* Fflush is neccessary to avoid overwriting buffered headers by direct fd writes */
	fflush(stdout);

	/* After we have sent headers we donnot return 0 */

	/* Allocating buffer */
	buffer = malloc(WRITE_BUFFER_LEN);
	if (!buffer)
		exit(66);

	/* Lets pipe data throut the buffer to stdout */
	bytes_read = total_bytes_read = 0;
	while(1) {
		/* reading buffer size or less amount of data */
		bytes_read = read(error_file, buffer, WRITE_BUFFER_LEN);
		if (bytes_read == -1) {
			debug(ERR,"Failed reading error file: %s", strerror(errno));
			exit(10);
		}
		/* all done */
		if (!bytes_read)
			break;

		total_bytes_read += bytes_read;
		
		/* loop until all read data was sent out */
		bytes_written = total_bytes_written = 0;
		do {
			debug(DEB, "Writing %d bytes to stdout", bytes_read);
			bytes_written = write(1, buffer + total_bytes_written, bytes_read);
			debug(DEB, "%d bytes written", bytes_written);
			if (bytes_written == -1) {
				debug(ERR,"Failed writting to stdout: %s", strerror(errno));
				exit(10);
			}
			bytes_read -= bytes_written;
			total_bytes_written += bytes_written;
			/* fsync(1); - this will only work on fs fils not stdout */
		} while(bytes_read);
		
	}

	close(error_file);
	free(buffer);
	return 1;
}

/* serving from blob */
void serve_from_blob(unsigned char *blob, size_t blob_len, char *mime_type) {
	size_t bytes_written;
	size_t total_blob_written;

	debug(DEB,"Serving from BLOB: size: %d", blob_len);

	printf("Content-Type: %s\n", mime_type);
	printf("Content-Length: %u\n", (unsigned int) blob_len);

	printf("\n");
	/* flushing buffers befor we do direct fd write */
	fflush(stdout);

	/* using stdout (FILE *) write instead of fd 1 is safer as printf also is using stdout */
/*	fwrite(blob, blob_len, 1, stdout); */

/* using write is risky as we are writing to fd directly... where using printf we are writting to stdout (FILE *) buffers but at this poing we should have buffers flushed */
	total_blob_written = 0;
	while(1) {
		debug(DEB, "Writing %d bytes to stdout", blob_len - total_blob_written);
		bytes_written = write(1, blob + total_blob_written, blob_len - total_blob_written);
		debug(DEB, "%d bytes written", bytes_written);
		if (bytes_written == -1) {
			debug(ERR, "Error writting to stdout %s", strerror(errno));
			exit(10);
		}
		
		total_blob_written += bytes_written;

		if (total_blob_written >= blob_len)
			break;
		/* fsync(1); - this will only work on fs fils not stdout */
	}
}

/* 
Serves image from cache file
Returns: 1 on success 0 when no proper cache file or read failure 
*/
int serve_from_cache(struct query_params *params) {
	char *cache_file_path;
	int ret;

	cache_file_path = create_cache_file_path(params);
	debug(DEB,"Trying cache file: '%s'", cache_file_path);

/*
Logick:
_ _
O C M

1 1 0 - exit
0 1 0 - exit
1 0 0 - rm, exit
0 0 1 - rm, exit
0 0 0 - serve
u*/

	switch (check_if_cached(params)) {
		/* we don't have cache file or both... returning */
		case NO_CACHE:
		case NO_CACHE | NO_ORIG:
			debug(DEB, "No cache file");
			free(cache_file_path);
			return 0;

		/* we don't have orig or cache file old... remove cache file and return */
		case NO_ORIG:
		case MTIME_DIFFER:
			debug(DEB, "No orginal or mtime with orginal differ");
			remove_cache_file(cache_file_path);
			free(cache_file_path);
			return 0;
	}

	/* serve */
	debug(DEB,"Serving from cache file: '%s'", cache_file_path);
	
	ret = serve_from_file(cache_file_path, OUT_FORMAT_MIME_TYPE);

	free(cache_file_path);

	return ret;
}

void remove_cache_file(char *cache_file_path) {
	debug(DEB, "Removing old cache file '%s'", cache_file_path);
	if (unlink(cache_file_path) == -1)
		debug(WARN, "Removing old cache file '%s' failed: %s", cache_file_path, strerror(errno));
}

/* we will try error miage but if it does not exist we will fail back to error message */
void serve_error() {
	char *file_path;

	file_path = malloc(strlen(MEDIA_PATH) + strlen(ERROR_FILE_PATH) + 1);
	strcpy(file_path, MEDIA_PATH);
	strcat(file_path, ERROR_FILE_PATH);

	debug(DEB,"Serving error image: '%s'", file_path);
	if (!serve_from_file(file_path, ERROR_FILE_MIME_TYPE))
		serve_error_message();

	free(file_path);
}

/* serving plain text message as an absolute failback */
void serve_error_message() {
	printf("Content-Type: text/plain\n");
	printf("\n");

	printf("[Error and error image not found]\n");
	fflush(stdout);
}


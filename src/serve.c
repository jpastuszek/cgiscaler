/***************************************************************************
 *   Copyright (C) 2007, 2008, 2008 by Jakub Pastuszek   *
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "serve.h"
#include "runtime_config.h"
#include "cache.h"
#include "file_utils.h"
#include "debug.h"

extern struct runtime_config *runtime_config;
extern struct error_handling_config *error_handling_config;
extern struct operation_config *operation_config;


/** Send HTTP headers to STDOUT.
* @param content_length content lenght that will be send after HTTP headers - used to set "Content-Length" header
* @param mime_type mime type to include in HTTP headers
*/
void send_headers(unsigned int content_length, char *mime_type) {
	printf("Content-Type: %s\n", mime_type);
	printf("Content-Length: %u\n", content_length);
	printf("\n");

	/* Fflush is necessary to avoid overwriting buffered headers by direct fd writes */
	fflush(stdout);
}

/** Serve file content.
* @param absolute_file_path absolute file location that will be served
* @param mime_type mime type to include in HTTP headers
* @return 1 on success 0 on failure
* @see serve_from_blob()
*/
int serve_from_file(abs_fpath *absolute_file_path, char *mime_type) {
	unsigned char *buffer;
	char *ext;
	int file;
	size_t bytes_read, bytes_written, total_bytes_read, total_bytes_written;
	off_t file_size;
	struct format_info *fi;

	if (!mime_type) {
		ext = extract_file_extension(absolute_file_path);
		if (ext) {
			fi = file_extension_to_format_info(ext);
			if (fi) {
				mime_type = strdup(fi->mime_type);
				free_format_info(fi);
			}
			free(ext);
		}
		
		if (!mime_type) {
			debug(WARN,"Failed to find maching mime-type for file: %s, using fail back default: %s", absolute_file_path, output_config->fail_mime_type);
			mime_type = strdup(output_config->fail_mime_type);
		}
	} else {
		// we duplicat that string as we will try to free it later on
		mime_type = strdup(mime_type);
	}

	debug(DEB,"Serving from file: '%s' mime-type: '%s'", absolute_file_path, mime_type);

	file = open(absolute_file_path, O_RDONLY);
	if (file == -1) {
		debug(WARN,"Failed to open file '%s': %s", absolute_file_path, strerror(errno));
		free(mime_type);
		return 0;
	}

	/* Getting image size */
	file_size = lseek(file, 0, SEEK_END);
	if (file_size == -1) {
		close(file);
		debug(WARN,"Failed to get file size: %s", strerror(errno));
		free(mime_type);
		return 0;
	}

	if (lseek(file, 0, SEEK_SET) == -1) {
		close(file);
		debug(WARN,"Failed to reposition file offset: %s", strerror(errno));
		free(mime_type);
		return 0;
	}

	/* Sending headers */
	if (!operation_config->no_headers)
		send_headers((unsigned int) file_size, mime_type);

	/* After we have sent headers we don't return 0 */

	/* Allocating buffer */
	buffer = malloc(WRITE_BUFFER_LEN);
	if (!buffer)
		exit(66);

	/* Lets pipe data through the buffer to stdout */
	bytes_read = total_bytes_read = 0;
	while(1) {
		/* reading buffer size or less amount of data */
		bytes_read = read(file, buffer, WRITE_BUFFER_LEN);
		if (bytes_read == -1) {
			debug(ERR,"Failed reading file: %s", strerror(errno));
			free(mime_type);
			return(0);
		}

		/* all done */
		if (!bytes_read) {
			/* check if we have served what we supposed to */
			if (total_bytes_read != file_size) {
				debug(ERR,"Failed to serve all data: bytes served: %d content length sent: %d", total_bytes_read, file_size);
				close(file);
				free(buffer);
				free(mime_type);
				return(0);
			}
			break;
		}

		total_bytes_read += bytes_read;
		
		/* loop until all read data was sent out */
		bytes_written = total_bytes_written = 0;
		do {
			debug(DEB, "Writing %d bytes to stdout", bytes_read);
			bytes_written = write(1, buffer + total_bytes_written, bytes_read);
			debug(DEB, "%d bytes written", bytes_written);
			if (bytes_written == -1) {
				debug(ERR,"Failed writing to stdout: %s", strerror(errno));
				close(file);
				free(buffer);
				free(mime_type);
				return 0;
			}
			bytes_read -= bytes_written;
			total_bytes_written += bytes_written;
			/* fsync(1); - this will only work on fs files not stdout */
		} while(bytes_read);
		
	}

	close(file);
	free(buffer);
	free(mime_type);
	return 1;
}

/** Serve from data array.
* @param blob data array to serve
* @param blob_len length of data array in bytes
* @param mime_type mime type to include in HTTP headers
* @see serve_from_file()
*/
int serve_from_blob(unsigned char *blob, size_t blob_len, char *mime_type) {
	size_t bytes_written;
	size_t total_blob_written;

	debug(DEB,"Serving from BLOB: size: %d", blob_len);

	if (!operation_config->no_headers)
		send_headers((unsigned int) blob_len, mime_type);

	/* using stdout (FILE *) write instead of fd 1 is safer as printf also is using stdout */
	/* fwrite(blob, blob_len, 1, stdout); */

	/* using write is risky as we are writing to fd directly... where using printf we are writing to stdout (FILE *) buffers but at this point we should have buffers flushed */
	total_blob_written = 0;
	while(1) {
		debug(DEB, "Writing %d bytes to stdout", blob_len - total_blob_written);
		bytes_written = write(1, blob + total_blob_written, blob_len - total_blob_written);
		debug(DEB, "%d bytes written", bytes_written);
		if (bytes_written == -1) {
			debug(ERR, "Error writing to stdout %s", strerror(errno));
			return 0;
		}
		
		total_blob_written += bytes_written;

		if (total_blob_written >= blob_len)
			break;
		/* fsync(1); - this will only work on fs files not stdout */
	}

	return 1;
}

/** This function will try to serve error image but if it does not exist we will fail back to error message.
* @see serve_error_message()
*/
void serve_error() {
	abs_fpath *absolute_media_file_path;

	

	absolute_media_file_path = create_absolute_media_file_path(error_handling_config->error_image_file);

	debug(DEB,"Serving error image: '%s'", absolute_media_file_path);
	if (!serve_from_file(absolute_media_file_path, 0))
		serve_error_message();

	free_fpath(absolute_media_file_path);
}

/** Serve plain text error message.
* This function will be used as absolute file back to return anythink to web server
*/
void serve_error_message() {
	if (!operation_config->no_headers) {
		printf("Content-Type: text/plain\n");
		printf("\n");
	}

	printf(error_handling_config->error_message);
	fflush(stdout);
}


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

#include <strings.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../cgreen/cgreen.h"
#include "stdio_capture.h"
#include "asserts.h"
#include "file_utils.h"
#include "runtime_config.h"
#include "debug.h"
#include "serve.h"

struct operation_config *operation_config;

/* serve.c tests */
static void test_serve_from_file() {
	int stdout_fd;
	int status;
	abs_fpath *absolute_media_file_path;

	/* real file */
	absolute_media_file_path = create_absolute_media_file_path(IMAGE_TEST_FILE);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		operation_config->no_headers = 0;
		status = serve_from_file(absolute_media_file_path, output_config->format->mime_type);
		if (!status)
			assert_true_with_message(0, "serve_from_file failed");
		exit(0);
	}

	assert_headers_read(stdout_fd);
	assert_byte_read(stdout_fd, get_file_size(absolute_media_file_path));

	finish_fork_with_stdout_capture(stdout_fd);
	free_fpath(absolute_media_file_path);

	/* real file, no headers*/
	absolute_media_file_path = create_absolute_media_file_path(IMAGE_TEST_FILE);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		operation_config->no_headers = 1;
		status = serve_from_file(absolute_media_file_path, output_config->format->mime_type);
		if (!status)
			assert_true_with_message(0, "serve_from_file failed");
		exit(0);
	}

	assert_byte_read(stdout_fd, get_file_size(absolute_media_file_path));

	finish_fork_with_stdout_capture(stdout_fd);
	free_fpath(absolute_media_file_path);

	/* bogus file */
	if (!fork_with_stdout_capture(&stdout_fd)) {
		operation_config->no_headers = 0;
		status = serve_from_file("bogo_file_name.jpg", output_config->format->mime_type);
		if (status)
			assert_true_with_message(0, "serve_from_file failed");
		exit(0);
	}
	
	finish_fork_with_stdout_capture(stdout_fd);
}

static void test_serve_from_blob() {
	int stdout_fd;
	unsigned char *blob;

	blob = malloc(31666);
	assert_not_equal(blob, 0);
	memset(blob, 54, 31666);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		operation_config->no_headers = 0;
		serve_from_blob(blob, 31666, output_config->format->mime_type);
		exit(0);
	}

	assert_headers_read(stdout_fd);
	assert_byte_read(stdout_fd, 31666);

	finish_fork_with_stdout_capture(stdout_fd);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		operation_config->no_headers = 1;
		serve_from_blob(blob, 31666, output_config->format->mime_type);
		exit(0);
	}

	assert_byte_read(stdout_fd, 31666);

	finish_fork_with_stdout_capture(stdout_fd);

	free(blob);
}


static void test_serve_error() {
	int stdout_fd;
	abs_fpath *absolute_media_file_path;

	absolute_media_file_path = create_absolute_media_file_path(error_handling_config->error_image_file);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		operation_config->no_headers = 0;
		serve_error();
		exit(0);
	}

	assert_headers_read(stdout_fd);
	assert_byte_read(stdout_fd, get_file_size(absolute_media_file_path));

	finish_fork_with_stdout_capture(stdout_fd);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		operation_config->no_headers = 1;
		serve_error();
		exit(0);
	}

	assert_byte_read(stdout_fd, get_file_size(absolute_media_file_path));

	finish_fork_with_stdout_capture(stdout_fd);


	free_fpath(absolute_media_file_path);
}

static void test_serve_error_message() {
	int stdout_fd;

	if (!fork_with_stdout_capture(&stdout_fd)) {
		operation_config->no_headers = 0;
		serve_error_message();
		exit(0);
	}

	assert_headers_read(stdout_fd);
	assert_byte_read(stdout_fd, strlen(error_handling_config->error_message));

	finish_fork_with_stdout_capture(stdout_fd);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		operation_config->no_headers = 1;
		serve_error_message();
		exit(0);
	}


	assert_byte_read(stdout_fd, strlen(error_handling_config->error_message));

	finish_fork_with_stdout_capture(stdout_fd);

}


/* setup and teardown */
static void test_setup() {
	alloc_default_config();
	debug_start(logging_config->log_file);
}

static void test_teardown() {
	free_config();
	debug_stop();
}

int main(int argc, char **argv) {
	TestSuite *serve_suite = create_named_test_suite(__FILE__);

	add_test(serve_suite, test_serve_from_file);
	add_test(serve_suite, test_serve_from_blob);
	add_test(serve_suite, test_serve_error);
	add_test(serve_suite, test_serve_error_message);

	setup(serve_suite, test_setup);
	teardown(serve_suite, test_teardown);

	return run_test_suite(serve_suite, create_text_reporter());
}

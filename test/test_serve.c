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

#include <strings.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../cgreen/cgreen.h"
#include "asserts.h"
#include "file_utils.h"
#include "test_config.h"
#include "serve.h"

/* serve.c tests */

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
static int fork_with_stdout_capture(int *stdout_fd) {
	*stdout_fd = capture_stdout();
	if(!fork())
		return 0;

	/* We restore our local stdout */
	restore_stdout();
	return 1;
}

/* make sure you finish_fork after doing fork_with_stdout_capture, between this two calls things will happen simultaneously */
static void finish_fork(int stdout_fd) {
	int status;
	/* we will wait for child to finish */
	wait(&status);

	/* and close stdout redirect pipe */
	close(stdout_fd);
}

static void test_serve_from_file() {
	int stdout_fd;
	int status;
	abs_fpath *absolute_media_file_path;

	/* real file */
	absolute_media_file_path = create_absolute_media_file_path(IMAGE_TEST_FILE);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		status = serve_from_file(absolute_media_file_path, OUT_FORMAT_MIME_TYPE, 0);
		if (!status) {
			restore_stdout();
			assert_true_with_message(0, "serve_from_file failed");
		}
		exit(0);
	}

	assert_headers_read(stdout_fd);
	assert_byte_read(stdout_fd, get_file_size(absolute_media_file_path));

	finish_fork(stdout_fd);
	free_fpath(absolute_media_file_path);

	/* real file, no headers*/
	absolute_media_file_path = create_absolute_media_file_path(IMAGE_TEST_FILE);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		status = serve_from_file(absolute_media_file_path, OUT_FORMAT_MIME_TYPE, 1);
		if (!status) {
			restore_stdout();
			assert_true_with_message(0, "serve_from_file failed");
		}
		exit(0);
	}

	assert_byte_read(stdout_fd, get_file_size(absolute_media_file_path));

	finish_fork(stdout_fd);
	free_fpath(absolute_media_file_path);

	/* bogus file */
	if (!fork_with_stdout_capture(&stdout_fd)) {
		status = serve_from_file("bogo_file_name.jpg", OUT_FORMAT_MIME_TYPE, 0);
		if (status) {
			restore_stdout();
			assert_true_with_message(0, "serve_from_file failed");
		}
		exit(0);
	}
	
	finish_fork(stdout_fd);
}

static void test_serve_from_blob() {
	int stdout_fd;
	unsigned char *blob;

	blob = malloc(31666);
	assert_not_equal(blob, 0);
	memset(blob, 54, 31666);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		serve_from_blob(blob, 31666, OUT_FORMAT_MIME_TYPE, 0);
		exit(0);
	}

	assert_headers_read(stdout_fd);
	assert_byte_read(stdout_fd, 31666);

	finish_fork(stdout_fd);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		serve_from_blob(blob, 31666, OUT_FORMAT_MIME_TYPE, 1);
		exit(0);
	}

	assert_byte_read(stdout_fd, 31666);

	finish_fork(stdout_fd);

	free(blob);
}

static void test_serve_from_cache_file() {
	int stdout_fd;
	int status;
	unsigned char *blob;
	cache_fpath *cache_file_path;
	abs_fpath *absolute_cache_file_path;

	blob = malloc(3000);
	assert_not_equal(blob, 0);
	memset(blob, 54, 3000);

	/* tests with real file */
	cache_file_path = create_cache_file_path(IMAGE_TEST_FILE, OUT_FORMAT_EXTENSION, 0, 0, 0, 0);
	absolute_cache_file_path = create_absolute_cache_file_path(cache_file_path);

	/* create test cache file - mtime should be set properly */
	assert_equal(write_blob_to_cache(blob, 3000, IMAGE_TEST_FILE, cache_file_path), 1);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		status = serve_from_cache_file(IMAGE_TEST_FILE, cache_file_path, OUT_FORMAT_MIME_TYPE, 0);
		if (!status) {
			restore_stdout();
			assert_true_with_message(0, "serve_from_cache failed");
		}
		exit(0);
	}

	assert_headers_read(stdout_fd);
	assert_byte_read(stdout_fd, 3000);

	finish_fork(stdout_fd);

	/* cleaning up test file */
	assert_not_equal(-1, unlink(absolute_cache_file_path));

	/* create test cache file - mtime should be set properly */
	assert_equal(write_blob_to_cache(blob, 3000, IMAGE_TEST_FILE, cache_file_path), 1);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		status = serve_from_cache_file(IMAGE_TEST_FILE, cache_file_path, OUT_FORMAT_MIME_TYPE, 1);
		if (!status) {
			restore_stdout();
			assert_true_with_message(0, "serve_from_cache failed");
		}
		exit(0);
	}

	assert_byte_read(stdout_fd, 3000);

	finish_fork(stdout_fd);

	/* cleaning up test file */
	assert_not_equal(-1, unlink(absolute_cache_file_path));

	/* test with wrong mtime */
	/* create test file - mtime should be set to current time */
	assert_equal(write_blob_to_file(blob, 3000, absolute_cache_file_path), 1);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		status = serve_from_cache_file(IMAGE_TEST_FILE, cache_file_path, OUT_FORMAT_MIME_TYPE, 1);
		if (status) {
			restore_stdout();
			assert_true_with_message(0, "serve_from_cache failed");
		}
		exit(0);
	}

	finish_fork(stdout_fd);

	/* there should be no cache file as it should be removed */
	assert_file_not_exists(absolute_cache_file_path);

	/* test with no cache */
	if (!fork_with_stdout_capture(&stdout_fd)) {
		status = serve_from_cache_file(IMAGE_TEST_FILE, cache_file_path, OUT_FORMAT_MIME_TYPE, 1);
		if (status) {
			restore_stdout();
			assert_true_with_message(0, "serve_from_cache failed");
		}
		exit(0);
	}

	finish_fork(stdout_fd);

	free_fpath(absolute_cache_file_path);
	free_fpath(cache_file_path);

	/* test with no original file */
	cache_file_path = create_cache_file_path("bogo.file", OUT_FORMAT_EXTENSION, 0, 0, 0, 0);
	absolute_cache_file_path = create_absolute_cache_file_path(cache_file_path);

	/* create test cache file */
	assert_equal(write_blob_to_file(blob, 3000, absolute_cache_file_path), 1);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		status = serve_from_cache_file("bogo.file", cache_file_path, OUT_FORMAT_MIME_TYPE, 1);
		if (status) {
			restore_stdout();
			assert_true_with_message(0, "serve_from_cache failed");
		}
		exit(0);
	}

	finish_fork(stdout_fd);

	/* there should be no cache file as it should be removed */
	assert_file_not_exists(absolute_cache_file_path);

	free_fpath(absolute_cache_file_path);
	free_fpath(cache_file_path);

	free(blob);
}

static void test_serve_error() {
	int stdout_fd;
	abs_fpath *absolute_media_file_path;

	absolute_media_file_path = create_absolute_media_file_path(ERROR_FILE_PATH);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		serve_error(0);
		exit(0);
	}

	assert_headers_read(stdout_fd);
	assert_byte_read(stdout_fd, get_file_size(absolute_media_file_path));

	finish_fork(stdout_fd);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		serve_error(1);
		exit(0);
	}

	assert_byte_read(stdout_fd, get_file_size(absolute_media_file_path));

	finish_fork(stdout_fd);


	free_fpath(absolute_media_file_path);
}

static void test_serve_error_message() {
	int stdout_fd;

	if (!fork_with_stdout_capture(&stdout_fd)) {
		serve_error_message(0);
		exit(0);
	}

	assert_headers_read(stdout_fd);
	assert_byte_read(stdout_fd, strlen(ERROR_FAILBACK_MESSAGE));

	finish_fork(stdout_fd);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		serve_error_message(1);
		exit(0);
	}


	assert_byte_read(stdout_fd, strlen(ERROR_FAILBACK_MESSAGE));

	finish_fork(stdout_fd);

}


/* setup and teardown */
static void test_setup() {
	debug_start(DEBUG_FILE);
}

static void test_teardown() {
	debug_stop();
}

int main(int argc, char **argv) {
	TestSuite *serve_suite = create_test_suite();

	add_test(serve_suite, test_serve_from_file);
	add_test(serve_suite, test_serve_from_blob);
	add_test(serve_suite, test_serve_from_cache_file);
	add_test(serve_suite, test_serve_error);
	add_test(serve_suite, test_serve_error_message);

	setup(serve_suite, test_setup);
	teardown(serve_suite, test_teardown);

	return run_test_suite(serve_suite, create_text_reporter());
}

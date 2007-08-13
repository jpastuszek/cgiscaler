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
#include <utime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "../cgreen/cgreen.h"
#include "stdio_capture.h"
#include "asserts.h"
#include "file_utils.h"
#include "test_config.h"
#include "cache.h"
#include "serve.h"


/* cache.c tests */
static void test_if_cached() {
	cache_fpath *cache_file_path;
	abs_fpath *absolute_cach_file_path;
	abs_fpath *absolute_media_file_path;
	int test_fd;
	struct utimbuf time_buf;

	/* tests with bogo file */
	cache_file_path = create_cache_file_path("bogo.jpg", OUT_FORMAT_EXTENSION, 0, 0, 0, 0);
	absolute_cach_file_path = create_absolute_cache_file_path(cache_file_path);

	assert_equal(check_if_cached("bogo.jpg", cache_file_path), NO_ORIG | NO_CACHE);

	/* creating test file */
	test_fd = open(absolute_cach_file_path, O_CREAT|O_WRONLY|O_TRUNC);
	assert_not_equal(test_fd, -1);
	close(test_fd);
	assert_equal(check_if_cached("bogo.jpg", cache_file_path), NO_ORIG);

	/* cleaning up test file */
	assert_not_equal(unlink(absolute_cach_file_path), -1);

	free_fpath(absolute_cach_file_path);
	free_fpath(cache_file_path);

	/* and now with real file */
	cache_file_path = create_cache_file_path(IMAGE_TEST_FILE, OUT_FORMAT_EXTENSION, 0, 0, 0, 0);
	absolute_cach_file_path = create_absolute_cache_file_path(cache_file_path);
	absolute_media_file_path = create_absolute_media_file_path(IMAGE_TEST_FILE);

	assert_equal(check_if_cached(IMAGE_TEST_FILE, cache_file_path), NO_CACHE);

	/* creating test file */
	test_fd = open(absolute_cach_file_path, O_CREAT|O_WRONLY|O_TRUNC);
	assert_not_equal(test_fd, -1);
	close(test_fd);

	assert_equal(check_if_cached(IMAGE_TEST_FILE, cache_file_path), MTIME_DIFFER);

	/* now we will set mtime to match original file */
	time_buf.actime = time_buf.modtime = get_file_mtime(absolute_media_file_path);
	assert_not_equal(utime(absolute_cach_file_path, &time_buf), -1);

	assert_equal(check_if_cached(IMAGE_TEST_FILE, cache_file_path), CACHE_OK);

	/* cleaning up test file */
	assert_not_equal(unlink(absolute_cach_file_path), -1);
	
	free_fpath(absolute_media_file_path);
	free_fpath(absolute_cach_file_path);
	free_fpath(cache_file_path);
}

static void test_write_blob_to_cache() {
	unsigned char *blob;
	cache_fpath *cache_file_path;
	abs_fpath *absolute_cach_file_path;

	blob = malloc(3000);
	memset(blob, 54, 3000);


	/* tests with bogo file */
	cache_file_path = create_cache_file_path("blob.test", OUT_FORMAT_EXTENSION, 0, 0, 0, 0);

	assert_equal(write_blob_to_cache(blob, 3000, "blob.test", cache_file_path), 0);

	free_fpath(cache_file_path);

	/* now we will try existing original file */
	cache_file_path = create_cache_file_path(IMAGE_TEST_FILE, OUT_FORMAT_EXTENSION, 0, 0, 0, 0);
	absolute_cach_file_path = create_absolute_cache_file_path(cache_file_path);

	assert_not_equal(write_blob_to_cache(blob, 3000, IMAGE_TEST_FILE, cache_file_path), 0);
	assert_file_size(absolute_cach_file_path, 3000);

	/* cleaning up test file */
	assert_not_equal(unlink(absolute_cach_file_path), -1);

	free_fpath(absolute_cach_file_path);
	free_fpath(cache_file_path);
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

/* setup and teardown */
static void test_setup() {
	debug_start(DEBUG_FILE);
}

static void test_teardown() {
	debug_stop();
}

int main(int argc, char **argv) {
	TestSuite *cache_suite = create_test_suite();

	add_test(cache_suite, test_if_cached);
	add_test(cache_suite, test_write_blob_to_cache);
	add_test(cache_suite, test_serve_from_cache_file);

	setup(cache_suite, test_setup);
	teardown(cache_suite, test_teardown);

	return run_test_suite(cache_suite, create_text_reporter());
}



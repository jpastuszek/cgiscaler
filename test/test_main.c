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
#include "stdio_capture.h"
#include "asserts.h"
#include "file_utils.h"
#include "test_config.h"
#include "debug.h"
#include "serve.h"
#include "main.h"


static void test_error() {
	int stdout_fd;
	abs_fpath *absolute_media_file_path;
	char *argv[] = {"test", "bogo/bogo.jpg"};
	char *argv_nh[] = {"test", "-nh", "true", "bogo/bogo.jpg"};

	absolute_media_file_path = create_absolute_media_file_path(ERROR_FILE_PATH);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		_main(2, argv);
		exit(0);
	}

	assert_headers_read(stdout_fd);
	assert_byte_read(stdout_fd, get_file_size(absolute_media_file_path));

	finish_fork(stdout_fd);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		_main(4, argv_nh);
		exit(0);
	}

	assert_byte_read(stdout_fd, get_file_size(absolute_media_file_path));

	finish_fork(stdout_fd);

	free_fpath(absolute_media_file_path);
}

static void test_without_cached() {
	int stdout_fd;
	abs_fpath *absolute_media_file_path;
	cache_fpath *cache_file_path;
	abs_fpath *absolute_cach_file_path;

	char *argv[] = {"test", IMAGE_TEST_FILE};
	char *argv_bogo[] = {"test", "bogo/bogo.jpg"};
	char *argv_nh[] = {"test", "-nh", "true", IMAGE_TEST_FILE};

	absolute_media_file_path = create_absolute_media_file_path(IMAGE_TEST_FILE);

	/* real file */
	if (!fork_with_stdout_capture(&stdout_fd)) {
		_main(2, argv);
		exit(0);
	}

	assert_headers_read(stdout_fd);
	/* We cannot know what size it will have after scale */
	assert_jpg_byte_read_in_range(stdout_fd, 1000, get_file_size(absolute_media_file_path));

	finish_fork(stdout_fd);

	/* real file, no headers */
	if (!fork_with_stdout_capture(&stdout_fd)) {
		_main(4, argv_nh);
		exit(0);
	}

	/* We cannot know what size it will have after scale */
	assert_jpg_byte_read_in_range(stdout_fd, 1000, get_file_size(absolute_media_file_path));

	finish_fork(stdout_fd);

	/* clean up */
	free_fpath(absolute_media_file_path);

	/* bogus file */
	if (!fork_with_stdout_capture(&stdout_fd)) {
		_main(4, argv_bogo);
		exit(0);
	}
	
	finish_fork(stdout_fd);

	/* get rid of the cache file */
	cache_file_path = create_cache_file_path(IMAGE_TEST_FILE, OUT_FORMAT_EXTENSION, DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_STRICT, DEFAULT_QUALITY);
	absolute_cach_file_path = create_absolute_cache_file_path(cache_file_path);

	assert_not_equal(unlink(absolute_cach_file_path), -1);

	/* clean up */
	free_fpath(absolute_cach_file_path);
	free_fpath(cache_file_path);
}

static void test_no_cache_no_file_name_given() {
	int stdout_fd;
	abs_fpath *absolute_media_file_path;
	char *argv_nc[] = {"test", "-nc", "true"};

	absolute_media_file_path = create_absolute_media_file_path(ERROR_FILE_PATH);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		_main(3, argv_nc);
		exit(0);
	}

	assert_headers_read(stdout_fd);
	assert_byte_read(stdout_fd, get_file_size(absolute_media_file_path));

	finish_fork(stdout_fd);

	free_fpath(absolute_media_file_path);
}


static void test_from_cache() {
	int stdout_fd;
	abs_fpath *absolute_media_file_path;
	cache_fpath *cache_file_path;
	abs_fpath *absolute_cach_file_path;
	char *argv[] = {"test", IMAGE_TEST_FILE};

	absolute_media_file_path = create_absolute_media_file_path(IMAGE_TEST_FILE);
	cache_file_path = create_cache_file_path(IMAGE_TEST_FILE, OUT_FORMAT_EXTENSION, DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_STRICT, DEFAULT_QUALITY);
	absolute_cach_file_path = create_absolute_cache_file_path(cache_file_path);

	/* real file  - no cache file yet */
	assert_file_not_exists(absolute_cach_file_path);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		_main(2, argv);
		exit(0);
	}

	assert_headers_read(stdout_fd);
	/* We cannot know what size it will have after scale */
	assert_jpg_byte_read_in_range(stdout_fd, 1000, get_file_size(absolute_media_file_path));

	finish_fork(stdout_fd);

	/* real file - now we should have the cache file*/
	assert_file_exists(absolute_cach_file_path);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		_main(2, argv);
		exit(0);
	}

	assert_headers_read(stdout_fd);
	/* We cannot know what size it will have after scale */
	assert_jpg_byte_read_in_range(stdout_fd, 1000, get_file_size(absolute_media_file_path));

	finish_fork(stdout_fd);

	/* get rid of the cache file */
	assert_not_equal(unlink(absolute_cach_file_path), -1);

	/* clean up */
	free_fpath(absolute_media_file_path);
	free_fpath(absolute_cach_file_path);
	free_fpath(cache_file_path);
}


static void test_bad_param() {
	int stdout_fd;
	abs_fpath *absolute_media_file_path;
	char *argv_nc[] = {"test", "-bogo", "true"};

	absolute_media_file_path = create_absolute_media_file_path(ERROR_FILE_PATH);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		_main(3, argv_nc);
		exit(0);
	}

	assert_headers_read(stdout_fd);
	assert_byte_read(stdout_fd, get_file_size(absolute_media_file_path));

	finish_fork(stdout_fd);

	free_fpath(absolute_media_file_path);
}


static void test_no_cache () {
	int stdout_fd;
	abs_fpath *absolute_media_file_path;
	cache_fpath *cache_file_path;
	abs_fpath *absolute_cach_file_path;
	char *argv[] = {"test", "-nc", "true", IMAGE_TEST_FILE};

	absolute_media_file_path = create_absolute_media_file_path(IMAGE_TEST_FILE);
	cache_file_path = create_cache_file_path(IMAGE_TEST_FILE, OUT_FORMAT_EXTENSION, DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_STRICT, DEFAULT_QUALITY);
	absolute_cach_file_path = create_absolute_cache_file_path(cache_file_path);

	/* real file  - no cache file yet */
	assert_file_not_exists(absolute_cach_file_path);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		_main(4, argv);
		exit(0);
	}

	assert_headers_read(stdout_fd);
	/* We cannot know what size it will have after scale */
	assert_jpg_byte_read_in_range(stdout_fd, 1000, get_file_size(absolute_media_file_path));

	finish_fork(stdout_fd);

	/* cache file should not be there */
	assert_file_not_exists(absolute_cach_file_path);

	/* clean up */
	free_fpath(absolute_media_file_path);
	free_fpath(absolute_cach_file_path);
	free_fpath(cache_file_path);
}

static void test_no_heders() {
	int stdout_fd;
	abs_fpath *absolute_media_file_path;
	cache_fpath *cache_file_path;
	abs_fpath *absolute_cach_file_path;
	char *argv[] = {"test", "-nh", "true", IMAGE_TEST_FILE};

	absolute_media_file_path = create_absolute_media_file_path(IMAGE_TEST_FILE);
	cache_file_path = create_cache_file_path(IMAGE_TEST_FILE, OUT_FORMAT_EXTENSION, DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_STRICT, DEFAULT_QUALITY);
	absolute_cach_file_path = create_absolute_cache_file_path(cache_file_path);

	/* real file  - no cache file yet */
	assert_file_not_exists(absolute_cach_file_path);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		_main(4, argv);
		exit(0);
	}

	/* We cannot know what size it will have after scale */	assert_jpg_byte_read_in_range(stdout_fd, 1000, get_file_size(absolute_media_file_path));

	finish_fork(stdout_fd);

	/* get rid of the cache file */
	assert_not_equal(unlink(absolute_cach_file_path), -1);

	/* clean up */
	free_fpath(absolute_media_file_path);
	free_fpath(absolute_cach_file_path);
	free_fpath(cache_file_path);	
}

static void test_no_server() {
	int stdout_fd;
	abs_fpath *absolute_media_file_path;
	cache_fpath *cache_file_path;
	abs_fpath *absolute_cach_file_path;
	char *argv[] = {"test", "-ns", "true", IMAGE_TEST_FILE};

	absolute_media_file_path = create_absolute_media_file_path(IMAGE_TEST_FILE);
	cache_file_path = create_cache_file_path(IMAGE_TEST_FILE, OUT_FORMAT_EXTENSION, DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_STRICT, DEFAULT_QUALITY);
	absolute_cach_file_path = create_absolute_cache_file_path(cache_file_path);

	/* real file  - no cache file yet */
	assert_file_not_exists(absolute_cach_file_path);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		_main(4, argv);
		exit(0);
	}

	/* We cannot know what size it will have after scale */	
	assert_byte_read(stdout_fd, 0);

	finish_fork(stdout_fd);

	/* get rid of the cache file */
	assert_not_equal(unlink(absolute_cach_file_path), -1);

	/* clean up */
	free_fpath(absolute_media_file_path);
	free_fpath(absolute_cach_file_path);
	free_fpath(cache_file_path);	
}

static void test_no_serve_no_cache_no_header() {
	int stdout_fd;
	cache_fpath *cache_file_path;
	abs_fpath *absolute_cach_file_path;
	char *argv[] = {"test", "-ns", "true", "-nc", "true", "-nh", "true", IMAGE_TEST_FILE};

	cache_file_path = create_cache_file_path(IMAGE_TEST_FILE, OUT_FORMAT_EXTENSION, DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_STRICT, DEFAULT_QUALITY);
	absolute_cach_file_path = create_absolute_cache_file_path(cache_file_path);

	/* real file  - no cache file yet */
	assert_file_not_exists(absolute_cach_file_path);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		_main(8, argv);
		exit(0);
	}

	/* We cannot know what size it will have after scale */	
	assert_byte_read(stdout_fd, 0);

	finish_fork(stdout_fd);

	assert_file_not_exists(absolute_cach_file_path);

	/* clean up */
	free_fpath(absolute_cach_file_path);
	free_fpath(cache_file_path);
}


/* setup and teardown */
static void test_setup() {
	debug_start(DEBUG_FILE);
}

static void test_teardown() {
	debug_stop();
}

int main(int argc, char **argv) {
	TestSuite *main_suite = create_named_test_suite(__FILE__);

	add_test(main_suite, test_error);
	add_test(main_suite, test_without_cached);
	add_test(main_suite, test_from_cache);
	add_test(main_suite, test_no_cache_no_file_name_given);
	add_test(main_suite,test_bad_param);
	add_test(main_suite, test_no_cache);
	add_test(main_suite, test_no_heders);
	add_test(main_suite, test_no_server);
	add_test(main_suite, test_no_serve_no_cache_no_header);

	setup(main_suite, test_setup);
	teardown(main_suite, test_teardown);

	return run_test_suite(main_suite, create_text_reporter());
}

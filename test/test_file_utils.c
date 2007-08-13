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
#include <unistd.h>

#include "../cgreen/cgreen.h"
#include "asserts.h"
#include "file_utils.h"
#include "test_config.h"
#include "debug.h"

static void test_create_cache_file_path() {
	char compare_file_path[255];
	char *cache_file;

	cache_file = create_cache_file_path("test.jpg", OUT_FORMAT_EXTENSION, 23, 12, 1, 32);

	snprintf(compare_file_path, 256, "%s-23-12-1-32.%s", "test.jpg", OUT_FORMAT_EXTENSION);
	assert_string_equal(cache_file, compare_file_path);

	free_fpath(cache_file);
}

/* file_utils.c tests */
static void test_create_absolute_media_file_path() {
	char compare_file_path[255];
	media_fpath *file_path;

	/* we don't get too much in to as it would reproduce function it self :D */
	file_path = create_absolute_media_file_path("test.jpg");
	assert_not_equal(file_path, 0);

	snprintf(compare_file_path, 256, "%s%s", MEDIA_PATH, "test.jpg");
	assert_string_equal(file_path, compare_file_path);

	free_fpath(file_path);
}

static void test_create_absolute_cache_file_path() {
	char compare_file_path[255];
	media_fpath *file_path;

	/* we don't get too much in to as it would reproduce function it self :D */
	file_path = create_absolute_cache_file_path("test.jpg");
	assert_not_equal(file_path, 0);

	snprintf(compare_file_path, 256, "%s%s", CACHE_PATH, "test.jpg");
	assert_string_equal(file_path, compare_file_path);

	free_fpath(file_path);
}


static void test_make_file_name_relative() {
	fpath *test = make_file_name_relative("///00/ff/test.jpg");
	assert_not_equal(test, 0);	

	assert_string_equal(test, "00/ff/test.jpg");

	free_fpath(test);
}

static void test_check_for_double_dot() {
	fpath *test = make_file_name_relative("///00/ff/test.jpg");
	fpath *dot_test = make_file_name_relative("///00/../ff/test.jpg");

	assert_equal(check_for_double_dot(test), 0);
	assert_equal(check_for_double_dot(dot_test), 1);

	free_fpath(dot_test);
	free_fpath(test);
}

static void test_create_cache_dir_struct() {
	cache_fpath path1[80], path2[80], path3[80];
	int status;

	status = create_cache_dir_struct("abc/d///ghi/test.jpg");
	assert_not_equal(status, 0);

	strcpy(path1, CACHE_PATH);
	strcat(path1, "abc");

	strcpy(path2, CACHE_PATH);
	strcat(path2, "abc/d");

	strcpy(path3, CACHE_PATH);
	strcat(path3, "abc/d/ghi");

	assert_dir_exists(path1);
	assert_dir_exists(path2);
	assert_dir_exists(path3);

	assert_not_equal(rmdir(path3),-1);
	assert_not_equal(rmdir(path2),-1);
	assert_not_equal(rmdir(path1),-1);

	status = create_cache_dir_struct("abc/../def/ghi/test.jpg");
	assert_equal(status, 0);
}

static void test_get_media_file_mtime() {
	assert_equal(get_media_file_mtime("bogous"), 0);
	assert_not_equal(get_media_file_mtime(IMAGE_TEST_FILE), 0);
}

static void test_sanitize_file_path() {
	fpath *sanitized_test;
	fpath *test = make_file_name_relative("///00/ff/test.jpg");
	fpath *sanitized_dot_test;
	fpath *dot_test = make_file_name_relative("///00/../ff/test.jpg");
	
	sanitized_test = sanitize_file_path(test);
	assert_not_equal(sanitized_test, 0);

	sanitized_dot_test = sanitize_file_path(dot_test);
	assert_equal(sanitized_dot_test, 0); 

	free_fpath(sanitized_test);
	free_fpath(dot_test);
	free_fpath(test);
}


static void test_write_blob_to_file() {
	unsigned char *test_blob;
	abs_fpath *absolute_file_path;

	test_blob = malloc(3666);
	memset(test_blob, 54, 3666);

	absolute_file_path = create_absolute_cache_file_path("test.file");

	assert_not_equal(write_blob_to_file(test_blob, 3666, absolute_file_path), 0);
	assert_file_size(absolute_file_path, 3666);

	/* cleaning up test file */
	assert_not_equal(unlink(absolute_file_path), -1);
	
	free_fpath(absolute_file_path);
	free(test_blob);
}

/* we need test_write_blob_to_file passing first */
static void test_get_cache_file_mtime() {
	unsigned char *test_blob;
	abs_fpath *absolute_file_path;

	test_blob = malloc(3666);
	memset(test_blob, 54, 3666);

	absolute_file_path = create_absolute_cache_file_path("test.file");

	assert_not_equal(write_blob_to_file(test_blob, 3666, absolute_file_path), 0);
	assert_file_size(absolute_file_path, 3666);

	assert_equal(get_cache_file_mtime("bogous"), 0);
	assert_not_equal(get_cache_file_mtime("test.file"), 0);

	/* cleaning up test file */
	assert_not_equal(unlink(absolute_file_path), -1);
	
	free_fpath(absolute_file_path);
	free(test_blob);
}


/* setup and teardown */
static void test_setup() {
	debug_start(DEBUG_FILE);
}

static void test_teardown() {
	debug_stop();
}

int main(int argc, char **argv) {
	TestSuite *file_utils_suite = create_test_suite();

	add_test(file_utils_suite, test_create_cache_file_path);
	add_test(file_utils_suite, test_create_absolute_media_file_path);
	add_test(file_utils_suite, test_create_absolute_cache_file_path);
	add_test(file_utils_suite, test_make_file_name_relative);
	add_test(file_utils_suite, test_check_for_double_dot);
	add_test(file_utils_suite, test_create_cache_dir_struct);
	add_test(file_utils_suite, test_get_media_file_mtime);
	add_test(file_utils_suite, test_sanitize_file_path);
	add_test(file_utils_suite, test_write_blob_to_file);
	add_test(file_utils_suite, test_get_cache_file_mtime);

	setup(file_utils_suite, test_setup);
	teardown(file_utils_suite, test_teardown);

	return run_test_suite(file_utils_suite, create_text_reporter());
}

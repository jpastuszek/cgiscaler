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
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <magick/MagickCore.h>

#include "../cgreen/cgreen.h"
#include "file_utils.h"
#include "query_string.h"
#include "geometry_math.h"
#include "cgiscaler.h"
#include "cache.h"
#include "serve.h"
#include "test_config.h"

#include "debug.h"

ssize_t get_file_size(char *file_path);

/* my own asserts */

void assert_equal_low_precision(double value, double expected, double low_precision_error) {
	int ret, range_min;

	range_min = expected - (expected * low_precision_error);
	if (value <= expected && value >= range_min)
		ret = 1;
	else
		ret = 0;


	assert_true_with_message(ret, "value [%f] not in precision range of [%f] and [%f] (%f)", value, range_min, expected, low_precision_error);
}

void assert_equal_precision(double value, double expected, double precision_error) {
	int ret;
	double range_delta;

	range_delta = (expected * precision_error);
	if (value <= expected + (range_delta / 2) && value >= expected - (range_delta / 2))
		ret = 1;
	else
		ret = 0;

	assert_true_with_message(ret, "value [%f] not in precision range of [%f] and [%f] (%f)", value, expected - (range_delta / 2), expected + (range_delta / 2), precision_error);
}

void assert_image_pixel_color(MagickWand *magick_wand, int x, int y, const char *color) {
	PixelWand *pixel_wand;
	MagickBooleanType status;

	pixel_wand = NewPixelWand();
	status = MagickGetImagePixelColor(magick_wand, x, y, pixel_wand);
	assert_not_equal(status, MagickFalse);

	/* we assert that we no longer have transparent pixel */
	assert_string_equal(PixelGetColorAsString(pixel_wand), color);

	DestroyPixelWand(pixel_wand);
}

void assert_image_pixel_alpha(MagickWand *magick_wand, int x, int y, float alpha) {
	PixelWand *pixel_wand;
	MagickBooleanType status;
	int true;
	float tested_alpha;

	pixel_wand = NewPixelWand();
	status = MagickGetImagePixelColor(magick_wand, x, y, pixel_wand);
	assert_not_equal(status, MagickFalse);

	true = 0;
	tested_alpha = PixelGetAlpha(pixel_wand);
	if (tested_alpha == alpha)
		true = 1;

	assert_true_with_message(true, "pixel alpha should be [%f] but is [%f]", alpha, tested_alpha);

	DestroyPixelWand(pixel_wand);
}

void assert_dir_exists(char *dir_path) {
	struct stat s;
	assert_not_equal_with_message(stat(dir_path, &s), -1, "directory [%s] does not exist", dir_path);
	assert_true_with_message(S_ISDIR(s.st_mode), "directory [%s] is not a directory", dir_path);
}

void assert_file_exists(char *file_path) {
	int fd;
	fd = open(file_path, O_RDONLY);
	assert_not_equal_with_message(fd, -1, "file [%s] does not exist", file_path);
	if (fd != -1)
		close(fd);
}

void assert_file_size(char *file_path, off_t size) {
	ssize_t fs;
	fs = get_file_size(file_path);

	assert_equal_with_message(fs, size, "file [%s] size is [%d] while should be [%d]", file_path, fs, size);
}

/* This function will read from fd until EOF and check if it read bytes number of bytes */
void assert_byte_read(int fd, ssize_t bytes) {
	int bread;
	int btotal;
	char buf[256];

	btotal = 0;

	do {
		bread = read(fd, buf, 256);
		if (bread == -1)
			assert_true_with_message(0, "read from fd [%d] failed", fd);
		btotal += bread;

		/* debug(DEB, "Test: Bytes read %d", bread); */
	} while(bread > 0);

	assert_equal_with_message(btotal, bytes, "byte read [%d] from fd [%d] not equal [%d]", btotal, fd, bytes);
}

/* This function will look for \n\n in first 1000 bytes */
void assert_headers_read(int fd) {
	int bread;
	int btotal;
	char buf[1];
	int has_end_line;

	btotal = 0;
	has_end_line = 0;

	do {
		bread = read(fd, buf, 1);
		if (bread == -1)
			assert_true_with_message(0, "read from fd [%d] failed", fd);
		btotal += bread;

		if (buf[0] == '\n')
			has_end_line++;
		else
			has_end_line = 0;

		if (has_end_line == 2)
			break;
		/* debug(DEB, "Test: Bytes read %d", bread); */
	} while(bread > 0);

	if (btotal > 1000)
		assert_true_with_message(0, "headers not in first 1000 bytes");
	assert_equal_with_message(has_end_line, 2, "no double end line found while reading from fd [%d]", fd);
}


/* helpers */

/* Returns file size in bytes or -1 */
ssize_t get_file_size(char *file_path) {
	int fd;
	off_t pos;

	assert_file_exists(file_path);

	fd = open(file_path, O_RDONLY);
	pos = lseek(fd, 0, SEEK_END);
	assert_not_equal(pos, -1);

	if (fd != -1)
		close(fd);

	return pos;
}

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

/* file_utils.c tests */
static void test_create_media_file_path() {
	char *path;
	/* we don't get too much in to as it would reproduce function it self :D */
	path = create_media_file_path("test.jpg");
	assert_not_equal(path, 0);
}

static void test_create_cache_file_path() {
	char query_string[256];
	char compare_cache_file[255];
	char *cache_file;
	struct query_params *params;	

	snprintf(query_string, 256, "%s=123&%s=213&beer=czech_lager&%s=%s&%s=%s", WIDTH_PARAM, HEIGHT_PARAM, STRICT_PARAM, TRUE_PARAM_VAL, LOWQ_PARAM, TRUE_PARAM_VAL);

	params = get_query_params("test.jpg", query_string);
	cache_file = create_cache_file_path(params);

	snprintf(compare_cache_file, 256, "%s%s-123-213-1-1", CACHE_PATH, "test.jpg");
	assert_string_equal(cache_file, compare_cache_file);

	free_query_params(params);
	free(cache_file);
}

static void test_check_for_double_dot() {
	char *test = make_file_name_relative("///00/ff/test.jpg");
	char *dot_test = make_file_name_relative("///00/../ff/test.jpg");

	assert_equal(check_for_double_dot(test), 0);
	assert_equal(check_for_double_dot(dot_test), 1);
}

static void test_create_cache_dir_struct() {
	char path1[80], path2[80], path3[80];
	int status;

	status = create_cache_dir_struct("abc/def/ghi/test.jpg");
	assert_not_equal(status, 0);

	strcpy(path1, CACHE_PATH);
	strcat(path1, "abc");

	strcpy(path2, CACHE_PATH);
	strcat(path2, "abc/def");

	strcpy(path3, CACHE_PATH);
	strcat(path3, "abc/def/ghi");

	assert_dir_exists(path1);
	assert_dir_exists(path2);
	assert_dir_exists(path3);

	assert_not_equal(rmdir(path3),-1);
	assert_not_equal(rmdir(path2),-1);
	assert_not_equal(rmdir(path1),-1);

	status = create_cache_dir_struct("abc/../def/ghi/test.jpg");
	assert_equal(status, 0);
}

static void test_get_file_mtime() {
	char *real_file;

	assert_equal(get_file_mtime("bogous"), 0);

	real_file = create_media_file_path(IMAGE_TEST_FILE);

	assert_not_equal(get_file_mtime(real_file), 0);

	free(real_file);
}

static void test_make_file_name_relative() {
	char *test = make_file_name_relative("///00/ff/test.jpg");
	assert_not_equal(test, 0);	

	assert_string_equal(test, "00/ff/test.jpg");
	free(test);
}

static void test_write_blob_to_file() {
	unsigned char *test_blob;
	char file_path[80];

	test_blob = malloc(3666);
	snprintf(file_path, 80, "%s/test.file", CACHE_PATH);

	assert_not_equal(write_blob_to_file(test_blob, 3666, file_path), 0);
	assert_file_size(file_path, 3666);

	/* cleaning up test file */
	assert_not_equal(unlink(file_path), -1);
	
	free(test_blob);
}

/* query_string.c tests */
static void test_query_string_param() {
	char *prog;
	char *name;
	char *year;
	char *null;

	prog = get_query_string_param("name=kaz&prog=cgiscaler&year=2007","prog");
	assert_string_equal(prog, "cgiscaler");
	free(prog);

	name = get_query_string_param("name=kaz&prog=cgiscaler&year=2007","name");
	assert_string_equal(name, "kaz");
	free(name);

	year = get_query_string_param("name=kaz&prog=cgiscaler&year=2007","year");
	assert_string_equal(year, "2007");
	free(year);

	null = get_query_string_param("name=kaz&prog=cgiscaler&year=2007","null");
	assert_equal(null, 0);
}

static void test_get_query_params() {
	struct query_params *params;
	char test_query_string[256];

	/* testing no file name and no params */
	params = get_query_params("", "");
	assert_equal(params, 0);

	/* testing defaults */
	params = get_query_params("/some/path/funny.jpeg", "");
	assert_not_equal(params, 0);
	assert_equal(params->size.w, 0);
	assert_equal(params->size.h, 0);
	assert_equal(params->strict, 0);
	assert_equal(params->lowq, 0);
	free_query_params(params);

	snprintf(test_query_string, 256, "%s=123&%s=213&beer=czech_lager&%s=%s&%s=%s", WIDTH_PARAM, HEIGHT_PARAM, STRICT_PARAM, TRUE_PARAM_VAL, LOWQ_PARAM, TRUE_PARAM_VAL);

	/* testing no file name */
	params = get_query_params("", test_query_string);
	assert_equal(params, 0);

	params = get_query_params("/some/path/funny.jpeg", test_query_string);
	assert_not_equal(params, 0);
	assert_string_equal(params->file_name, "some/path/funny.jpeg");
	assert_equal(params->size.w, 123);
	assert_equal(params->size.h, 213);
	assert_equal(params->strict, 1);
	assert_equal(params->lowq, 1);
	free_query_params(params);

	snprintf(test_query_string, 256, "beer=czech_lager&%s=%s&%s=%s", STRICT_PARAM, "xbrna", LOWQ_PARAM, "false");

	params = get_query_params("/some/path/funny.jpeg", test_query_string);
	assert_not_equal(params, 0);
	assert_string_equal(params->file_name, "some/path/funny.jpeg");
	assert_equal(params->size.w, 0);
	assert_equal(params->size.h, 0);
	assert_equal(params->strict, 0);
	assert_equal(params->lowq, 0);
	free_query_params(params);

	snprintf(test_query_string, 256, "beer=czech_lager&%s=%s&%s=%s", STRICT_PARAM, "xbrna", LOWQ_PARAM, TRUE_PARAM_VAL);

	params = get_query_params("///some/path/funn/y2.jpeg", test_query_string);
	assert_not_equal(params, 0);
	assert_string_equal(params->file_name, "some/path/funn/y2.jpeg");
	assert_equal(params->size.w, 0);
	assert_equal(params->size.h, 0);
	assert_equal(params->strict, 0);
	assert_equal(params->lowq, 1);
	free_query_params(params);
}

/* geometry_math.c tests */
static void test_resize_to_fit_in() {
	struct dimensions a, b, c;

	a.w = 100;
	a.h = 200;

	/* down fit */
	b.w = 200;
	b.h = 100;

	c = resize_to_fit_in(a, b);
	assert_equal(c.w, 50);
	assert_equal(c.h, 100);

	/* up fit */
	b.w = 1200;
	b.h = 1200;

	c = resize_to_fit_in(a, b);
	assert_equal(c.w, 600);
	assert_equal(c.h, 1200);
}

static void test_reduce_filed() {
	struct dimensions a, b;

	/* field = 100*200 = 20000 */
	a.w = 100;
	a.h = 200;

	b = reduce_filed(a, 1234);
	/* we check if reduction worked (within 10% lower precision margin) */
	assert_equal_low_precision(b.w * b.h, 1234.0, 0.1);
	/* we check if aspect ratio did not changed (within 10% precision margin) */
	assert_equal_precision((float) b.w / b.h,(float) a.w / a.h, 0.1);

	/* over reduction - nothing should change */
	b = reduce_filed(a, 999999);
	assert_equal(a.w, 100);
	assert_equal(a.h, 200);
}

/* cgiscaler.c tests */
static void test_load_image() {
	MagickWand *magick_wand;

	magick_wand = load_image("non_existing_file.jpg");
	assert_equal(magick_wand, 0);

	magick_wand = load_image(IMAGE_TEST_FILE);
	assert_not_equal(magick_wand, 0);
		
	assert_equal(MagickGetImageWidth(magick_wand), IMAGE_TEST_FILE_WIDTH);
	assert_equal(MagickGetImageHeight(magick_wand), IMAGE_TEST_FILE_HEIGHT);
	/* Checking if comment was striped off */
	/* MagickGetImageAttribute is depricated but not yet in IM v. 6.3.0 */
	assert_equal(MagickGetImageAttribute(magick_wand, "comment"), 0);
/* TODO: ./configure should generate IM version and if newer versions are used use instead:	
	assert_equal(MagickGetImageProperty(magick_wand, "comment"), 0);
*/

	free_image(magick_wand);
}

static void test_fit_resize() {
	MagickWand *magick_wand;
	struct dimensions a, img, b;

	a.w = 100;
	a.h = 200;

	magick_wand = load_image(IMAGE_TEST_FILE);
	assert_not_equal(magick_wand, 0);

	img.w = MagickGetImageWidth(magick_wand);
	img.h = MagickGetImageHeight(magick_wand);

	magick_wand = fit_resize(magick_wand, a);
	assert_not_equal(magick_wand, 0);

	/* as we know this works from previous tests we can calculate image size to see if it works */
	b = resize_to_fit_in(img, a);

	assert_equal(MagickGetImageWidth(magick_wand), b.w);
	assert_equal(MagickGetImageHeight(magick_wand), b.h);

	free_image(magick_wand);
}

static void test_strict_resize() {
	MagickWand *magick_wand;
	struct dimensions a;

	a.w = 100;
	a.h = 200;

	magick_wand = load_image(IMAGE_TEST_FILE);
	assert_not_equal(magick_wand, 0);

	magick_wand = strict_resize(magick_wand, a);
	assert_not_equal(magick_wand, 0);

	assert_equal(MagickGetImageWidth(magick_wand), 100);
	assert_equal(MagickGetImageHeight(magick_wand), 200);

	free_image(magick_wand);
}

static void test_resize_field_limiting() {
	MagickWand *magick_wand;
	struct dimensions a;

	/* we will re-size a little bit so resizing will take effect */
	a.w = IMAGE_TEST_FILE_WIDTH - 10;
	a.h = IMAGE_TEST_FILE_HEIGHT - 10;

	/* MAX_PIXEL_NO is set to lower then a.w * a.h */
	if (a.w * a.h > MAX_PIXEL_NO)
		assert_true(1);
	else
		assert_true(0);

	/* fit_resize */
	magick_wand = load_image(IMAGE_TEST_FILE);
	assert_not_equal(magick_wand, 0);

	magick_wand = fit_resize(magick_wand, a);
	assert_not_equal(magick_wand, 0);

	assert_not_equal(MagickGetImageWidth(magick_wand), a.w);
	assert_not_equal(MagickGetImageHeight(magick_wand), a.h);
	assert_equal_low_precision(MagickGetImageWidth(magick_wand) * MagickGetImageHeight(magick_wand), MAX_PIXEL_NO, 0.1);

	free_image(magick_wand);

	/* strict_resize */
	magick_wand = load_image(IMAGE_TEST_FILE);
	assert_not_equal(magick_wand, 0);

	magick_wand = strict_resize(magick_wand, a);
	assert_not_equal(magick_wand, 0);

	assert_not_equal(MagickGetImageWidth(magick_wand), a.w);
	assert_not_equal(MagickGetImageHeight(magick_wand), a.h);
	assert_equal_low_precision(MagickGetImageWidth(magick_wand) * MagickGetImageHeight(magick_wand), MAX_PIXEL_NO, 0.1);

	free_image(magick_wand);
}

static void test_remove_transparentcy() {
	MagickWand *magick_wand;
	struct dimensions a;

	/* we re-size just 1 pixel so we don't loose our transparent region at (10,10) */
	a.w = IMAGE_TEST_FILE_WIDTH - 1;
	a.h = IMAGE_TEST_FILE_HEIGHT - 1;

	magick_wand = load_image(IMAGE_TEST_FILE);
	assert_not_equal(magick_wand, 0);

	/* assert we have our test image loaded OK */
	assert_image_pixel_color(magick_wand, IMAGE_TEST_FILE_WIDTH - 1, IMAGE_TEST_FILE_HEIGHT - 1, "rgb240,0,255");

	/* we assert that we don't have transparent pixel */
	assert_image_pixel_alpha(magick_wand, 10, 10, 1.0);
	/* and color is DEFAULT_BACKGROUND_COLOR_MAGICK_STR */
	assert_image_pixel_color(magick_wand, 10, 10, DEFAULT_BACKGROUND_COLOR_MAGICK_STR);

	magick_wand = strict_resize(magick_wand, a);
	assert_not_equal(magick_wand, 0);


	free_image(magick_wand);
}

static void test_prepare_blob() {
	MagickWand *magick_wand;
	struct dimensions a;
	size_t blob_len;
	unsigned char *blob;

	a.w = 100;
	a.h = 200;

	magick_wand = load_image(IMAGE_TEST_FILE);
	assert_not_equal(magick_wand, 0);

	magick_wand = strict_resize(magick_wand, a);
	assert_not_equal(magick_wand, 0);

	/* we are preparing JPEG data */
	blob = prepare_blob(magick_wand, 70, &blob_len, "JPG");
	assert_not_equal(blob, 0);
	assert_not_equal(blob_len, 0);

	/* JPEG data magic number */
	assert_equal(blob[0], 0xff);
	assert_equal(blob[1], 0xd8);

	free_blob(blob);
}

/* cache.c tests */
static void test_if_cached() {
	char query_string[80];
	char *cache_file_path;
	char *orig_file_path;
	struct query_params *params;	
	int test_fd;
	struct utimbuf time_buf;

	snprintf(query_string, 256, "%s=123&%s=213&beer=czech_lager&%s=%s&%s=%s", WIDTH_PARAM, HEIGHT_PARAM, STRICT_PARAM, TRUE_PARAM_VAL, LOWQ_PARAM, TRUE_PARAM_VAL);

	/* tests with bogo file */
	params = get_query_params("bogo.jpg", query_string);
	cache_file_path = create_cache_file_path(params);

	assert_equal(check_if_cached(params), NO_ORIG | NO_CACHE);

	/* creating test file */
	test_fd = open(cache_file_path, O_CREAT|O_WRONLY|O_TRUNC);
	assert_not_equal(test_fd, -1);
	close(test_fd);
	
	assert_equal(check_if_cached(params), NO_ORIG);

	/* cleaning up test file */
	assert_not_equal(unlink(cache_file_path), -1);

	free(cache_file_path);
	free_query_params(params);

	/* and now with real file */
	params = get_query_params(IMAGE_TEST_FILE, query_string);
	cache_file_path = create_cache_file_path(params);
	orig_file_path = create_media_file_path(IMAGE_TEST_FILE);

	assert_equal(check_if_cached(params), NO_CACHE);

	/* creating test file */
	test_fd = open(cache_file_path, O_CREAT|O_WRONLY|O_TRUNC);
	assert_not_equal(test_fd, -1);
	close(test_fd);

	assert_equal(check_if_cached(params), MTIME_DIFFER);

	/* now we will set mtime to match original file */
	time_buf.actime = time_buf.modtime = get_file_mtime(orig_file_path);
	assert_not_equal(utime(cache_file_path, &time_buf), -1);

	assert_equal(check_if_cached(params), CACHE_OK);

	/* cleaning up test file */
	assert_not_equal(unlink(cache_file_path), -1);
	
	free(orig_file_path);
	free(cache_file_path);
	free_query_params(params);
}

static void test_write_blob_to_cache() {
	unsigned char *test_blob;
	char *cache_file_path;
	struct query_params *params;

	test_blob = malloc(3000);

	/* tests with bogo file */
	params = get_query_params("blob.test", "");	

	assert_equal(write_blob_to_cache(test_blob, 3000, params), 0);

	free_query_params(params);

	/* now we will try existing original file */
	params = get_query_params(IMAGE_TEST_FILE, "");
	cache_file_path = create_cache_file_path(params);

	assert_not_equal(write_blob_to_cache(test_blob, 3000, params), 0);
	assert_file_size(cache_file_path, 3000);

	/* cleaning up test file */
	assert_not_equal(unlink(cache_file_path), -1);

	free(cache_file_path);
	free_query_params(params);
	free(test_blob);
}

/* serve.c tests */

/* this function will fork and redirect child stdout to stdout_fd pipe end */
static int fork_with_stdout_capture(int *stdout_fd) {
	*stdout_fd = capture_stdout();
	if(!fork())
		return 0;

	/* We restore our local stdout */
	restore_stdout();
	return 1;
}

static void test_serve_from_file() {
	int stdout_fd;
	int status;
	char *media_file_path;

	/* real file */
	media_file_path = create_media_file_path(IMAGE_TEST_FILE);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		status = serve_from_file(media_file_path, OUT_FORMAT_MIME_TYPE);
		if (!status) {
			restore_stdout();
			assert_true_with_message(0, "serve_from_file failed");
		}
		exit(0);
	}

	assert_headers_read(stdout_fd);
	assert_byte_read(stdout_fd, get_file_size(media_file_path));

	close(stdout_fd);
	free(media_file_path);

	/* bogus file */
	if (!fork_with_stdout_capture(&stdout_fd)) {
		status = serve_from_file(media_file_path, OUT_FORMAT_MIME_TYPE);
		if (status) {
			restore_stdout();
			assert_true_with_message(0, "serve_from_file failed");
		}
		exit(0);
	}
	close(stdout_fd);

}

static void test_serve_from_blob() {
	int stdout_fd;
	unsigned char *blob;

	blob = malloc(31666);
	assert_not_equal(blob, 0);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		serve_from_blob(blob, 31666, OUT_FORMAT_MIME_TYPE);
		exit(0);
	}
	free(blob);

	assert_headers_read(stdout_fd);
	assert_byte_read(stdout_fd, 31666);

	close(stdout_fd);
}

static void test_serve_from_cache() {
	int stdout_fd;
	int status;
	unsigned char *blob;
	struct query_params *params;
	char *cache_file_path;
	blob = malloc(3000);

	/* tests with real file */
	params = get_query_params(IMAGE_TEST_FILE, "");	
	cache_file_path = create_cache_file_path(params);

	/* create test cache file - mtime should be set properly */
	assert_equal(write_blob_to_cache(blob, 3000, params), 1);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		status = serve_from_cache(params, OUT_FORMAT_MIME_TYPE);
		if (!status) {
			restore_stdout();
			assert_true_with_message(0, "serve_from_cache failed");
		}
		exit(0);
	}

	assert_headers_read(stdout_fd);
	assert_byte_read(stdout_fd, 3000);

	/* cleaning up test file */
	assert_not_equal(unlink(cache_file_path), -1);

	/* test with wrong mtime */
	/* create test file - mtime should be set to current time */
	assert_equal(write_blob_to_file(blob, 3000, cache_file_path), 1);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		status = serve_from_cache(params, OUT_FORMAT_MIME_TYPE);
		if (status) {
			restore_stdout();
			assert_true_with_message(0, "serve_from_cache failed");
		}
		exit(0);
	}

	/* there should be no cache file as it should be removed */
	assert_equal(unlink(cache_file_path), -1);

	/* test with no cache */
	if (!fork_with_stdout_capture(&stdout_fd)) {
		status = serve_from_cache(params, OUT_FORMAT_MIME_TYPE);
		if (status) {
			restore_stdout();
			assert_true_with_message(0, "serve_from_cache failed");
		}
		exit(0);
	}

	close(stdout_fd);

	free(cache_file_path);
	free_query_params(params);

	/* test with no original file */
	params = get_query_params("bogo.file", "");
	cache_file_path = create_cache_file_path(params);

	/* create test cache file */
	assert_equal(write_blob_to_file(blob, 3000, cache_file_path), 1);

	if (!fork_with_stdout_capture(&stdout_fd)) {
		status = serve_from_cache(params, OUT_FORMAT_MIME_TYPE);
		if (status) {
			restore_stdout();
			assert_true_with_message(0, "serve_from_cache failed");
		}
		exit(0);
	}

	close(stdout_fd);
	free_query_params(params);

	/* there should be no cache file as it should be removed */
	assert_equal(unlink(cache_file_path), -1);
	free(cache_file_path);
	free(blob);
}

static void test_serve_error() {
	int stdout_fd;
	char *media_file_path;

	if (!fork_with_stdout_capture(&stdout_fd)) {
		serve_error();
		exit(0);
	}
	
	media_file_path = create_media_file_path(ERROR_FILE_PATH);

	assert_headers_read(stdout_fd);
	assert_byte_read(stdout_fd, get_file_size(media_file_path));

	close(stdout_fd);
}

static void test_serve_error_message() {
	int stdout_fd;

	if (!fork_with_stdout_capture(&stdout_fd)) {
		serve_error_message();
		exit(0);
	}

	assert_headers_read(stdout_fd);
	assert_byte_read(stdout_fd, strlen(ERROR_FAILBACK_MESSAGE));

	close(stdout_fd);
}

/* setup and teardown */
static void test_setup() {
	debug_start(DEBUG_FILE);
}

static void test_teardown() {
	debug_stop();
}

int main(int argc, char **argv) {
	TestSuite *suite = create_test_suite();

	TestSuite *file_utils_suite = create_test_suite();
	TestSuite *query_string_suite = create_test_suite();
	TestSuite *geometry_math_suite = create_test_suite();
	TestSuite *cgiscaler_suite = create_test_suite();
	TestSuite *cache_suite = create_test_suite();
	TestSuite *serve_suite = create_test_suite();

	add_test(file_utils_suite, test_create_media_file_path);
	add_test(file_utils_suite, test_create_cache_file_path);
	add_test(file_utils_suite, test_check_for_double_dot);
	add_test(file_utils_suite, test_create_cache_dir_struct);
	add_test(file_utils_suite, test_get_file_mtime);
	add_test(file_utils_suite, test_make_file_name_relative);
	add_test(file_utils_suite, test_write_blob_to_file);
	add_suite(suite, file_utils_suite);
	
	add_test(query_string_suite, test_query_string_param);
	add_test(query_string_suite, test_get_query_params);
	add_suite(suite, query_string_suite);

	add_test(geometry_math_suite, test_resize_to_fit_in);
	add_test(geometry_math_suite, test_reduce_filed);
	add_suite(suite, geometry_math_suite);

	add_test(cgiscaler_suite, test_load_image);
	add_test(cgiscaler_suite, test_fit_resize);
	add_test(cgiscaler_suite, test_strict_resize);
	add_test(cgiscaler_suite, test_resize_field_limiting);
	add_test(cgiscaler_suite, test_remove_transparentcy);
	add_test(cgiscaler_suite, test_prepare_blob);
	add_suite(suite, cgiscaler_suite);

	add_test(cache_suite, test_if_cached);
	add_test(cache_suite, test_write_blob_to_cache);
	add_suite(suite, cache_suite);

	add_test(serve_suite, test_serve_from_file);
	add_test(serve_suite, test_serve_from_blob);
	add_test(serve_suite, test_serve_from_cache);
	add_test(serve_suite, test_serve_error);
	add_test(serve_suite, test_serve_error_message);
	add_suite(suite, serve_suite);

	setup(suite, test_setup);
	teardown(suite, test_teardown);

	return run_test_suite(suite, create_text_reporter());
}


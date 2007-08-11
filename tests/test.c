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
#include <sys/wait.h>

#include "../cgreen/cgreen.h"
#include "file_utils.h"
#include "query_string.h"
#include "geometry_math.h"
#include "cgiscaler.h"
#include "cache.h"
#include "serve.h"
#include "runtime_config.h"
#include "commandline.h"
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

/* TODO: Different quantum and different versions of IM returns different strings */
void assert_image_pixel_color(MagickWand *magick_wand, int x, int y, const char *color) {
	PixelWand *pixel_wand;
	MagickBooleanType status;

	pixel_wand = NewPixelWand();
	status = MagickGetImagePixelColor(magick_wand, x, y, pixel_wand);
	assert_not_equal(status, MagickFalse);

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
        struct stat s;
        assert_not_equal_with_message(stat(file_path, &s), -1, "file [%s] does not exist", file_path);
}

void assert_file_not_exists(char *file_path) {
	struct stat s;
        assert_equal_with_message(stat(file_path, &s), -1, "file [%s] exist", file_path);
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
	char *media_file_path;
	struct dimensions size;

	size.w = 100;
	size.h = 100;

	media_file_path = create_media_file_path("non_existing_file.jpg");

	magick_wand = load_image("non_existing_file.jpg", size);
	assert_equal(magick_wand, 0);

	free(media_file_path);
	media_file_path = create_media_file_path(IMAGE_TEST_FILE);

	magick_wand = load_image(media_file_path, size);
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
	free(media_file_path);
}

static void test_fit_resize() {
	char *media_file_path;
	MagickWand *image_ping;
	MagickWand *image;
	struct dimensions a, img, b;
	struct dimensions size;

	size.w = 0;
	size.h = 0;

	a.w = 100;
	a.h = 200;

	media_file_path = create_media_file_path(IMAGE_TEST_FILE);

	image_ping = ping_image(media_file_path);
	assert_not_equal(image_ping, 0);

	img = get_image_size(image_ping);

	free_image(image_ping);

	image = fit_resize(media_file_path, a);
	assert_not_equal(image, 0);

	/* as we know this works from previous tests we can calculate image size to see if it works */
	b = resize_to_fit_in(img, a);
	img = get_image_size(image);

	assert_equal(img.w, b.w);
	assert_equal(img.h, b.h);

	free_image(image);
	free(media_file_path);
}

static void test_strict_resize() {
	char *media_file_path;
	MagickWand *image;
	struct dimensions a;
	struct dimensions size;

	size.w = 0;
	size.h = 0;

	a.w = 100;
	a.h = 200;

	media_file_path = create_media_file_path(IMAGE_TEST_FILE);

	image = strict_resize(media_file_path, a);
	assert_not_equal(image, 0);

	size = get_image_size(image);
	
	assert_equal(size.w, a.w);
	assert_equal(size.h, a.h);

	free_image(image);
	free(media_file_path);
}

static void test_resize_field_limiting() {
	char *media_file_path;
	MagickWand *image;
	struct dimensions a;
	struct dimensions img;

	/* we will re-size a little bit so resizing will take effect */
	a.w = IMAGE_TEST_FILE_WIDTH - 10;
	a.h = IMAGE_TEST_FILE_HEIGHT - 10;

	/* MAX_PIXEL_NO is set to lower then a.w * a.h */
	if (a.w * a.h > MAX_PIXEL_NO)
		assert_true(1);
	else
		assert_true(0);

	media_file_path = create_media_file_path(IMAGE_TEST_FILE);

	/* fit_resize */
	image = fit_resize(media_file_path, reduce_filed(a, MAX_PIXEL_NO));
	assert_not_equal(image, 0);

	img = get_image_size(image);

	assert_not_equal(img.w, a.w);
	assert_not_equal(img.h, a.h);
	assert_equal_low_precision(img.w * img.h, MAX_PIXEL_NO, 0.1);

	free_image(image);

	/* strict_resize */
	image = strict_resize(media_file_path, reduce_filed(a, MAX_PIXEL_NO));
	assert_not_equal(image, 0);

	img = get_image_size(image);

	assert_not_equal(img.w, a.w);
	assert_not_equal(img.h, a.h);
	assert_equal_low_precision(img.w * img.h, MAX_PIXEL_NO, 0.1);

	free_image(image);
	free(media_file_path);
}

static void test_remove_transparentcy() {
	char *media_file_path;
	MagickWand *image;
	struct dimensions a;

	/* we re-size just 1 pixel so we don't loose our transparent region at (10,10) */
	a.w = IMAGE_TEST_FILE_WIDTH - 1;
	a.h = IMAGE_TEST_FILE_HEIGHT - 1;

	media_file_path = create_media_file_path(IMAGE_TEST_FILE);

	image = load_image(media_file_path, a);
	assert_not_equal(image, 0);

	/* assert we have our test image loaded OK */
	assert_image_pixel_color(image, IMAGE_TEST_FILE_WIDTH - 1, IMAGE_TEST_FILE_HEIGHT - 1, "rgb240,0,255");

	/* we assert that we don't have transparent pixel */
	assert_image_pixel_alpha(image, 10, 10, 1.0);
	/* and color is DEFAULT_BACKGROUND_COLOR_MAGICK_STR */
	assert_image_pixel_color(image, 10, 10, DEFAULT_BACKGROUND_COLOR_MAGICK_STR);

	free_image(image);
	free(media_file_path);
}

static void test_prepare_blob() {
	char *media_file_path;
	MagickWand *image;
	struct dimensions a;
	size_t blob_len;
	unsigned char *blob;

	a.w = 100;
	a.h = 200;

	media_file_path = create_media_file_path(IMAGE_TEST_FILE);

	image = strict_resize(media_file_path, a);
	assert_not_equal(image, 0);

	/* we are preparing JPEG data */
	blob = prepare_blob(image, 70, &blob_len, "JPG");
	assert_not_equal(blob, 0);
	assert_not_equal(blob_len, 0);

	/* JPEG data magic number */
	assert_equal(blob[0], 0xff);
	assert_equal(blob[1], 0xd8);

	free_blob(blob);
	free_image(image);
	free(media_file_path);
}

/* cache.c tests */
static void test_if_cached() {
	char *cache_file_path;
	char *media_file_path;
	int test_fd;
	struct utimbuf time_buf;

	/* tests with bogo file */
	media_file_path = create_media_file_path("bogo.jpg");
	cache_file_path = create_cache_file_path("bogo.jpg", OUT_FORMAT_EXTENSION, 0, 0, 0, 0);

	assert_equal(check_if_cached(media_file_path, cache_file_path), NO_ORIG | NO_CACHE);

	/* creating test file */
	test_fd = open(cache_file_path, O_CREAT|O_WRONLY|O_TRUNC);
	assert_not_equal(test_fd, -1);
	close(test_fd);
	
	assert_equal(check_if_cached(media_file_path, cache_file_path), NO_ORIG);

	/* cleaning up test file */
	assert_not_equal(unlink(cache_file_path), -1);

	free(cache_file_path);
	free(media_file_path);

	/* and now with real file */
	media_file_path = create_media_file_path(IMAGE_TEST_FILE);
	cache_file_path = create_cache_file_path(IMAGE_TEST_FILE, OUT_FORMAT_EXTENSION, 0, 0, 0, 0);

	assert_equal(check_if_cached(media_file_path, cache_file_path), NO_CACHE);

	/* creating test file */
	test_fd = open(cache_file_path, O_CREAT|O_WRONLY|O_TRUNC);
	assert_not_equal(test_fd, -1);
	close(test_fd);

	assert_equal(check_if_cached(media_file_path, cache_file_path), MTIME_DIFFER);

	/* now we will set mtime to match original file */
	time_buf.actime = time_buf.modtime = get_file_mtime(media_file_path);
	assert_not_equal(utime(cache_file_path, &time_buf), -1);

	assert_equal(check_if_cached(media_file_path, cache_file_path), CACHE_OK);

	/* cleaning up test file */
	assert_not_equal(unlink(cache_file_path), -1);
	
	free(media_file_path);
	free(cache_file_path);
}

static void test_write_blob_to_cache() {
	unsigned char *test_blob;
	char *cache_file_path;
	char *media_file_path;

	test_blob = malloc(3000);

	/* tests with bogo file */
	media_file_path = create_media_file_path("blob.test");
	cache_file_path = create_cache_file_path("blob.test", OUT_FORMAT_EXTENSION, 0, 0, 0, 0);

	assert_equal(write_blob_to_cache(test_blob, 3000, media_file_path, cache_file_path), 0);

	free(media_file_path);
	free(cache_file_path);

	/* now we will try existing original file */
	media_file_path = create_media_file_path(IMAGE_TEST_FILE);
	cache_file_path = create_cache_file_path(IMAGE_TEST_FILE, OUT_FORMAT_EXTENSION, 0, 0, 0, 0);

	assert_not_equal(write_blob_to_cache(test_blob, 3000, media_file_path, cache_file_path), 0);
	assert_file_size(cache_file_path, 3000);

	/* cleaning up test file */
	assert_not_equal(unlink(cache_file_path), -1);

	free(media_file_path);
	free(cache_file_path);
	free(test_blob);
}


/* runtime_config.c tests */
static void test_alloc_default_runtime_config() {
	struct runtime_config *config;

	config = alloc_default_runtime_config();

	assert_not_equal(config, 0);
	assert_equal(config->file_name, 0);
	assert_equal(config->size.w, DEFAULT_WIDTH);
	assert_equal(config->size.h, DEFAULT_HEIGHT);
	assert_equal(config->strict, DEFAULT_STRICT);
	assert_equal(config->quality, DEFAULT_QUALITY);
	assert_equal(config->no_cache, DEFAULT_NO_CACHE);
	assert_equal(config->no_serve, DEFAULT_NO_SERVE);
	assert_equal(config->no_headers, DEFAULT_NO_HEADERS);

	free_runtime_config(config);

	config = alloc_default_runtime_config();
	config->file_name = malloc(6);
	assert_not_equal(config->file_name, 0);

	/* for memory leak test (valgrind) - file name should be freed */
	free_runtime_config(config);
}

/* commandline.c tests */
static void test_apply_commandline_config() {
	struct runtime_config *config;
	char *args1[] = { "test", "-w", "100", "-h", "200" };
	char *args2[] = { "test", "-w", "100", "-s", "true", "-nc", "false", "-ns", "true", "-nh", "dsfa", "abc/e/f.jpg" };
	char *args3[] = { "test", "file.gif", "-h", "300", "-h", "100" };


	config = alloc_default_runtime_config();

	apply_commandline_config(config, 5, args1);

	assert_equal(config->file_name, 0);
	assert_equal(config->size.w, 100);
	assert_equal(config->size.h, 200);
	assert_equal(config->strict, DEFAULT_STRICT);
	assert_equal(config->quality, DEFAULT_QUALITY);
	assert_equal(config->no_cache, DEFAULT_NO_CACHE);
	assert_equal(config->no_serve, DEFAULT_NO_SERVE);
	assert_equal(config->no_headers, DEFAULT_NO_HEADERS);

	free_runtime_config(config);

	config = alloc_default_runtime_config();

	apply_commandline_config(config, 12, args2);

	assert_string_equal(config->file_name, "abc/e/f.jpg");
	assert_equal(config->size.w, 100);
	assert_equal(config->size.h, DEFAULT_HEIGHT);
	assert_equal(config->strict, 1);
	assert_equal(config->quality, DEFAULT_QUALITY);
	assert_equal(config->no_cache, 0);
	assert_equal(config->no_serve, 1);
	assert_equal(config->no_headers, 0);

	apply_commandline_config(config, 6, args3);

	assert_string_equal(config->file_name, "file.gif");
	assert_equal(config->size.w, 100);
	assert_equal(config->size.h, 100);
	assert_equal(config->strict, 1);
	assert_equal(config->quality, DEFAULT_QUALITY);
	assert_equal(config->no_cache, 0);
	assert_equal(config->no_serve, 1);
	assert_equal(config->no_headers, 0);

	free_runtime_config(config);
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

static void test_get_query_string_config() {
	struct runtime_config *config;
	char test_query_string[256];

	config = alloc_default_runtime_config();

	/* testing defaults */
	apply_query_string_config(config, "", "");

	assert_not_equal(config, 0);
	assert_equal(config->file_name, 0);
	assert_equal(config->size.w, DEFAULT_WIDTH);
	assert_equal(config->size.h, DEFAULT_HEIGHT);
	assert_equal(config->strict, DEFAULT_STRICT);
	assert_equal(config->quality, DEFAULT_QUALITY);

	apply_query_string_config(config, "/some/path/funny.jpeg", "");
	assert_not_equal(config, 0);
	assert_string_equal(config->file_name, "some/path/funny.jpeg");
	assert_equal(config->size.w, DEFAULT_WIDTH);
	assert_equal(config->size.h, DEFAULT_HEIGHT);
	assert_equal(config->strict, DEFAULT_STRICT);
	assert_equal(config->quality, DEFAULT_QUALITY);


	snprintf(test_query_string, 256, "%s=123&%s=213&beer=czech_lager&%s=%s&%s=%s", QUERY_WIDTH_PARAM, QUERY_HEIGHT_PARAM, QUERY_STRICT_PARAM, QUERY_TRUE_VAL, QUERY_LOWQ_PARAM, QUERY_TRUE_VAL);

	apply_query_string_config(config, "/some/path/funny.jpeg", test_query_string);
	assert_string_equal(config->file_name, "some/path/funny.jpeg");
	assert_equal(config->size.w, 123);
	assert_equal(config->size.h, 213);
	assert_equal(config->strict, 1);
	assert_equal(config->quality, LOWQ_QUALITY);

	free_runtime_config(config);
	config = alloc_default_runtime_config();

	snprintf(test_query_string, 256, "beer=czech_lager&%s=%s&%s=%s", QUERY_STRICT_PARAM, "xbrna", QUERY_LOWQ_PARAM, "false");

	apply_query_string_config(config, "/some/path/funny.jpeg", test_query_string);
	assert_string_equal(config->file_name, "some/path/funny.jpeg");
	assert_equal(config->size.w, DEFAULT_WIDTH);
	assert_equal(config->size.h, DEFAULT_HEIGHT);
	assert_equal(config->strict, 0);
	assert_equal(config->quality, DEFAULT_QUALITY);

	snprintf(test_query_string, 256, "beer=czech_lager&%s=%s&%s=%s", QUERY_STRICT_PARAM, "xbrna", QUERY_LOWQ_PARAM, QUERY_TRUE_VAL);

	apply_query_string_config(config, "///some/path/funn/y2.jpeg", test_query_string);
	assert_string_equal(config->file_name, "some/path/funn/y2.jpeg");
	assert_equal(config->size.w, DEFAULT_WIDTH);
	assert_equal(config->size.h, DEFAULT_HEIGHT);
	assert_equal(config->strict, 0);
	assert_equal(config->quality, LOWQ_QUALITY);

	free_runtime_config(config);
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

	TestSuite *geometry_math_suite = create_test_suite();
	TestSuite *cgiscaler_suite = create_test_suite();
	TestSuite *cache_suite = create_test_suite();
	TestSuite *runtime_config_suite = create_test_suite();
	TestSuite *commandline_suite = create_test_suite();
	TestSuite *query_string_suite = create_test_suite();

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

	add_test(runtime_config_suite, test_alloc_default_runtime_config);
	add_suite(suite, runtime_config_suite);

	add_test(commandline_suite, test_apply_commandline_config);
	add_suite(suite, commandline_suite);

	add_test(query_string_suite, test_query_string_param);
	add_test(query_string_suite, test_get_query_string_config);
	add_suite(suite, query_string_suite);

	setup(suite, test_setup);
	teardown(suite, test_teardown);

	return run_test_suite(suite, create_text_reporter());
}


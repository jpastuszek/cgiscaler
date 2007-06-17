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
#include <unistd.h>
#include <string.h>
#include <magick/MagickCore.h>

#include "../cgreen/cgreen.h"
#include "file_utils.h"
#include "query_string.h"
#include "geometry_math.h"
#include "cgiscaler.h"
#include "cache.h"
#include "test_config.h"

#include "debug.h"

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

	assert_true_with_message(true, "pixel alpha shoud be [%f] but is [%f]", alpha, tested_alpha);

	DestroyPixelWand(pixel_wand);
}

void assert_dir_exists(char *dir_path) {
	struct stat s;
	assert_not_equal_with_message(stat(dir_path, &s), -1, "dir [%s] does not exist", dir_path);
	assert_true_with_message(S_ISDIR(s.st_mode), "dir [%s] is not a directory", dir_path);
}

/* file_utils.c tests */
static void test_create_media_file_path() {
	char *path;
	/* we don't get too much in to as it would reproduce funcion it self :D */
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

static void test_check_for_double_dot() {
	char *test = make_file_name_relative("///00/ff/test.jpg");
	char *dot_test = make_file_name_relative("///00/../ff/test.jpg");

	assert_equal(check_for_double_dot(test), 0);
	assert_equal(check_for_double_dot(dot_test), 1);
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
	struct dimmensions a, b, c;

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
	struct dimmensions a, b;

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
/* TODO: ./configure should gatere IM version and if newer versions are used use instead:	
	assert_equal(MagickGetImageProperty(magick_wand, "comment"), 0);
*/

	free_image(magick_wand);
}

static void test_fit_resize() {
	MagickWand *magick_wand;
	struct dimmensions a, img, b;

	a.w = 100;
	a.h = 200;

	magick_wand = load_image(IMAGE_TEST_FILE);
	assert_not_equal(magick_wand, 0);

	img.w = MagickGetImageWidth(magick_wand);
	img.h = MagickGetImageHeight(magick_wand);

	magick_wand = fit_resize(magick_wand, a);
	assert_not_equal(magick_wand, 0);

	/* as we know this works from previous tests we can calculete image size to see if it works */
	b = resize_to_fit_in(img, a);

	assert_equal(MagickGetImageWidth(magick_wand), b.w);
	assert_equal(MagickGetImageHeight(magick_wand), b.h);

	free_image(magick_wand);
}

static void test_strict_resize() {
	MagickWand *magick_wand;
	struct dimmensions a;

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
	struct dimmensions a;

	/* we will resize a little bit so resizing will take effect */
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
	struct dimmensions a;

	/* we resize just 1 pixel so we don't loose our transparent region at (10,10) */
	a.w = IMAGE_TEST_FILE_WIDTH - 1;
	a.h = IMAGE_TEST_FILE_HEIGHT - 1;

	magick_wand = load_image(IMAGE_TEST_FILE);
	assert_not_equal(magick_wand, 0);

	/* assert we have our test image loaded ok */
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
	struct dimmensions a;
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

	/* JPEG data magick number */
	assert_equal(blob[0], 0xff);
	assert_equal(blob[1], 0xd8);

	free_blob(blob);
}

/* cache.c tests */
static void test_if_cached() {
	char query_string[256];
	char *cache_file;
	struct query_params *params;	

	snprintf(query_string, 256, "%s=123&%s=213&beer=czech_lager&%s=%s&%s=%s", WIDTH_PARAM, HEIGHT_PARAM, STRICT_PARAM, TRUE_PARAM_VAL, LOWQ_PARAM, TRUE_PARAM_VAL);

	params = get_query_params("test.jpg", query_string);
	cache_file = create_cache_file_path(params);

	

	free_query_params(params);
	free(cache_file);
}

/* seturp and teardown */
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

	add_test(file_utils_suite, test_create_media_file_path);
	add_test(file_utils_suite, test_create_cache_file_path);
	add_test(file_utils_suite, test_create_cache_dir_struct);
	add_test(file_utils_suite, test_get_file_mtime);
	add_test(file_utils_suite, test_make_file_name_relative);
	add_test(file_utils_suite, test_check_for_double_dot);
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
	add_suite(suite, cache_suite);

	setup(suite, test_setup);
	teardown(suite, test_teardown);

	return run_test_suite(suite, create_text_reporter());
}


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

#include "../cgreen/cgreen.h"
#include "query_string.h"
#include "geometry_math.h"
#include "cgiscaler.h"
#include "test_config.h"

#include "debug.h"

#define PATTERN_TEST_FILE "quick_gimp_pattern_test.png"

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

/* query_string tests */
static void test_query_string_param() {
	char *prog = get_query_string_param("name=kaz&prog=cgiscaler&year=2007","prog");
	char *name = get_query_string_param("name=kaz&prog=cgiscaler&year=2007","name");
	char *year = get_query_string_param("name=kaz&prog=cgiscaler&year=2007","year");

	assert_string_equal(prog, "cgiscaler");
	free(prog);
	assert_string_equal(name, "kaz");
	free(name);
	assert_string_equal(year, "2007");
	free(year);
}

static void test_process_file_name() {
	char *test1 = process_file_name("///00/ff/test.jpg");
	char *dot_test = process_file_name("///00/../ff/test.jpg");
	
	assert_string_equal(test1, "00/ff/test.jpg");
	free(test1);

	assert_equal(dot_test, 0);
}

static void test_get_query_params() {
	struct query_params *params;
	char test_query_string[256];

	params = get_query_params();
	assert_equal(params, 0);

	snprintf(test_query_string, 256, "%s=123&%s=213&beer=czech_lager&%s=%s&%s=%s", WIDTH_PARAM, HEIGHT_PARAM, STRICT_PARAM, TRUE_PARAM_VAL, LOWQ_PARAM, TRUE_PARAM_VAL);

	setenv("QUERY_STRING", test_query_string, 1);
	params = get_query_params();
	assert_equal(params, 0);

	setenv("PATH_INFO", "/some/path/funny.jpeg", 1);

	params = get_query_params();
	assert_not_equal(params, 0);
	assert_string_equal(params->file_name, "some/path/funny.jpeg");
	assert_equal(params->size.w, 123);
	assert_equal(params->size.h, 213);
	assert_equal(params->strict, 1);
	assert_equal(params->lowq, 1);
	free_query_params(params);

	snprintf(test_query_string, 256, "beer=czech_lager&%s=%s&%s=%s", STRICT_PARAM, "xbrna", LOWQ_PARAM, "false");

	setenv("QUERY_STRING", test_query_string, 1);

	params = get_query_params();
	assert_not_equal(params, 0);
	assert_string_equal(params->file_name, "some/path/funny.jpeg");
	assert_equal(params->size.w, 0);
	assert_equal(params->size.h, 0);
	assert_equal(params->strict, 0);
	assert_equal(params->lowq, 0);
	free_query_params(params);

	snprintf(test_query_string, 256, "beer=czech_lager&%s=%s&%s=%s", STRICT_PARAM, "xbrna", LOWQ_PARAM, TRUE_PARAM_VAL);

	setenv("QUERY_STRING", test_query_string, 1);

	params = get_query_params();
	assert_not_equal(params, 0);
	assert_string_equal(params->file_name, "some/path/funny.jpeg");
	assert_equal(params->size.w, 0);
	assert_equal(params->size.h, 0);
	assert_equal(params->strict, 0);
	assert_equal(params->lowq, 1);
	free_query_params(params);
}

/* geometry_math tests */
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

/* cgiscaler image tests */

static void test_load_image() {
	MagickWand *wand;

	wand = load_image("non_existing_file.jpg");
	assert_equal(wand, 0);

	wand = load_image(PATTERN_TEST_FILE);
	assert_not_equal(wand, 0);
	
	if (wand)
		free_image(wand);

	
}

int main(int argc, char **argv) {
	TestSuite *suite = create_test_suite();

	TestSuite *query_string_suite = create_test_suite();
	TestSuite *geometry_math_suite = create_test_suite();
	TestSuite *cgiscaler_suite = create_test_suite();

	add_test(query_string_suite, test_query_string_param);
	add_test(query_string_suite, test_process_file_name);
	add_test(query_string_suite, test_get_query_params);
	add_suite(suite, query_string_suite);

	add_test(geometry_math_suite, test_resize_to_fit_in);
	add_test(geometry_math_suite, test_reduce_filed);
	add_suite(suite, geometry_math_suite);

	add_test(cgiscaler_suite, test_load_image);
	add_suite(suite, cgiscaler_suite);

/* debug just in case of hmm... problems */
//	debug_start(0);

	return run_test_suite(suite, create_text_reporter());

//	debug_stop();
}


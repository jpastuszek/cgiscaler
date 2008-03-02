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
#include "runtime_config.h"
#include "debug.h"
#include "query_string.h"

extern struct runtime_config *runtime_config;
extern struct query_string_config *query_string_config;


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
	char test_query_string[256];

	output_config = alloc_default_output_config();
	simple_query_string_config = alloc_default_simple_query_string_config();

	/* testing defaults */
	apply_simple_query_string_config("", "");

	assert_not_equal(output_config, 0);
	assert_equal(output_config->file_name, 0);
	assert_equal(output_config->size.w, DEFAULT_WIDTH);
	assert_equal(output_config->size.h, DEFAULT_HEIGHT);
	assert_equal(output_config->scale_method, DEFAULT_STRICT);
	assert_equal(output_config->quality, DEFAULT_QUALITY);

	apply_simple_query_string_config("/some/path/funny.jpeg", "");
	assert_not_equal(output_config, 0);
	assert_string_equal(output_config->file_name, "some/path/funny.jpeg");
	assert_equal(output_config->size.w, DEFAULT_WIDTH);
	assert_equal(output_config->size.h, DEFAULT_HEIGHT);
	assert_equal(output_config->scale_method, DEFAULT_STRICT);
	assert_equal(output_config->quality, DEFAULT_QUALITY);


	snprintf(test_query_string, 256, "%s=123&%s=213&beer=czech_lager&%s=%s&%s=%s", QUERY_WIDTH_PARAM, QUERY_HEIGHT_PARAM, QUERY_STRICT_PARAM, QUERY_TRUE_VAL, QUERY_LOWQ_PARAM, QUERY_TRUE_VAL);

	apply_simple_query_string_config("/some/path/funny.jpeg", test_query_string);
	assert_string_equal(output_config->file_name, "some/path/funny.jpeg");
	assert_equal(output_config->size.w, 123);
	assert_equal(output_config->size.h, 213);
	assert_equal(output_config->scale_method, SM_STRICT);
	assert_equal(output_config->quality, LOWQ_QUALITY);

	free_output_config(output_config);
	free_simple_query_string_config(simple_query_string_config);

	output_config = alloc_default_output_config();
	simple_query_string_config = alloc_default_simple_query_string_config();

	snprintf(test_query_string, 256, "beer=czech_lager&%s=%s&%s=%s", QUERY_STRICT_PARAM, "xbrna", QUERY_LOWQ_PARAM, "false");

	apply_simple_query_string_config("/some/path/funny.jpeg", test_query_string);
	assert_string_equal(output_config->file_name, "some/path/funny.jpeg");
	assert_equal(output_config->size.w, DEFAULT_WIDTH);
	assert_equal(output_config->size.h, DEFAULT_HEIGHT);
	assert_equal(output_config->scale_method, DEFAULT_STRICT);
	assert_equal(output_config->quality, DEFAULT_QUALITY);

	snprintf(test_query_string, 256, "beer=czech_lager&%s=%s&%s=%s", QUERY_STRICT_PARAM, "xbrna", QUERY_LOWQ_PARAM, QUERY_TRUE_VAL);

	apply_simple_query_string_config("///some/path/funn/y2.jpeg", test_query_string);
	assert_string_equal(output_config->file_name, "some/path/funn/y2.jpeg");
	assert_equal(output_config->size.w, DEFAULT_WIDTH);
	assert_equal(output_config->size.h, DEFAULT_HEIGHT);
	assert_equal(output_config->scale_method, DEFAULT_STRICT);
	assert_equal(output_config->quality, LOWQ_QUALITY);

	snprintf(test_query_string, 256, "%s=104&%s=137&%s=%s", QUERY_WIDTH_PARAM, QUERY_HEIGHT_PARAM, QUERY_STRICT_PARAM, QUERY_FALSE_VAL);

	apply_simple_query_string_config("photo04/3d/03/0b64a0af1869.jpg", test_query_string);
	assert_string_equal(output_config->file_name, "photo04/3d/03/0b64a0af1869.jpg");
	assert_equal(output_config->size.w, 104);
	assert_equal(output_config->size.h, 137);
	assert_equal(output_config->scale_method, SM_FIT);
	assert_equal(output_config->quality, LOWQ_QUALITY);


	/* set switches to true */
	snprintf(test_query_string, 256, "%s=%s&%s=%s", QUERY_STRICT_PARAM, QUERY_TRUE_VAL, QUERY_LOWQ_PARAM, QUERY_TRUE_VAL);
	apply_simple_query_string_config("test", test_query_string);

	assert_equal(output_config->scale_method, SM_STRICT);
	assert_equal(output_config->quality, LOWQ_QUALITY);

	/* now try some undefined valuses */
	snprintf(test_query_string, 256, "%s=%s&%s=%s", QUERY_STRICT_PARAM, "abc", QUERY_LOWQ_PARAM, "0");
	apply_simple_query_string_config("test", test_query_string);

	/* nothing should change */
	assert_equal(output_config->scale_method, SM_STRICT);
	assert_equal(output_config->quality, LOWQ_QUALITY);

	/* false swiches */
	snprintf(test_query_string, 256, "%s=%s&%s=%s", QUERY_STRICT_PARAM, QUERY_FALSE_VAL, QUERY_LOWQ_PARAM, QUERY_FALSE_VAL);
	apply_simple_query_string_config("test", test_query_string);

	assert_equal(output_config->scale_method, SM_FIT);
	assert_equal(output_config->quality, DEFAULT_QUALITY);

	/* try undefined again */
	snprintf(test_query_string, 256, "%s=%s&%s=%s", QUERY_STRICT_PARAM, "abc", QUERY_LOWQ_PARAM, "0");
	apply_simple_query_string_config("test", test_query_string);

	/* all should be unchanged */
	assert_equal(output_config->scale_method, SM_FIT);
	assert_equal(output_config->quality, DEFAULT_QUALITY);

	free_output_config(output_config);
	free_simple_query_string_config(simple_query_string_config);
}


/* setup and teardown */
static void test_setup() {
	debug_start(DEBUG_FILE);
}

static void test_teardown() {
	debug_stop();
}

int main(int argc, char **argv) {

	TestSuite *query_string_suite = create_named_test_suite(__FILE__);;

	add_test(query_string_suite, test_query_string_param);
	add_test(query_string_suite, test_get_query_string_config);

	setup(query_string_suite, test_setup);
	teardown(query_string_suite, test_teardown);

	return run_test_suite(query_string_suite, create_text_reporter());
}


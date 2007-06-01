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
#include "../src/query_string.h"
#include "../src/config.h"


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
}

int main(int argc, char **argv) {
	TestSuite *suite = create_test_suite();
	add_test(suite, test_query_string_param);
	add_test(suite, test_process_file_name);
	add_test(suite, test_get_query_params);
	return run_test_suite(suite, create_text_reporter());
}


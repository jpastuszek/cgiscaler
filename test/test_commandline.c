/***************************************************************************
 *   Copyright (C) 2007, 2008 by Jakub Pastuszek   *
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

#include "../cgreen/cgreen.h"
#include "runtime_config.h"
#include "defaults.h"
#include "debug.h"
#include "commandline.h"
#include "scaler.h"

/* commandline.c tests */
static void test_apply_commandline_config() {
	alloc_default_config();

	/* if I change this to use config defines... */
	char *args1[] = { "test", "-w", "100", "-h", "200" };
	char *args2[] = { "test", "-w", "100", "-s", "-S", "-i", "abc/e/f.jpg" };
	char *args3[] = { "test", "-i", "file.gif", "-h", "300", "-h", "100" };
	char *all_true[] = { "test", "-w", "100", "-s", "-l", "-C", "-S", "-H" };

	/* default values already set by test_setup */

	apply_commandline_config(5, args1);

	assert_equal(output_config->file_name, 0);
	assert_equal(output_config->size.w, 100);
	assert_equal(output_config->size.h, 200);
	assert_equal(output_config->scale_method, DEFAULT_STRICT);
	assert_equal(output_config->quality, NORMAL_QUALITY_VALUE);
	assert_equal(operation_config->no_cache, NO_CACHE);
	assert_equal(operation_config->no_serve, NO_SERVE);
	assert_equal(operation_config->no_headers, NO_HEADERS);

	free_operation_config(operation_config);
	free_output_config(output_config);

	output_config = alloc_default_output_config();
	operation_config = alloc_default_operation_config();

	apply_commandline_config(7, args2);

	assert_string_equal(output_config->file_name, "abc/e/f.jpg");
	assert_equal(output_config->size.w, 100);
	assert_equal(output_config->size.h, HEIGHT);
	assert_equal(output_config->scale_method, SM_STRICT);
	assert_equal(output_config->quality, NORMAL_QUALITY_VALUE);
	assert_equal(operation_config->no_cache, 0);
	assert_equal(operation_config->no_serve, 1);
	assert_equal(operation_config->no_headers, 0);

	apply_commandline_config(7, args3);

	assert_string_equal(output_config->file_name, "file.gif");
	assert_equal(output_config->size.w, 100);
	assert_equal(output_config->size.h, 100);
	assert_equal(output_config->scale_method, SM_STRICT);
	assert_equal(output_config->quality, NORMAL_QUALITY_VALUE);
	assert_equal(operation_config->no_cache, 0);
	assert_equal(operation_config->no_serve, 1);
	assert_equal(operation_config->no_headers, 0);

	apply_commandline_config(8, all_true);

	assert_string_equal(output_config->file_name, "file.gif");
	assert_equal(output_config->size.w, 100);
	assert_equal(output_config->size.h, 100);
	assert_equal(output_config->scale_method, SM_STRICT);
	assert_equal(output_config->quality, LOW_QUALITY_VALUE);
	assert_equal(operation_config->no_cache, 1);
	assert_equal(operation_config->no_serve, 1);
	assert_equal(operation_config->no_headers, 1);

	free_config();
}

static void test_output_geometry() {
	char *good_args[] = {"test", "-h", "123", "-w", "666" };
	int good_args_len = 5;

	char *no_args[] = {"test"};

	char *some_args[] = {"test", "-w", "25"};
	int some_args_len = 3;

	alloc_default_config();

	apply_commandline_config(good_args_len, good_args);
	assert_equal(output_config->size.h, 123);
	assert_equal(output_config->size.w, 666);

	apply_commandline_config(1, no_args);
	assert_equal(output_config->size.h, 123);
	assert_equal(output_config->size.w, 666);

	apply_commandline_config(some_args_len, some_args);
	assert_equal(output_config->size.h, 123);
	assert_equal(output_config->size.w, 25);

	free_config();
}

static void test_output_format() {
	char *good_args[] = {"test", "--out-format", "JPG", "--failback-mime-type", "image/gif" };
	int good_args_len = 5;

	alloc_default_config();

	apply_commandline_config(good_args_len, good_args);
	assert_string_equal(output_config->format->format, "JPG");
	assert_string_equal(output_config->format->mime_type, "image/jpeg");
	assert_string_equal(output_config->fail_mime_type, "image/gif");

	free_config();
}

static void test_simple_cgi_query_parameters() {
	char *good_args[] = {"test", "--cgi-width", "width", "--cgi-height", "height", "--cgi-strict", "str", "--cgi-low-quality", "lq" };
	int good_args_len = 9;

	alloc_default_config();

	apply_commandline_config(good_args_len, good_args);
	assert_string_equal(simple_query_string_config->query_width_param, "width");
	assert_string_equal(simple_query_string_config->query_height_param, "height");
	assert_string_equal(simple_query_string_config->query_strict_param, "str");
	assert_string_equal(simple_query_string_config->query_lowq_param, "lq");

	free_config();
}

static void test_simple_cgi_query_values() {
	char *good_args[] = {"test", "--cgi-true", "yes", "--cgi-false", "no"};
	int good_args_len = 5;

	alloc_default_config();

	apply_commandline_config(good_args_len, good_args);
	assert_string_equal(simple_query_string_config->query_true_param, "yes");
	assert_string_equal(simple_query_string_config->query_false_param, "no");

	free_config();
}

static void test_simple_cgi_query_defaults() {
	char *good_args[] = {"test", "--strict-resize", "--low-quality", "--file-name", "test/car.gif", "--low-quality-value", "33", "--normal-quality-value", "77" };
	int good_args_len =9;

	char *good_args_lq[] = {"test", "--strict-resize",  "--file-name", "test/car.gif", "--low-quality-value", "22", "--normal-quality-value", "77", "--low-quality"};

	alloc_default_config();

	apply_commandline_config(good_args_len, good_args);
	assert_equal(output_config->scale_method, SM_STRICT);
	assert_equal(output_config->quality, simple_query_string_config->low_quality_value);
	assert_string_equal(output_config->file_name, "test/car.gif");
	assert_equal(simple_query_string_config->low_quality_value, 33);
	assert_equal(simple_query_string_config->default_quality_value, 77);

	free_config();

	alloc_default_config();

	apply_commandline_config(good_args_len, good_args_lq);
	assert_equal(output_config->quality, simple_query_string_config->low_quality_value);
	assert_equal(simple_query_string_config->low_quality_value, 22);

	free_config();
}

static void test_storage() {
	char *good_args[] = {"test", "--media-dir", "/blah/bleh", "--cache-dir", "/waga/chigi"};
	int good_args_len = 5;

	alloc_default_config();

	apply_commandline_config(good_args_len, good_args);
	assert_string_equal(storage_config->media_directory, "/blah/bleh");
	assert_string_equal(storage_config->cache_directory, "/waga/chigi");

	free_config();
}

static void test_resize_filters() {
	char *good_args[] = {"test", "--scaling-filter", "BoxFilter", "--blur-factor", "0.5"};
	int good_args_len = 5;

	alloc_default_config();

	apply_commandline_config(good_args_len, good_args);
	assert_equal(output_config->scaling_filter, BoxFilter);
	assert_equal(output_config->blur_factor, 0.5);

	free_config();
}

static void test_transparent_image_handling() {
	char *good_args[] = {"test", "--no-remove-transparency", "--transparency-colour", "#abcdef"};
	int good_args_len = 4;

	alloc_default_config();

	assert_equal(output_config->remove_transparency, 1);

	apply_commandline_config(good_args_len, good_args);
	assert_equal(output_config->remove_transparency, 0);
	assert_string_equal(output_config->transparency_replacement_color, "#abcdef");

	free_config();
}

static void test_general_operation() {
	char *good_args[] = {"test", "--no-serve", "--no-headers", "--no-cache"};
	int good_args_len = 4;

	alloc_default_config();

	assert_equal(operation_config->no_serve, 0);
	assert_equal(operation_config->no_headers, 0);
	assert_equal(operation_config->no_cache, 0);

	apply_commandline_config(good_args_len, good_args);
	assert_equal(operation_config->no_serve, 1);
	assert_equal(operation_config->no_headers, 1);
	assert_equal(operation_config->no_cache, 1);

	free_config();
}

static void test_error_handling() {
	char *good_args[] = {"test", "--error-file", "photos/error_file.tiff", "--error-message", "404: not found"};
	int good_args_len = 5;

	alloc_default_config();

	apply_commandline_config(good_args_len, good_args);
	assert_string_equal(error_handling_config->error_image_file, "photos/error_file.tiff");
	assert_string_equal(error_handling_config->error_message, "404: not found");

	free_config();
}

static void test_logging() {
#ifdef DEBUG
	char *good_args[] = {"test", "--log-file", "logs/scaler.log"};
	int good_args_len = 3;

	alloc_default_config();

	apply_commandline_config(good_args_len, good_args);
	assert_string_equal(logging_config->log_file, "logs/scaler.log");

	free_config();
#endif
}

static void test_limits() {
	char *good_args[] = {"test", "--max-out-pixels", "761", "--limit-files", "79", "--limit-disk", "696", "--limit-map", "273", "--limit-memory", "298", "--limit-area", "35"};
	int good_args_len = 13;

	alloc_default_config();

	apply_commandline_config(good_args_len, good_args);
	assert_equal(resource_limit_config->max_pixel_no,  761);
	assert_equal(resource_limit_config->file_limit,  79);
	assert_equal(resource_limit_config->disk_limit,  696);
	assert_equal(resource_limit_config->map_limit,  273);
	assert_equal(resource_limit_config->memory_limit,  298);
	assert_equal(resource_limit_config->area_limit,  35);

	free_config();
}

/* setup and teardown */
static void test_setup() {
// No debugging possible in this test
//	debug_start(logging_config->log_file);
}

static void test_teardown() {
//	debug_stop();
}

int main(int argc, char **argv) {
	TestSuite *commandline_suite = create_named_test_suite(__FILE__);

	add_test(commandline_suite, test_apply_commandline_config);
	add_test(commandline_suite, test_output_geometry);
	add_test(commandline_suite, test_output_format);
	add_test(commandline_suite, test_simple_cgi_query_parameters);
	add_test(commandline_suite, test_simple_cgi_query_values);
	add_test(commandline_suite, test_simple_cgi_query_defaults);
	add_test(commandline_suite, test_storage);
	add_test(commandline_suite, test_resize_filters);
	add_test(commandline_suite, test_transparent_image_handling);
	add_test(commandline_suite, test_general_operation);
	add_test(commandline_suite, test_error_handling);
	add_test(commandline_suite, test_logging);
	add_test(commandline_suite, test_limits);

	setup(commandline_suite, test_setup);
	teardown(commandline_suite, test_teardown);

	return run_test_suite(commandline_suite, create_text_reporter());
}

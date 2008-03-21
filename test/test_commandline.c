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

#include "../cgreen/cgreen.h"
#include "runtime_config.h"
#include "defaults.h"
#include "debug.h"
#include "commandline.h"

/* commandline.c tests */
static void test_apply_commandline_config() {
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
	assert_equal(output_config->quality, DEFAULT_QUALITY);
	assert_equal(operation_config->no_cache, DEFAULT_NO_CACHE);
	assert_equal(operation_config->no_serve, DEFAULT_NO_SERVE);
	assert_equal(operation_config->no_headers, DEFAULT_NO_HEADERS);

	free_operation_config(operation_config);
	free_output_config(output_config);

	output_config = alloc_default_output_config();
	operation_config = alloc_default_operation_config();

	apply_commandline_config(7, args2);

	assert_string_equal(output_config->file_name, "abc/e/f.jpg");
	assert_equal(output_config->size.w, 100);
	assert_equal(output_config->size.h, DEFAULT_HEIGHT);
	assert_equal(output_config->scale_method, SM_STRICT);
	assert_equal(output_config->quality, DEFAULT_QUALITY);
	assert_equal(operation_config->no_cache, 0);
	assert_equal(operation_config->no_serve, 1);
	assert_equal(operation_config->no_headers, 0);

	apply_commandline_config(7, args3);

	assert_string_equal(output_config->file_name, "file.gif");
	assert_equal(output_config->size.w, 100);
	assert_equal(output_config->size.h, 100);
	assert_equal(output_config->scale_method, SM_STRICT);
	assert_equal(output_config->quality, DEFAULT_QUALITY);
	assert_equal(operation_config->no_cache, 0);
	assert_equal(operation_config->no_serve, 1);
	assert_equal(operation_config->no_headers, 0);

	apply_commandline_config(8, all_true);

	assert_string_equal(output_config->file_name, "file.gif");
	assert_equal(output_config->size.w, 100);
	assert_equal(output_config->size.h, 100);
	assert_equal(output_config->scale_method, SM_STRICT);
	assert_equal(output_config->quality, LOWQ_QUALITY);
	assert_equal(operation_config->no_cache, 1);
	assert_equal(operation_config->no_serve, 1);
	assert_equal(operation_config->no_headers, 1);
}

/* setup and teardown */
static void test_setup() {
	alloc_default_config();
	debug_start(logging_config->log_file);
}

static void test_teardown() {
	free_config();
	debug_stop();
}

int main(int argc, char **argv) {
	TestSuite *commandline_suite = create_named_test_suite(__FILE__);

	add_test(commandline_suite, test_apply_commandline_config);

	setup(commandline_suite, test_setup);
	teardown(commandline_suite, test_teardown);

	return run_test_suite(commandline_suite, create_text_reporter());
}

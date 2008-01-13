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
#include "test_config.h"
#include "runtime_config.h"
#include "debug.h"
#include "commandline.h"

/* commandline.c tests */
static void test_apply_commandline_config() {
	struct runtime_config *runtime_config;
	struct operation_config *operation_config;

	/* if I change this to use config defines... */
	char *args1[] = { "test", "-w", "100", "-h", "200" };
	char *args2[] = { "test", "-w", "100", "-s", "true", "-nc", "false", "-ns", "true", "-nh", "dsfa", "abc/e/f.jpg" };
	char *args3[] = { "test", "file.gif", "-h", "300", "-h", "100" };
	char *all_true[] = { "test", "-w", "100", "-s", "true", "-wap", "true", "-nc", "true", "-ns", "true", "-nh", "true" };
	char *all_bogo[] = { "test", "-w", "100", "-s", "1", "-wap", "bogo", "-nc", "xxx", "-ns", "aaa", "-nh", "0" };
	char *all_false[] = { "test", "-w", "100", "-s", "false", "-wap", "false", "-nc", "false", "-ns", "false", "-nh", "false" };	
	char *args4[] = { "test",  "-bogo", "true" };


	runtime_config = alloc_default_runtime_config();
	operation_config = alloc_default_operation_config();

	apply_commandline_config(runtime_config, operation_config, 5, args1);

	assert_equal(runtime_config->file_name, 0);
	assert_equal(runtime_config->size.w, 100);
	assert_equal(runtime_config->size.h, 200);
	assert_equal(runtime_config->strict, DEFAULT_STRICT);
	assert_equal(runtime_config->quality, DEFAULT_QUALITY);
	assert_equal(operation_config->no_cache, DEFAULT_NO_CACHE);
	assert_equal(operation_config->no_serve, DEFAULT_NO_SERVE);
	assert_equal(operation_config->no_headers, DEFAULT_NO_HEADERS);

	free_operation_config(operation_config);
	free_runtime_config(runtime_config);

	runtime_config = alloc_default_runtime_config();
	operation_config = alloc_default_operation_config();

	apply_commandline_config(runtime_config, operation_config, 12, args2);

	assert_string_equal(runtime_config->file_name, "abc/e/f.jpg");
	assert_equal(runtime_config->size.w, 100);
	assert_equal(runtime_config->size.h, DEFAULT_HEIGHT);
	assert_equal(runtime_config->strict, 1);
	assert_equal(runtime_config->quality, DEFAULT_QUALITY);
	assert_equal(operation_config->no_cache, 0);
	assert_equal(operation_config->no_serve, 1);
	assert_equal(operation_config->no_headers, 0);

	apply_commandline_config(runtime_config, operation_config, 6, args3);

	assert_string_equal(runtime_config->file_name, "file.gif");
	assert_equal(runtime_config->size.w, 100);
	assert_equal(runtime_config->size.h, 100);
	assert_equal(runtime_config->strict, 1);
	assert_equal(runtime_config->quality, DEFAULT_QUALITY);
	assert_equal(operation_config->no_cache, 0);
	assert_equal(operation_config->no_serve, 1);
	assert_equal(operation_config->no_headers, 0);

	apply_commandline_config(runtime_config, operation_config, 13, all_true);

	assert_string_equal(runtime_config->file_name, "file.gif");
	assert_equal(runtime_config->size.w, 100);
	assert_equal(runtime_config->size.h, 100);
	assert_equal(runtime_config->strict, 1);
	assert_equal(runtime_config->quality, LOWQ_QUALITY);
	assert_equal(operation_config->no_cache, 1);
	assert_equal(operation_config->no_serve, 1);
	assert_equal(operation_config->no_headers, 1);

	apply_commandline_config(runtime_config, operation_config, 13, all_bogo);

	assert_string_equal(runtime_config->file_name, "file.gif");
	assert_equal(runtime_config->size.w, 100);
	assert_equal(runtime_config->size.h, 100);
	assert_equal(runtime_config->strict, 1);
	assert_equal(runtime_config->quality, LOWQ_QUALITY);
	assert_equal(operation_config->no_cache, 1);
	assert_equal(operation_config->no_serve, 1);
	assert_equal(operation_config->no_headers, 1);


	apply_commandline_config(runtime_config, operation_config, 13, all_false);

	assert_string_equal(runtime_config->file_name, "file.gif");
	assert_equal(runtime_config->size.w, 100);
	assert_equal(runtime_config->size.h, 100);
	assert_equal(runtime_config->strict, 0);
	assert_equal(runtime_config->quality, DEFAULT_QUALITY);
	assert_equal(operation_config->no_cache, 0);
	assert_equal(operation_config->no_serve, 0);
	assert_equal(operation_config->no_headers, 0);

	apply_commandline_config(runtime_config, operation_config, 13, all_bogo);

	assert_string_equal(runtime_config->file_name, "file.gif");
	assert_equal(runtime_config->size.w, 100);
	assert_equal(runtime_config->size.h, 100);
	assert_equal(runtime_config->strict, 0);
	assert_equal(runtime_config->quality, DEFAULT_QUALITY);
	assert_equal(operation_config->no_cache, 0);
	assert_equal(operation_config->no_serve, 0);
	assert_equal(operation_config->no_headers, 0);

	apply_commandline_config(runtime_config, operation_config, 3, args4);

	free_operation_config(operation_config);
	free_runtime_config(runtime_config);
}

/* setup and teardown */
static void test_setup() {
	debug_start(DEBUG_FILE);
}

static void test_teardown() {
	debug_stop();
}

int main(int argc, char **argv) {
	TestSuite *commandline_suite = create_named_test_suite(__FILE__);

	add_test(commandline_suite, test_apply_commandline_config);

	setup(commandline_suite, test_setup);
	teardown(commandline_suite, test_teardown);

	return run_test_suite(commandline_suite, create_text_reporter());
}

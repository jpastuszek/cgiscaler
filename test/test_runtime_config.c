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

/* setup and teardown */
static void test_setup() {
	debug_start(DEBUG_FILE);
}

static void test_teardown() {
	debug_stop();
}

int main(int argc, char **argv) {
	TestSuite *runtime_config_suite = create_test_suite();

	add_test(runtime_config_suite, test_alloc_default_runtime_config);

	setup(runtime_config_suite, test_setup);
	teardown(runtime_config_suite, test_teardown);

	return run_test_suite(runtime_config_suite, create_text_reporter());
}

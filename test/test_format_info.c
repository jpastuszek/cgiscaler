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
#include "debug.h"
#include "format_info.h"


static void test_get_format_info_from_builtin() {
	struct format_info *fi;

	fi = get_format_info_from_builtin("JPG");
	assert_string_equal(fi->format, "JPG");
	assert_string_equal(fi->mime_type, "image/jpeg");
	assert_string_equal(fi->file_ext, "jpg");

	free_format_info(fi);
}

static void test_get_format_info_from_magick() {
	struct format_info *fi;

	fi = get_format_info_from_magick("JPG");
	assert_string_equal(fi->format, "JPG");
	assert_string_equal(fi->mime_type, "image/jpeg");
	assert_string_equal(fi->file_ext, "jpg");

	free_format_info(fi);
}

static void test_format_to_format_info() {
	struct format_info *fi;

	/* this should use builtin */
	fi = format_to_format_info("JPG");
	assert_string_equal(fi->format, "JPG");
	assert_string_equal(fi->mime_type, "image/jpeg");
	assert_string_equal(fi->file_ext, "jpg");

	free_format_info(fi);

	/* this should use magick */
	fi = format_to_format_info("MIFF");
	assert_string_equal(fi->format, "MIFF");
	assert_string_equal(fi->mime_type, "image/x-miff");
	assert_string_equal(fi->file_ext, "miff");

	free_format_info(fi);
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
	TestSuite *file_format_info_suite = create_named_test_suite(__FILE__);

	add_test(file_format_info_suite, test_get_format_info_from_builtin);
	add_test(file_format_info_suite, test_get_format_info_from_magick);
	add_test(file_format_info_suite, test_format_to_format_info);

	setup(file_format_info_suite, test_setup);
	teardown(file_format_info_suite, test_teardown);

	return run_test_suite(file_format_info_suite, create_text_reporter());
}
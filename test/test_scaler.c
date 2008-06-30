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
#include "asserts.h"
#include "test_config.h"
#include "file_utils.h"
#include "runtime_config.h"
#include "debug.h"
#include "scaler.h"

/* cgiscaler.c tests */
static void test_load_image() {
	MagickWand *magick_wand;
	struct dimensions size;

	size.w = 100;
	size.h = 100;

	magick_wand = load_image("non_existing_file.jpg", size);
	assert_equal(magick_wand, 0);

	magick_wand = load_image(IMAGE_TEST_FILE, size);
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
}

static void test_fit_resize() {
	MagickWand *image_ping;
	MagickWand *image;
	struct dimensions a, img, b;
	struct dimensions size;

	size.w = 0;
	size.h = 0;

	a.w = 100;
	a.h = 200;

	image_ping = ping_image(IMAGE_TEST_FILE);
	assert_not_equal(image_ping, 0);

	img = get_image_size(image_ping);

	free_image(image_ping);

	image = fit_resize(IMAGE_TEST_FILE, a);
	assert_not_equal(image, 0);

	/* as we know this works from previous tests we can calculate image size to see if it works */
	b = resize_to_fit_in(img, a);
	img = get_image_size(image);

	assert_equal(img.w, b.w);
	assert_equal(img.h, b.h);

	free_image(image);
}

static void test_strict_resize() {
	MagickWand *image;
	struct dimensions a;
	struct dimensions size;

	size.w = 0;
	size.h = 0;

	a.w = 100;
	a.h = 200;

	image = strict_resize(IMAGE_TEST_FILE, a);
	assert_not_equal(image, 0);

	size = get_image_size(image);
	
	assert_equal(size.w, a.w);
	assert_equal(size.h, a.h);

	free_image(image);
}

static void test_resize_field_limiting() {
	MagickWand *image;
	struct dimensions a;
	struct dimensions img;

	/* we will re-size a little bit so resizing will take effect */
	a.w = IMAGE_TEST_FILE_WIDTH - 10;
	a.h = IMAGE_TEST_FILE_HEIGHT - 10;

	/* resource_limit_config->max_pixel_no is set to lower then a.w * a.h */
	if (a.w * a.h > resource_limit_config->max_pixel_no)
		assert_true(1);
	else
		assert_true(0);

	/* fit_resize */
	image = fit_resize(IMAGE_TEST_FILE, reduce_filed(a, resource_limit_config->max_pixel_no));
	assert_not_equal(image, 0);

	img = get_image_size(image);

	assert_not_equal(img.w, a.w);
	assert_not_equal(img.h, a.h);
	assert_equal_low_precision(img.w * img.h, resource_limit_config->max_pixel_no, 0.1);

	free_image(image);

	/* strict_resize */
	image = strict_resize(IMAGE_TEST_FILE, reduce_filed(a, resource_limit_config->max_pixel_no));
	assert_not_equal(image, 0);

	img = get_image_size(image);

	assert_not_equal(img.w, a.w);
	assert_not_equal(img.h, a.h);
	assert_equal_low_precision(img.w * img.h, resource_limit_config->max_pixel_no, 0.1);

	free_image(image);
}

static void test_remove_transparentcy() {
	MagickWand *image;
	struct dimensions a;

	/* we re-size just 1 pixel so we don't loose our transparent region at (10,10) */
	a.w = IMAGE_TEST_FILE_WIDTH - 1;
	a.h = IMAGE_TEST_FILE_HEIGHT - 1;

	image = load_image(IMAGE_TEST_FILE, a);
	assert_not_equal(image, 0);

	/* assert we have our test image loaded OK */
	assert_image_pixel_color(image, IMAGE_TEST_FILE_WIDTH - 1, IMAGE_TEST_FILE_HEIGHT - 1, "#F000FF");

	/* we assert that we don't have transparent pixel */
	assert_image_pixel_alpha(image, 10, 10, 1.0);
	debug(DEB, "### Transparency color: %s", output_config->transparency_replacement_color);
	/* and color is TRANSPARENCY_COLOUR_MAGICK_STR */
	assert_image_pixel_color(image, 10, 10, output_config->transparency_replacement_color);

	free_image(image);
}

static void test_prepare_blob() {
	MagickWand *image;
	struct dimensions a;
	size_t blob_len;
	unsigned char *blob;

	a.w = 100;
	a.h = 200;

	image = strict_resize(IMAGE_TEST_FILE, a);
	assert_not_equal(image, 0);

	/* we are preparing JPEG data */
	blob = prepare_blob(image, 70, &blob_len, "JPG");
	assert_not_equal(blob, 0);
	assert_not_equal(blob_len, 0);

	/* JPEG data magic number */
	assert_equal(blob[0], 0xff);
	assert_equal(blob[1], 0xd8);

	free_blob(blob);

	/* try to use non existing format */
	blob = prepare_blob(image, 70, &blob_len, "BOGO");
	assert_equal(blob, 0);
	assert_equal(blob_len, 0);

	free_blob(blob);
	free_image(image);
}

static void test_invalid_size() {
	MagickWand *image;
	struct dimensions size;

	size.w = -100;
	size.h = -200;
	
	image = fit_resize(IMAGE_TEST_FILE, size);
	assert_equal(image, 0);

	image = strict_resize(IMAGE_TEST_FILE, size);
	assert_equal(image, 0);
}


/* setup and teardown */
static void test_setup() {
	alloc_default_config();
	debug_start(logging_config->log_file);
	/* now we need ImageMagick after this we should terminate ImgeMagick afterwards */
	MagickWandGenesis();
}

static void test_teardown() {
	free_config();
	MagickWandTerminus();
	debug_stop();
}

int main(int argc, char **argv) {
	TestSuite *cgiscaler_suite = create_named_test_suite(__FILE__);

	add_test(cgiscaler_suite, test_load_image);
	add_test(cgiscaler_suite, test_fit_resize);
	add_test(cgiscaler_suite, test_strict_resize);
	add_test(cgiscaler_suite, test_resize_field_limiting);
	add_test(cgiscaler_suite, test_remove_transparentcy);
	add_test(cgiscaler_suite, test_prepare_blob);
	add_test(cgiscaler_suite, test_invalid_size);

	setup(cgiscaler_suite, test_setup);
	teardown(cgiscaler_suite, test_teardown);

	return run_test_suite(cgiscaler_suite, create_text_reporter());
}

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

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "asserts.h"
#include "../cgreen/cgreen.h"
#include "test_config.h"
#include "debug.h"

/* Returns file size in bytes or -1 */
ssize_t get_file_size(char *file_path) {
	int fd;
	off_t pos;

	assert_file_exists(file_path);

	fd = open(file_path, O_RDONLY);
	pos = lseek(fd, 0, SEEK_END);
	assert_not_equal(pos, -1);

	if (fd != -1)
		close(fd);

	return pos;
}


void assert_dir_exists(char *dir_path) {
	struct stat s;
	assert_not_equal_with_message(stat(dir_path, &s), -1, "directory [%s] does not exist", dir_path);
	assert_true_with_message(S_ISDIR(s.st_mode), "directory [%s] is not a directory", dir_path);
}

void assert_file_exists(char *file_path) {
        struct stat s;
        assert_not_equal_with_message(stat(file_path, &s), -1, "file [%s] does not exist", file_path);
}

void assert_file_not_exists(char *file_path) {
	struct stat s;
        assert_equal_with_message(stat(file_path, &s), -1, "file [%s] exist", file_path);
}

void assert_file_size(char *file_path, off_t size) {
	ssize_t fs;
	fs = get_file_size(file_path);

	assert_equal_with_message(fs, size, "file [%s] size is [%d] while should be [%d]", file_path, fs, size);
}

size_t data_read(int fd) {
	int bread;
	int btotal;
	char buf[256];

	btotal = 0;

	do {
		bread = read(fd, buf, 256);
		if (bread == -1)
			assert_true_with_message(0, "read from fd [%d] failed", fd);
		btotal += bread;

		/* debug(DEB, "Test: Bytes read %d", bread); */
	} while(bread > 0);
	
	return btotal;
}

size_t assert_jpg_data_read(int fd) {
	int bread;
	int btotal;
	unsigned char buf[256];

	btotal = 0;

	do {
		bread = read(fd, buf, 256);
		if (!btotal) {
			/* JPEG data magic number */
			assert_equal(buf[0], 0xff);
			assert_equal(buf[1], 0xd8);
		}

		if (bread == -1)
			assert_true_with_message(0, "read from fd [%d] failed", fd);
		btotal += bread;

		/* debug(DEB, "Test: Bytes read %d", bread); */
	} while(bread > 0);
	
	return btotal;
}


/* This function will read from fd until EOF and check if it read bytes number of bytes */
void assert_byte_read(int fd, ssize_t bytes) {
	ssize_t bytes_total = data_read(fd);
	assert_equal_with_message(bytes_total, bytes, "byte read [%d] from fd [%d] not equal [%d]", bytes_total, fd, bytes);
}

void asser_byte_read_in_range(int fd, ssize_t min, ssize_t max) {
	ssize_t bytes_total;

	bytes_total = data_read(fd);
	if (bytes_total > max || bytes_total < min)
		assert_true_with_message(0,  "byte read [%d] from fd [%d] not in range [%d] and [%d]", bytes_total, fd, min, max);
}

void assert_jpg_byte_read(int fd, ssize_t bytes) {
	ssize_t bytes_total = assert_jpg_data_read(fd);
	assert_equal_with_message(bytes_total, bytes, "byte read [%d] from fd [%d] not equal [%d]", bytes_total, fd, bytes);
}

void assert_jpg_byte_read_in_range(int fd, ssize_t min, ssize_t max) {
	ssize_t bytes_total;

	bytes_total = assert_jpg_data_read(fd);
	if (bytes_total > max || bytes_total < min)
		assert_true_with_message(0,  "byte read [%d] from fd [%d] not in range [%d] and [%d]", bytes_total, fd, min, max);
}

/* This function will look for \n\n in first 1000 bytes */
void assert_headers_read(int fd) {
	int bread;
	int btotal;
	char buf[1];
	int has_end_line;

	btotal = 0;
	has_end_line = 0;

	do {
		bread = read(fd, buf, 1);
		if (bread == -1)
			assert_true_with_message(0, "read from fd [%d] failed", fd);
		btotal += bread;

		if (buf[0] == '\n')
			has_end_line++;
		else
			has_end_line = 0;

		if (has_end_line == 2)
			break;
		/* debug(DEB, "Test: Bytes read %d", bread); */
	} while(bread > 0);

	if (btotal > 1000)
		assert_true_with_message(0, "headers not in first 1000 bytes");
	assert_equal_with_message(has_end_line, 2, "no double end line found while reading from fd [%d]", fd);
}

void assert_equal_low_precision(double value, double expected, double low_precision_error) {
	double ret, range_min;

	range_min = expected - (expected * low_precision_error);
	if (value <= expected && value >= range_min)
		ret = 1;
	else
		ret = 0;

	assert_true_with_message(ret, "value [%F] not in precision range of [%F] and [%F] (%F)", value, range_min, expected, low_precision_error);
}

void assert_equal_precision(double value, double expected, double precision_error) {
	int ret;
	double range_delta;

	range_delta = (expected * precision_error);
	if (value <= expected + (range_delta / 2) && value >= expected - (range_delta / 2))
		ret = 1;
	else
		ret = 0;

	assert_true_with_message(ret, "value [%F] not in precision range of [%F] and [%F] (%F)", value, expected - (range_delta / 2), expected + (range_delta / 2), precision_error);
}

/* TODO: Different quantum and different versions of IM returns different strings */
void assert_image_pixel_color(MagickWand *magick_wand, int x, int y, const char *color) {
	PixelWand *pixel_wand;
	MagickBooleanType status;
	char *color_string;

	pixel_wand = NewPixelWand();
	status = MagickGetImagePixelColor(magick_wand, x, y, pixel_wand);
	assert_not_equal(status, MagickFalse);

	color_string = PixelGetColorAsString(pixel_wand);
	assert_string_equal(color_string, color);
	free(color_string);

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

	assert_true_with_message(true, "pixel alpha should be [%f] but is [%f]", alpha, tested_alpha);

	DestroyPixelWand(pixel_wand);
}

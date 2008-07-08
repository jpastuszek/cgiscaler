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

#include "config.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

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

/** Read all data from file descriptor.
* Read data until EOF from file descriptor and store it in newly allocated memory.
* @param fd file discriptor to read from
* @param len will be set to amount of data read/length of returned buffer
* @return allocated buffer containing all read data or 0 on failure
**/
unsigned char *data_read_all(int fd, unsigned int *len) {
	int bread;
	unsigned int btotal;
	
	unsigned int buf_len = 1024;
	unsigned char buf[1024];

	unsigned char *store = 0;

	btotal = 0;

	while (1) {
		bread = read(fd, buf, buf_len);
		if (bread == -1) {
			assert_true_with_message(0, "read from fd [%d] failed", fd);
			return 0;
		}

		if (bread == 0)
			break;

		//assert_true_with_message(0, "Debug: realloc size %d", btotal + bread);
		store = realloc(store, btotal + bread);
		memcpy(store + btotal, buf, bread);

		btotal += bread;
	}

	*len = btotal;
	return store;
}

/** Tests if data read from file descriptor contains given string.
* @param fd file descriptor to read from
* @param str stirng to be found in read data
* @return true if found, false if not found
**/
unsigned int test_read_contains(int fd, char *str) {
	char *data;
	unsigned int len;
	unsigned int str_len;
	int i;

	data = (char *) data_read_all(fd, &len);
	if (!data)
		return 0;

	data = realloc(data, len + 1);
	data[len] = '\0';

	//assert_true_with_message(0, "data [%d] [%s]", len, data);

	str_len = strlen(str);
	for (i = 0; i < len; i++) {
		if (!strncmp(data + i, str, str_len)) {
			free(data);
			return 1;
		}
	}

	free(data);

	return 0;
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
	short int bread;
	int btotal;
	unsigned char buf[1];
	short int has_end_line;

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
	short int ret;
	double range_delta;

	range_delta = (expected * precision_error);
	if (value <= expected + (range_delta / 2) && value >= expected - (range_delta / 2))
		ret = 1;
	else
		ret = 0;

	assert_true_with_message(ret, "value [%F] not in precision range of [%F] and [%F] (%F)", value, expected - (range_delta / 2), expected + (range_delta / 2), precision_error);
}

char *color_to_hex(unsigned int r,unsigned  int g,unsigned  int b) {
	char *str;
	debug(DEB, "%d,%d,%d", r, g, b);
	
	str = malloc(8);  /* #112233 + \0 */

	snprintf(str, 8, "#%.2X%.2X%.2X", r, g, b);

	return str;
}

//TODO: Two variants of strings returned by IM... no support for IM build with Q16/Q32... may return components > 255
char *im_color_string_to_hex(char *str) {
	int r, g, b;
	char *formats[] = {"rgb%d,%d,%d", "rgb(%d,%d,%d)", 0};
	int i;

	r = g = b = -1;

	for (i = 0; formats[i]; i++) {
		sscanf(str, formats[i],  &r, &g, &b);

		if (r != -1 && g != -1 && b != -1)
			return color_to_hex(r, g, b);
	}
	return 0;
}

void assert_image_pixel_color(MagickWand *magick_wand, int x, int y, const char *hex_color) {
	PixelWand *pixel_wand;
	MagickBooleanType status;
	char *color_string;
	char *hex_color_string;

	pixel_wand = NewPixelWand();

	status = MagickGetImagePixelColor(magick_wand, x, y, pixel_wand);
	assert_not_equal(status, MagickFalse);

	color_string = PixelGetColorAsString(pixel_wand);

	hex_color_string = im_color_string_to_hex(color_string);

	free(color_string);
	DestroyPixelWand(pixel_wand);

	assert_not_equal_with_message(hex_color_string, 0, "Cannot convert IM color string!");

	if (!hex_color_string)
		return;

	assert_string_equal(hex_color_string, hex_color);
	free(hex_color_string);
}

void assert_image_pixel_alpha(MagickWand *magick_wand, int x, int y, float alpha) {
	PixelWand *pixel_wand;
	MagickBooleanType status;
	short int true;
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

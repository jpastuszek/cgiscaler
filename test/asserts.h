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

#include <sys/types.h>
#include <wand/MagickWand.h>

void assert_dir_exists(char *dir_path);
void assert_file_exists(char *file_path);
void assert_file_not_exists(char *file_path);
void assert_file_size(char *file_path, off_t size);

void assert_byte_read(int fd, ssize_t bytes) ;
void asser_byte_read_in_range(int fd, ssize_t min, ssize_t max);

void assert_jpg_byte_read(int fd, ssize_t bytes);
void assert_jpg_byte_read_in_range(int fd, ssize_t min, ssize_t max);

void assert_headers_read(int fd);

void assert_equal_low_precision(double value, double expected, double low_precision_error);
void assert_equal_precision(double value, double expected, double precision_error) ;

void assert_image_pixel_color(MagickWand *magick_wand, int x, int y, const char *hex_color);
void assert_image_pixel_alpha(MagickWand *magick_wand, int x, int y, float alpha) ;

ssize_t get_file_size(char *file_path);

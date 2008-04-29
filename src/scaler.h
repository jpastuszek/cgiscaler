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

#include <wand/MagickWand.h>
#include <stdlib.h>

#include "file_utils.h"
#include "query_string.h"
#include "geometry_math.h"

#ifndef SCALER_H
#define SCALER_H

struct _resize_filters {
	int value;
	char *name;
};

struct _resize_filters resize_filters[11];

#endif

unsigned char *fit_resize_to_blob(media_fpath *media_file_path, struct dimensions resize_to, int quality, size_t *blob_len, const char *format);
unsigned char *strict_resize_to_blob(media_fpath *media_file_path, struct dimensions resize_to, int quality, size_t *blob_len, const char *format) ;

void free_blob(unsigned char *blob);

/* private - exported just for tests */
MagickWand *fit_resize(media_fpath *media_file_path, struct dimensions resize_to);
MagickWand *strict_resize(media_fpath *media_file_path, struct dimensions resize_to);

MagickWand *load_image(media_fpath *media_file_path, struct dimensions to_size);
void free_image(MagickWand *image);
unsigned char *prepare_blob(MagickWand *image, int quality, size_t *blob_len,const char *format);

struct dimensions get_image_size(MagickWand *image);
MagickWand *ping_image(media_fpath *media_file_path);
short int resize(MagickWand *image, struct dimensions to_size);
short int crop(MagickWand *image, struct dimensions to_size, struct point position);

int set_resource_limits(int disk, int map, int file, int memory, int area);

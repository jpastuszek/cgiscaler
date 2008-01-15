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
#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <sys/types.h>
#include "query_string.h"

typedef char fpath;
typedef char abs_fpath;

typedef fpath media_fpath;
typedef fpath cache_fpath;

cache_fpath *create_cache_file_path(fpath *file_path, char *file_extension, int w, int h, int strict, int quality);

abs_fpath *create_absolute_media_file_path(media_fpath *media_file_path);
abs_fpath *create_absolute_cache_file_path(cache_fpath *cache_file_path);

void free_fpath(fpath *file_path);


int create_cache_dir_struct(cache_fpath *cache_file_path);

time_t get_media_file_mtime(media_fpath *media_file_path);
time_t get_cache_file_mtime(cache_fpath *cahce_file_path);

fpath *sanitize_file_path(fpath *file_path);

int write_blob_to_file(unsigned char *blob, int blob_len, abs_fpath *absolute_file_path);

/* private - exported for tests */
time_t get_file_mtime(abs_fpath *absolute_file_path);
fpath *make_file_name_relative(fpath *file_path);
int check_for_double_dot(fpath *file_path);

#endif

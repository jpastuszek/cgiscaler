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

#include "query_string.h"
#include "file_utils.h"

#define NO_ORIG 1
#define NO_CACHE 2
#define MTIME_DIFFER 4
#define CACHE_OK 0

int check_if_cached(media_fpath *media_file_path, media_fpath *cache_file_path);

int write_blob_to_cache(unsigned char *blob, int blob_len, media_fpath *media_file_path, cache_fpath *cache_file_path);
int serve_from_cache_file(media_fpath *media_file_path, cache_fpath *cache_file_path, char *mime_type);

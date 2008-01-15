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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <utime.h>
#include <errno.h>

#include "cache.h"
#include "serve.h"
#include "file_utils.h"
#include "config.h"
#include "debug.h"

void remove_cache_file(cache_fpath *cache_file_path);

/** This function will check if cache file exists and if corresponding original file mtime differs with cached version.
* @param media_file_path path to original media file
* @param cache_file_path path to cache file
* @return Returns bit-mask:
*	NO_ORIG original file does not exist
*	NO_CACHE cache file does not exist
*	MTIME_DIFFER original file mtime differs from cache file mtime or
*	CACHE_OK original file mtime is same as cache file mtime
*/
int check_if_cached(media_fpath *media_file_path, media_fpath *cache_file_path) {
	int orginal_mtime, cache_mtime;

	debug(DEB, "Checking cache");
	
	debug(DEB, "Original media file path: '%s' cache file path: '%s'", media_file_path, cache_file_path);


	orginal_mtime = get_media_file_mtime(media_file_path);
	cache_mtime = get_cache_file_mtime(cache_file_path);

	debug(DEB,"Original media mtime: %d, Cache mtime: %d", orginal_mtime, cache_mtime);

	if (!cache_mtime && !orginal_mtime)
		return NO_ORIG | NO_CACHE;
	
	if (cache_mtime && !orginal_mtime)
		return NO_ORIG;

	if (!cache_mtime && orginal_mtime)
		return NO_CACHE;

	if (cache_mtime != orginal_mtime)
		return MTIME_DIFFER;

	return CACHE_OK;
}

/** Writes data to cache file making sure mtime is updated accordingly.
* @param blob data array
* @param blob_len length of data array in bytes
* @param media_file_path path to original media file - needed to get original mtime
* @param cache_file_path path to cache file to be created
* @return 1 on success 0 on failure
*/
int write_blob_to_cache(unsigned char *blob, int blob_len, media_fpath *media_file_path, cache_fpath *cache_file_path) {
	time_t orginal_mtime;
	struct utimbuf time_buf;
	abs_fpath *absolute_cache_file_path;

	debug(DEB, "Writing cache file for media file '%s'", media_file_path);

	orginal_mtime = get_media_file_mtime(media_file_path);
	if (!orginal_mtime) {
		debug(WARN, "No original file while writing BLOB to cache file!");
		return 0;
	}

	/* writting cache file */
	if (!create_cache_dir_struct(cache_file_path)) {
		debug(ERR, "Cannot create path structure cache file: '%s'", cache_file_path);
		return 0;
	}

	absolute_cache_file_path = create_absolute_cache_file_path(cache_file_path);
	
	if (!write_blob_to_file(blob, blob_len, absolute_cache_file_path)) {
		debug(ERR, "Writing blob to file '%s' failed", absolute_cache_file_path);
		free_fpath(absolute_cache_file_path);
		return 0;
	}
	
	/* setting mod time to */
	time_buf.actime = time_buf.modtime = orginal_mtime;
	utime(absolute_cache_file_path, &time_buf);

	free_fpath(absolute_cache_file_path);

	return 1;
}

/** Serves image from cache file.
* @param media_file_path to check if original file still exist and if if it's mtime differs with cache file
* @param cache_file_path cache file to serve
* @param mime_type mime type of cache file - to include in HTTP headers
* @return 1 on success 0 when no proper cache file or read failure
*/
int serve_from_cache_file(media_fpath *media_file_path, cache_fpath *cache_file_path, char *mime_type) {
	abs_fpath *absolute_cache_file_path;
	int ret;

	debug(DEB,"Trying cache file: '%s'", cache_file_path);
/*
Logick:
_ _
O C M

1 1 0 - exit
0 1 0 - exit
1 0 0 - rm, exit
0 0 1 - rm, exit
0 0 0 - serve
*/

	switch (check_if_cached(media_file_path, cache_file_path)) {
		/* we don't have cache file or both... returning */
		case NO_CACHE:
		case NO_CACHE | NO_ORIG:
			debug(DEB, "No cache file");
			return 0;

		/* we don't have orig or cache file old... remove cache file and return */
		case NO_ORIG:
		case MTIME_DIFFER:
			debug(DEB, "No original or mtime with original differ");
			remove_cache_file(cache_file_path);
			return 0;
	}

	absolute_cache_file_path = create_absolute_cache_file_path(cache_file_path);

	/* serve */
	debug(DEB,"Serving from cache");
	ret = serve_from_file(absolute_cache_file_path, mime_type);

	free_fpath(absolute_cache_file_path);
	return ret;
}

/** Removes file from cache.
* @param cache_file_path path to cache file to remove
*/
void remove_cache_file(cache_fpath *cache_file_path) {
	abs_fpath *absolute_cache_file_path;
	debug(DEB, "Removing old cache file '%s'", cache_file_path);
	absolute_cache_file_path = create_absolute_cache_file_path(cache_file_path);

	if (unlink(absolute_cache_file_path) == -1)
		debug(WARN, "Removing old cache file '%s' failed: %s", absolute_cache_file_path, strerror(errno));

	free_fpath(absolute_cache_file_path);
}

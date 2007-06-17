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

/* this function will check if cache file exists and if corresponding orginal file mtime differs with cached version
Returns bitmask:
	NO_ORIG orginal file does not exist
	NO_CACHE cache file does not exist
	MTIME_DIFFER orginal file mtime differs from cache file mtime
or
	CACHE_OK orginal file mtime is same as chache file mtime
*/
int check_if_cached(struct query_params *params) {
	char *cache_file_path;
	char *orginal_file_path;
	int orginal_mtime, cache_mtime;

	debug(DEB, "Checking cache");

	orginal_file_path = malloc(strlen(MEDIA_PATH) + strlen(params->file_name) + 1);
	strcpy(orginal_file_path, MEDIA_PATH);
	strcat(orginal_file_path, params->file_name);

	cache_file_path = create_cache_file_path(params);
	
	debug(DEB, "Orginal file path: '%s' cache file path: '%s'", orginal_file_path, cache_file_path);


	orginal_mtime = get_file_mtime(orginal_file_path);
	cache_mtime = get_file_mtime(cache_file_path);

	free(orginal_file_path);
	free(cache_file_path);

	debug(DEB,"Orginal mtime: %d, Cache mtime: %d", orginal_mtime, cache_mtime);

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

int write_blob_to_cache(unsigned char *blob, int blob_len, struct query_params *params) {
	char *file_path;
	char *orginal_file_path;
	time_t orginal_mtime;
	struct utimbuf time_buf;

	debug(DEB, "Writing cache file for image  '%s'", params->file_name);

	/* getting orginal file mtime */
	orginal_file_path = malloc(strlen(MEDIA_PATH) + strlen(params->file_name) + 1);
	strcpy(orginal_file_path, MEDIA_PATH);
	strcat(orginal_file_path, params->file_name);

	orginal_mtime = get_file_mtime(orginal_file_path);
	if (!orginal_mtime) {
		free(orginal_file_path);
		return 0;
	}

	free(orginal_file_path);

	/* writting cache file */
	if (!create_cache_dir_struct(params->file_name)) {
		debug(ERR, "Cannot create path structure for ptath: '%s'", params->file_name);
		return 0;
	}

	file_path = create_cache_file_path(params);
	if (!write_blob_to_file(blob, blob_len, file_path)) {
		debug(ERR, "Writing blob to file '%s' failed", file_path);
		free(file_path);
		return 0;
	}
	
	/* setting mod time to */
	time_buf.actime = time_buf.modtime = orginal_mtime;
	utime(file_path, &time_buf);

	free(file_path);
	return 1;
}

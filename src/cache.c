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
#include "config.h"
#include "debug.h"

time_t get_file_mtime(char *path);

/* returns allocated cache file name string */
char *prepare_cache_file_path(struct query_params *params) {
	char *file_name;
	int file_name_len, file_name_buff_len = 44;

	/* we are allocating initial file name buffor */
	file_name = malloc(file_name_buff_len);
	if (!file_name)
		exit(66);

	/* now wi will loop until snprintf will return less than our buffor size */
	while (1) {
		file_name_len = snprintf(file_name, file_name_buff_len, "%s%s-%u-%u-%u-%u", CACHE_PATH, params->file_name, params->size.w, params->size.h, params->strict, params->lowq);
	
		/* it worked, we have less then file_name_buff_len */
		if (file_name_len > -1 && file_name_len < file_name_buff_len)
			break;
		
		/* we have more then file_name_buff_len */
		if (file_name_len > -1)
			file_name_buff_len = file_name_len + 1;
		else
			file_name_buff_len *= 2;
	
		/* resize to add more space */
		if ((file_name = realloc(file_name, file_name_buff_len)) == NULL)
			exit(66);
	}

	debug(DEB, "Cache file name entry: '%s'", file_name);
	return file_name;
}

/* this function will check if cache file exists and if corresponding orginal file mtime differs with cached version
Returns bitmask:
	NO_ORIG orginal file does not exist
	NO_CACHE cache file does not exist
	MTIME_DIFFER orginal file mtime differs from cache file mtime
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

	cache_file_path = prepare_cache_file_path(params);
	
	debug(DEB, "Orginal file path: '%s' cache file path: '%s'", orginal_file_path, cache_file_path);


	orginal_mtime = get_file_mtime(orginal_file_path);
	cache_mtime = get_file_mtime(cache_file_path);

	free(orginal_file_path);
	free(cache_file_path);

	debug(DEB,"Orginal mtime: %d, Cache mtime: %d", orginal_mtime, cache_mtime);

	if (!cache_mtime && !orginal_mtime)
		return NO_ORIG & NO_CACHE;
	
	if (cache_mtime && !orginal_mtime)
		return NO_ORIG;

	if (!cache_mtime && orginal_mtime)
		return NO_CACHE;

	if (cache_mtime != orginal_mtime)
		return MTIME_DIFFER;

	return CACHE_OK;
}

/* returns file mtime or 0 if file does not exists */
time_t get_file_mtime(char *path) {
	struct stat s;
	if (stat(path, &s) == -1)
		return 0;
	return s.st_mtime;
}

int create_dir_struct(char *file_path) {
	char *next_slash;
	char *full_path;
	char *dir_name;
	int dir_name_len;

	/* we are not going to include tailing '/' */
	full_path = malloc(strlen(CACHE_PATH));
	if (!full_path)
		exit(66);

	strncpy(full_path, CACHE_PATH, strlen(CACHE_PATH) - 1);
	full_path[strlen(CACHE_PATH) - 1] = 0;

	while ((next_slash = index(file_path, '/')) != 0) {
		/* if next char in file path is '/' we skip it (in case of "////" like stuff */
		if (next_slash == file_path + 1) {
			file_path++;
			continue;
		}
		
		dir_name_len = next_slash - file_path;
		dir_name = malloc(dir_name_len + 1);
		if (!dir_name)
			exit(66);
	
		strncpy(dir_name, file_path, dir_name_len);
		dir_name[dir_name_len] = 0;
		debug(DEB, "Dir name: '%s'", dir_name);

		full_path = realloc(full_path, strlen(full_path) + 1 + dir_name_len + 1);
		if (!full_path)
			exit(66);

		strcat(full_path, "/");
		strcat(full_path, dir_name);
		file_path += dir_name_len + 1;
		free(dir_name);

		debug(DEB, "Creating directory: '%s'", full_path);
		if (mkdir(full_path, 0777) == -1) {
			if (errno == EEXIST)
				continue;
			
			debug(ERR, "Failed to create directory: '%s': %s", full_path, strerror(errno));
			free(full_path);
			return 0;
		}
	}

	free(full_path);
	return 1;
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
	if (!create_dir_struct(params->file_name)) {
		debug(ERR, "Cannot create path structure for ptath: '%s'", params->file_name);
		return 0;
	}

	file_path = prepare_cache_file_path(params);
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

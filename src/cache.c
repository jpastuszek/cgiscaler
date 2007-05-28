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

#include "cache.h"
#include "config.h"
#include "debug.h"

time_t get_file_mtime(char *path);

char *prepare_cache_file_name(struct query_params *params) {
	char *file_name;
	int file_name_len, file_name_buff_len = 24;

	/* we are allocating initial file name buffor */
	file_name = malloc(file_name_buff_len);
	if (!file_name)
		exit(66);

	/* now wi will loop until snprintf will return less than our buffor size */
	while (1) {
		file_name_len = snprintf(file_name, file_name_buff_len, "%s-%u-%u-%u-%u", params->file_name, params->size.w, params->size.h, params->strict, params->lowq);
	
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

	debug(DEB,"Cache file name entry: '%s'", file_name);
	return file_name;
}

/* this function will check if cache file exists and if corresponding orginal file mtime differs with cached version
Returns:
	-2 both orginal and cache file does not exist
	-1 orginal file does not exist but cashe file is on place
	0 cache file does not exist
	1 orginal file mtime differs from cache file mtime
	2 orginal file mtime is same as chache file mtime
*/
int check_if_cached(struct query_params *params) {
	char *file_name;
	char *cache_file_path;
	char *orginal_file_path;
	int orginal_mtime, cache_mtime;
	int ret;

	debug(DEB,"Checking cache");

	orginal_file_path = malloc(strlen(MEDIA_PATH) + strlen(params->file_name));
	strcpy(orginal_file_path, MEDIA_PATH);
	strcat(orginal_file_path, params->file_name);

	file_name = prepare_cache_file_name(params);
	cache_file_path = malloc(strlen(CACHE_PATH) + strlen(file_name));
	strcpy(cache_file_path, CACHE_PATH);
	strcat(cache_file_path, file_name);
	free(file_name);
	
	debug(DEB,"Orginal file path: '%s' cache file path: '%s'", orginal_file_path, cache_file_path);


	orginal_mtime = get_file_mtime(orginal_file_path);
	cache_mtime = get_file_mtime(cache_file_path);

	free(orginal_file_path);
	free(cache_file_path);

	if (!cache_mtime && !orginal_mtime)
		return -2;
	
	if (cache_mtime && !orginal_mtime)
		return -1;

	if (!cache_mtime && orginal_mtime)
		return 0;

	if (cache_mtime != orginal_mtime)
		return 1;

	return 2;
}

/* returns file mtime or 0 if file does not exists */
time_t get_file_mtime(char *path) {
	struct stat s;
	if (stat(path, &s) == -1)
		return 0;
	return s.st_mtime;
}



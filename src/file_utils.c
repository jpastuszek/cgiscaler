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
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "file_utils.h"
#include "config.h"
#include "debug.h"

/* Returns allocated media file path */
char *create_media_file_path(char *file_name) {
	char *path;
	path = malloc(strlen(MEDIA_PATH) + strlen(file_name) + 1);
	strcpy(path, MEDIA_PATH);
	strcat(path, file_name);
	
	return path;
}

/* Returns allocated cache file name string */
char *create_cache_file_path(struct query_params *params) {
	char *file_name;
	int file_name_len, file_name_buff_len = 44;

	/* we are allocating initial file name buffer */
	file_name = malloc(file_name_buff_len);
	if (!file_name)
		exit(66);

	/* now we will loop until snprintf will return less than our buffor size */
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

/* returns file mtime or 0 if file does not exists */
time_t get_file_mtime(char *path) {
	struct stat s;
	if (stat(path, &s) == -1)
		return 0;
	return s.st_mtime;
}

/* Returns relative file path (no beggining /) */
char *make_file_name_relative(char *file_path) {
	char *return_name;
	
	// removing front '/'
	while(file_path[0] == '/')
		file_path++;

	return_name = malloc(strlen(file_path) + 1);
	if (!return_name)
		exit(66);

	strcpy(return_name, file_path);
	return return_name;
}

/* Returns 1 if there are '..' sequences in the file_path */
int check_for_double_dot(char *file_path) {
	int dot_offset;
	char *dot;

	dot_offset = 0;
	while ((dot = index(file_path + dot_offset,'.')) != 0) {
		if (*(dot+1) == '.') {
			return 1;
		}
		dot_offset = dot - file_path + 1;
	}
	
	return 0;
}


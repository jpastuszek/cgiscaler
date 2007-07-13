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
#include <fcntl.h>
#include <errno.h>

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

	/* now we will loop until snprintf will return less than our buffer size */
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
	
		/* re-size to add more space */
		if ((file_name = realloc(file_name, file_name_buff_len)) == NULL)
			exit(66);
	}

	debug(DEB, "Cache file name entry: '%s'", file_name);
	return file_name;
}

int create_cache_dir_struct(char *file_path) {
	char *next_slash;
	char *full_path;
	char *dir_name;
	int dir_name_len;

	/* if we have ".." in path... failing */
	if (check_for_double_dot(file_path))
		return 0;

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

/* returns file mtime or 0 if file does not exists */
time_t get_file_mtime(char *path) {
	struct stat s;
	if (stat(path, &s) == -1)
		return 0;
	return s.st_mtime;
}

/* Returns relative file path (no beginning /) */
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


int write_blob_to_file(unsigned char *blob, int blob_len, char *file_path) {
	int out_file;
	size_t bytes_written;
	size_t total_blob_written;

	debug(DEB, "Writing BLOB to file: '%s'", file_path);

	out_file = open(file_path, O_WRONLY | O_CREAT | O_EXCL, 0666);
	if (out_file == -1) {
		debug(ERR, "Error while opening BLOB write file '%s': %s", file_path, strerror(errno));
		return 0;
	}

	total_blob_written = 0;
	while(1) {
		debug(DEB, "Writing %d bytes to '%s'", blob_len - total_blob_written, file_path);

		bytes_written = write(out_file, blob + total_blob_written, blob_len - total_blob_written);
		debug(DEB, "%d bytes written", bytes_written);
		if (bytes_written == -1) {
			debug(ERR, "Error writing to '%s': %s", file_path, strerror(errno));
			close(out_file);
			/* as we have failed we will remove the file so it won't be considered as valid thumbnail */
			debug(ERR, "Removing erroneous file '%s': %s", file_path, strerror(errno));
			unlink(file_path);
			return 0;
		}
		
		total_blob_written += bytes_written;

		if (total_blob_written >= blob_len)
			break;
	}

	close(out_file);
	return 1;
}

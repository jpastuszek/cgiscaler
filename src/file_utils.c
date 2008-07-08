/***************************************************************************
 *   Copyright (C) 2007, 2008, 2008 by Jakub Pastuszek   *
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

#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include "file_utils.h"
#include "runtime_config.h"
#include "debug.h"

extern struct storage_config *storage_config;

/** Returns allocated  relative cache file path string.
* For example "test/a/x.jpg-100-200-1-0.jpg"
* @param file_path path to file
* @param file_extension extension of cache file
* @param w width of thumbnail
* @param h height of thumbnail
* @param strict strict parameter
* @param quality quality value
* @return allocated relative cache file path
*/
cache_fpath *create_cache_file_path(fpath *file_path, char *file_extension, int w, int h, int strict, int quality) {
	char *cache_file_name;
	int cache_file_name_len, cache_file_name_buff_len = 40;

	/* we are allocating initial file name buffer */
	cache_file_name = malloc(cache_file_name_buff_len);
	if (!cache_file_name)
		exit(66);

	/* now we will loop until snprintf will return less than our buffer size */
	while (1) {
		cache_file_name_len = snprintf(cache_file_name, cache_file_name_buff_len, "%s-%d-%d-%d-%d.%s", file_path, w, h, strict, quality, file_extension);
	
		/* it worked, we have less then file_name_buff_len */
		if (cache_file_name_len > -1 && cache_file_name_len < cache_file_name_buff_len)
			break;
		
		/* if we have a valure -> alloc precise else double buffer len */
		if (cache_file_name_len > -1)
			cache_file_name_buff_len = cache_file_name_len + 1;
		else
			cache_file_name_buff_len *= 2;

		/* re-size to add more space */
		cache_file_name = realloc(cache_file_name, cache_file_name_buff_len);
		if (!cache_file_name)
			exit(66);
	}

	debug(DEB, "Cache file name entry: '%s'", cache_file_name);
	return cache_file_name;
}

/** Returns allocated media file path.
* @param media_file_path relative path to media file
* @return allocated absolute media file path
* @see free_fpath()
* @see create_absolute_cache_file_path()
*/
abs_fpath *create_absolute_media_file_path(media_fpath *media_file_path) {
	char *path;
	path = malloc(strlen(storage_config->media_directory) + strlen(media_file_path) + 2);
	strcpy(path, storage_config->media_directory);
	strcat(path, DIRECTORY_DELIMITER);
	strcat(path, media_file_path);
	
	return path;
}

/** Returns allocated cache file path.
* @param cache_file_path relative path to cache file
* @return allocated absolute cache file path
* @see free_fpath()
* @see create_absolute_media_file_path()
*/
abs_fpath *create_absolute_cache_file_path(cache_fpath *cache_file_path) {
	char *path;
	path = malloc(strlen(storage_config->cache_directory) + strlen(cache_file_path) + 2);
	strcpy(path, storage_config->cache_directory);
	strcat(path, DIRECTORY_DELIMITER);
	strcat(path, cache_file_path);
	
	return path;
}

/** Frees allocated memory for file path */
void free_fpath(fpath *file_path) {
	free(file_path);
}

/** Creates directory structure inside cache directory for cache file storage.
* @param cache_file_path relative path to cache file - directory part from this path will be used
* @return 1 for success 0 on filure
*/
int create_cache_dir_struct(cache_fpath *cache_file_path) {
	char *next_slash;
	char *full_path;
	char *dir_name;
	int dir_name_len;

	/* if we have ".." in path... failing */
	if (check_for_double_dot((fpath *) cache_file_path))
		return 0;

	/* we are not going to include tailing '/' */
	full_path = malloc(strlen(storage_config->cache_directory) + 1);
	if (!full_path)
		exit(66);

	strncpy(full_path, storage_config->cache_directory, strlen(storage_config->cache_directory));
	full_path[strlen(storage_config->cache_directory)] = 0;

	while ((next_slash = index(cache_file_path, '/')) != 0) {
		
		dir_name_len = next_slash - cache_file_path;
		/* if next char in file path is '/' we skip it (in case of "////" like stuff */
		if (!dir_name_len) {
			cache_file_path++;
			continue;
		}

		dir_name = malloc(dir_name_len + 1);
		if (!dir_name)
			exit(66);
	
		strncpy(dir_name, cache_file_path, dir_name_len);
		dir_name[dir_name_len] = 0;
		debug(DEB, "Dir name: '%s'", dir_name);

		full_path = realloc(full_path, strlen(full_path) + 1 + dir_name_len + 1);
		if (!full_path)
			exit(66);

		strcat(full_path, "/");
		strcat(full_path, dir_name);
		cache_file_path += dir_name_len + 1;
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

/** Returns file mtime. 
* @param absolute_file_path path to file
* @return mtime or 0 on failure
*/
time_t get_file_mtime(abs_fpath *absolute_file_path) {
	struct stat s;
	debug(DEB, "Checking mtime: %s", absolute_file_path);
	if (stat((char *) absolute_file_path, &s) == -1)
		return 0;
	return s.st_mtime;
}

/** Returns media file mtime.
* @param media_file_path relative path to media file
* @return mtime or 0 on failure
*/
time_t get_media_file_mtime(media_fpath *media_file_path) {
	abs_fpath *absolute_file_path;
	time_t time;

	absolute_file_path = create_absolute_media_file_path(media_file_path);
	time = get_file_mtime(absolute_file_path);
	free_fpath(absolute_file_path);

	return time;
}

/** Returns cache file mtime.
* @param cahce_file_path relative path to cache file
* @return mtime or 0 on failure
*/
time_t get_cache_file_mtime(cache_fpath *cahce_file_path) {
	abs_fpath *absolute_file_path;
	time_t time;

	absolute_file_path = create_absolute_cache_file_path(cahce_file_path);
	time = get_file_mtime(absolute_file_path);
	free_fpath(absolute_file_path);

	return time;
}

/** This function will sanitize file path.
* It will check if given path is not empty, if it does not contain ".." like parts and it will remove all leading '/' characters form given path.
* @param file_path path to check
* @return allocated sanitized file path or 0 if file path did not pass the test
*/
fpath *sanitize_file_path(fpath *file_path) {
	fpath *rel_file_path;
	rel_file_path = make_file_name_relative(file_path);

	/* bad file name */
	if (!rel_file_path)
		return 0;
	/* empty file_name... failing */
	if (rel_file_path[0] == 0) {
		free_fpath(rel_file_path);
		return 0;
	}

	if (check_for_double_dot(rel_file_path)) {
		debug(WARN, "Double dot found in file path! failing...");
		free_fpath(rel_file_path);
		return 0;
	}

	return rel_file_path;
}

/** Writes data array to file.
* @param blob data array
* @param blob_len length of data array in bytes
* @param absolute_file_path path to file to be written
* @return 1 on success 0 on failure
*/
int write_blob_to_file(unsigned char *blob, int blob_len,abs_fpath *absolute_file_path) {
	int out_file;
	ssize_t bytes_written;
	ssize_t total_blob_written;

	debug(DEB, "Writing BLOB to file: '%s'", absolute_file_path);

	out_file = open(absolute_file_path, O_WRONLY | O_CREAT | O_EXCL, 0666);
	if (out_file == -1) {
		debug(ERR, "Error while opening BLOB write file '%s': %s", absolute_file_path, strerror(errno));
		return 0;
	}

	total_blob_written = 0;
	while(1) {
		debug(DEB, "Writing %d bytes to '%s'", blob_len - total_blob_written, absolute_file_path);

		bytes_written = write(out_file, blob + total_blob_written, blob_len - total_blob_written);
		debug(DEB, "%u bytes written", bytes_written);
		if (bytes_written == -1) {
			debug(ERR, "Error writing to '%s': %s", absolute_file_path, strerror(errno));
			close(out_file);
			/* as we have failed we will remove the file so it won't be considered as valid thumbnail */
			debug(ERR, "Removing erroneous file '%s': %s", absolute_file_path, strerror(errno));
			unlink(absolute_file_path);
			return 0;
		}
		
		total_blob_written += bytes_written;

		if (total_blob_written >= blob_len)
			break;
	}

	close(out_file);
	return 1;
}

/** Returns relative file path.
* This function will remove all leading '/' character.
* @param file_path file path to use
* @return allocated modified file path
*/
fpath *make_file_name_relative(fpath *file_path) {
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

/** Checks for double dot sequences in given file path.
* @param file_path file path to check
* @return 1 if file path contain '..' sequences 0 otherwise
*/
int check_for_double_dot(fpath *file_path) {
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

/** Extracts extension from file path.
* @param file_path path to file or file name
* @return allocated string containing extracted extension or 0
*/
char *extract_file_extension(char *file_path) {
	char *ext;
	int len;

	len = strlen(file_path);
	if (!len)
		return 0;

	// point to last char
	ext = file_path + len - 1;

	// we look for a dot
	while (*ext != '.') {
		if (ext == file_path)
			return 0;
		ext--;
	}

	return strdup(++ext);
}

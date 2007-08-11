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

#include "cgiscaler.h"
#include "query_string.h"
#include "commandline.h"
#include "serve.h"
#include "cache.h"
#include "file_utils.h"
#include "config.h"
#include "debug.h"

/* this is main function, for simplicity we don't want exits at any other point (expect [m|rw]alloc related) */
int main(int argc, char *argv[])
{
	struct runtime_config *config;
	cache_fpath *cache_file_path;
	unsigned char *blob;
	size_t blob_len;

	debug_start(DEBUG_FILE);
	/* stop debugging on exit */
	atexit(debug_stop);
	
	config = alloc_default_runtime_config();

	apply_commandline_config(config, argc, argv);
	apply_query_string_config(config, getenv("PATH_INFO"), getenv("QUERY_STRING"));

	if (!config->file_name) {
		if (!config->no_serve)
			serve_error(config->no_headers);
		exit(70);
	}

	if (!config->no_cache)
		cache_file_path = create_cache_file_path(config->file_name, OUT_FORMAT_EXTENSION, config->size.w, config->size.h, config->strict, config->quality);

	if (!config->no_cache) {
		 if (!config->no_serve) {
			/* if we have served from cache OK... cleanup and exit success */
			if (serve_from_cache_file(media_file_path, cache_file_path, OUT_FORMAT_MIME_TYPE, config->no_headers)) {
				if (!config->no_cache)
					free(cache_file_path);
				free(media_file_path);
				free_runtime_config(config);
				exit(0);
			}
		} else {
			/* check cache only */
			if (check_if_cached(media_file_path, cache_file_path) == CACHE_OK) {
				if (!config->no_cache)
					free(cache_file_path);
				free(media_file_path);
				free_runtime_config(config);
				exit(0);
			}
		}
	}

	/* now we need ImageMagick after this we should terminate ImgeMagick afterwards */
	MagickWandGenesis();

	/* we are reducing requested thumbnail resolution to MAX_PIXEL_NO */
	config->size = reduce_filed(config->size, MAX_PIXEL_NO);

	if (config->strict)
		blob = strict_resize_to_blob(media_file_path, config->size, config->quality, &blob_len, OUT_FORMAT);
	else
		blob = fit_resize_to_blob(media_file_path, config->size, config->quality, &blob_len, OUT_FORMAT);

	if (!blob) {
		if (!config->no_serve)
			serve_error(config->no_headers);
		if (!config->no_cache)
			free(cache_file_path);
		free(media_file_path);
		free_runtime_config(config);
		MagickWandTerminus();
		exit(80);
	}

	/* image processing is done */
	if (!config->no_serve)
		serve_from_blob(blob, blob_len, OUT_FORMAT_MIME_TYPE, config->no_headers);

	/* as we are served it is time for cache file write and clean up */
	MagickWandTerminus();

	if (!config->no_cache)
		write_blob_to_cache(blob, blob_len, media_file_path, cache_file_path);

	free_blob(blob);
	free(media_file_path);
	if (!config->no_cache)
		free(cache_file_path);
	free_runtime_config(config);

	return EXIT_SUCCESS;
}

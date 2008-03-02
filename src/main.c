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

#include "main.h"
#include "runtime_config.h"

extern struct output_config *output_config;
extern struct operation_config *operation_config;
extern struct logging_config *logging_config;

#ifndef TEST_CONFIG_H

/* this is main function, for simplicity we don't want exits at any other point (expect [m|rw]alloc related) */
int main(int argc, char *argv[])
{
	return _main(argc, argv);
}

#endif

int _main(int argc, char *argv[]) {
	cache_fpath *cache_file_path;
	unsigned char *blob;
	size_t blob_len;
	struct timer run_timing;
	struct timer serve_timing;

	/* stop debugging on exit */
	atexit(debug_stop);

	timer_start(&run_timing);
	timer_start(&serve_timing);
	
	/* allocate configuration with default values */
	alloc_default_config();

	debug_start(logging_config->log_file);

	apply_commandline_config(argc, argv);
	apply_simple_query_string_config(getenv("PATH_INFO"), getenv("QUERY_STRING"));

	if (!output_config->file_name) {
		debug(ERR, "No file name given");

		serve_error();

		free_config();
		debug(PROF, "Finished with error after %.3f s",  timer_stop(&run_timing));
		exit(70);
	}

	if (!operation_config->no_cache)
		cache_file_path = create_cache_file_path(output_config->file_name, OUT_FORMAT_EXTENSION, output_config->size.w, output_config->size.h, output_config->scale_method, output_config->quality);

	if (!operation_config->no_cache) {
		 if (!operation_config->no_serve) {
			/* if we have served from cache OK... cleanup and exit success */
			if (serve_from_cache_file(output_config->file_name, cache_file_path, OUT_FORMAT_MIME_TYPE)) {
				if (!operation_config->no_cache)
					free_fpath(cache_file_path);

				free_config();
				debug(PROF, "Served from cache after %.3f s",  timer_stop(&run_timing));
				exit(0);
			}
		} else {
			/* check cache only */
			if (check_if_cached(output_config->file_name, cache_file_path) == CACHE_OK) {
				if (!operation_config->no_cache)
					free_fpath(cache_file_path);

				free_config();
				debug(PROF, "Finished from cache after %.3f s",  timer_stop(&run_timing));
				exit(0);
			}
		}
	}

	/* now we need ImageMagick after this we should terminate ImgeMagick */
	MagickWandGenesis();

	/* we set resource limits so we won't abuse available system resources */
 	set_resource_limits(RESOURCE_LIMIT_DISK, RESOURCE_LIMIT_MAP, RESOURCE_LIMIT_FILE, RESOURCE_LIMIT_MEMORY, RESOURCE_LIMIT_AREA);

	// TODO: Implement SM_FREE support
	if (output_config->scale_method == SM_STRICT)
		blob = strict_resize_to_blob(output_config->file_name, output_config->size, output_config->quality, &blob_len, output_config->format);
	else
		blob = fit_resize_to_blob(output_config->file_name, output_config->size, output_config->quality, &blob_len, output_config->format);

	if (!blob) {
		if (!operation_config->no_serve)
			serve_error();
		if (!operation_config->no_cache)
			free_fpath(cache_file_path);

		free_config();
		MagickWandTerminus();
		exit(80);
	}

	/* image processing is done */
	if (!operation_config->no_serve) {
		// TODO: Autogenerate mime type from output format
		serve_from_blob(blob, blob_len, OUT_FORMAT_MIME_TYPE);
		debug(PROF, "Served after %.3f s",  timer_stop(&serve_timing));
	}

	/* as we are served it is time for cache file write and clean up */
	MagickWandTerminus();

	if (!operation_config->no_cache)
		write_blob_to_cache(blob, blob_len, output_config->file_name, cache_file_path);

	free_blob(blob);
	if (!operation_config->no_cache)
		free_fpath(cache_file_path);

	free_config();

	debug(PROF, "Total run time %.3f s",  timer_stop(&run_timing));

	return EXIT_SUCCESS;
}


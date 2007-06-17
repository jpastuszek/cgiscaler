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
#include "serve.h"
#include "cache.h"
#include "config.h"
#include "debug.h"

/* this is main function, for simplicity we don't want exits at any other point (expect [m|rw]alloc related) */
int main(int argc, char *argv[])
{
	struct query_params *params;
	MagickWand *magick_wand;
	unsigned char *blob;
	size_t blob_len;
	int quality;

	debug_start(DEBUG_FILE);
	/* stopp debuging on exit */
	atexit(debug_stop);
	
	params = get_query_params(getenv("PATH_INFO"), getenv("QUERY_STRING"));
	if (!params) {
		serve_error();
		exit(70);
	}
	
	/* if we have served from cache ok... clenup and exit success */
	if (serve_from_cache(params, OUT_FORMAT_MIME_TYPE)) {
		free_query_params(params);
		exit(0);
	}

	/* now we need ImageMagick after this we should terminate ImgeMagick afterwards */
	MagickWandGenesis();

	/* loading image... if it fails wand will be 0 */
	magick_wand = load_image(params->file_name);
	if (!magick_wand) {
		free_query_params(params);
		serve_error();
		MagickWandTerminus();
		exit(71);
	}

	/* according to strict value we are resizing or cropresizing... if failes wand == 0 */
	if (params->strict)
		magick_wand = strict_resize(magick_wand, params->size);
	else
		magick_wand = fit_resize(magick_wand, params->size);
	if (!magick_wand) {
		free_query_params(params);
		serve_error();
		MagickWandTerminus();
		exit(72);
	}

	if (params->lowq)
		quality = LOWQ_QUALITY;
	else
		quality = NORMAL_QUALITY;

	blob = prepare_blob(magick_wand, quality, &blob_len, OUT_FORMAT);
	if (!blob) {
		free_query_params(params);
		serve_error();
		MagickWandTerminus();
		exit(73);
	}

	serve_from_blob(blob, blob_len, OUT_FORMAT_MIME_TYPE);

	write_blob_to_cache(blob, blob_len, params);

	free_blob(blob);
	free_image(magick_wand);
	free_query_params(params);
	MagickWandTerminus();

	return EXIT_SUCCESS;
}

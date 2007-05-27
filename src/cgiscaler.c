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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <wand/MagickWand.h>

#include "debug.h"
#include "query_string.h"
#include "config.h"
#include "geometry_math.h"

#define ThrowWandException(wand) \
{ \
char \
*description; \
\
ExceptionType \
severity; \
\
description=MagickGetException(wand,&severity); \
(void) debug(ERR,"%s %s %ld %s\n",GetMagickModule(),description); \
description=(char *) MagickRelinquishMemory(description); \
exit(-1); \
}

void do_on_exit(void);
void serve_error_image_and_exit();
void serve_image(MagickWand *magick_wand, struct query_params *params);
MagickWand *load_image(char *file_name);
MagickWand *crop_and_resize(MagickWand *magick_wand, struct dimmensions size);

int main(int argc, char *argv[])
{
	struct query_params *params;
	MagickWand *magick_wand;

	debug_start("/tmp/cgiscaler.deb");
	atexit(do_on_exit);
	
	MagickWandGenesis();

	params = get_query_params();
	if (!params)
		serve_error_image_and_exit();
	
	/* loading image... if it fails wand will be 0 */
	magick_wand = load_image(params->file_name);
	if (!magick_wand)
		serve_error_image_and_exit();

	/* according to strict value we are resizing or cropresizing... if failes wand == 0 */
	if (params->strict)
		magick_wand = crop_and_resize(magick_wand, params->size);


	if (!magick_wand)
		serve_error_image_and_exit();

	serve_image(magick_wand, params);
	DestroyMagickWand(magick_wand);

	free_query_params(params);
	return EXIT_SUCCESS;
}

/* we will stop debug when normal exit */
void do_on_exit(void) {
	MagickWandTerminus();
	debug_stop();
}

void serve_error_image_and_exit() {
	printf("Content-type: text/html\n");
	printf("\n");

	printf("Error: Error image not implemented\n");
	exit(7);
}

void serve_image(MagickWand *magick_wand, struct query_params *params) {
	unsigned char *blob;
	size_t blob_len;
	size_t bytes_written;
	size_t total_blob_written;
	MagickBooleanType status;

	debug(DEB,"Serving Image");

	/* this will remove meta data - this is very important as photos have loads of it */
	status = MagickStripImage(magick_wand);
	if (status == MagickFalse) {
		DestroyMagickWand(magick_wand);
		exit(1);
	}

	status = MagickSetFormat(magick_wand, OUT_FORMAT);
	if (status == MagickFalse) {
		DestroyMagickWand(magick_wand);
		exit(1);
	}

	if (params->lowq)
		status = MagickSetCompressionQuality(magick_wand, LOWQ_QUALITY);
	else
		status = MagickSetCompressionQuality(magick_wand, NORMAL_QUALITY);
	if (status == MagickFalse) {
		DestroyMagickWand(magick_wand);
		exit(1);
	}

	blob = MagickGetImageBlob(magick_wand, &blob_len);
	if (!blob) {
		DestroyMagickWand(magick_wand);
		exit(1);
	}

	printf("Content-Type: %s\n", OUT_FORMAT_MIME_TYPE);
	printf("Content-Length: %u\n", blob_len);

	printf("\n");
	/* flushing buffers befor we do direct fd write */
	fflush(stdout);

	/* using stdout (FILE *) write instead of fd 1 is safer as printf also is using stdout */
//	fwrite(blob, blob_len, 1, stdout);

/* using write is risky as we are writing to fd directly... where using printf we are writting to stdout (FILE *) buffers */
	total_blob_written = 0;
	while(1) {
		debug(DEB, "Writing %d bytes to stdout", blob_len - total_blob_written);
		bytes_written = write(1, blob + total_blob_written, blob_len - total_blob_written);
		debug(DEB, "%d bytes written", bytes_written);
		if (bytes_written == -1) {
			debug(ERR, "Error writting to stdout %s", strerror(errno));
			exit(1);
		}
		
		total_blob_written += bytes_written;

		if (total_blob_written >= blob_len)
			break;
		fsync(1);
	}

	MagickRelinquishMemory(blob);
}


MagickWand *load_image(char *file_name) {
	char *path;
	Image *image;
	MagickWand *magick_wand;
	MagickBooleanType status;

	path = malloc(strlen(MEDIA_PATH) + strlen(file_name) + 1);
	strcpy(path, MEDIA_PATH);
	strcat(path, file_name);

	debug(DEB,"Loading image: '%s'", path);

	magick_wand = NewMagickWand();
	if (!magick_wand)
		return 0;

	status = MagickReadImage(magick_wand, path);
	if (status == MagickFalse) {
		DestroyMagickWand(magick_wand);
		return 0;
	}

	return magick_wand;
}


MagickWand *crop_and_resize(MagickWand *magick_wand, struct dimmensions to_size) {
	struct dimmensions image_size, crop_size;
	int x, y;
	MagickBooleanType status;

	image_size.w = MagickGetImageWidth(magick_wand);
	image_size.h = MagickGetImageHeight(magick_wand);	

	debug(DEB, "Doing CropResize: orginal: %d x %d to: %d x %d", image_size.w, image_size.h, to_size.w, to_size.h);

	/* fit to_szie into image_size */
	crop_size = resize_to_fit_in(to_size, image_size);
	debug(DEB, "Crop size: %d x %d", crop_size.w, crop_size.h);

	/* calculate to center crop */
	x = (image_size.w - crop_size.w) / 2;
	y = (image_size.h - crop_size.h) / 2;
	debug(DEB, "Crop center: %d x %d", x, y);

	status = MagickCropImage(magick_wand, crop_size.w, crop_size.h, x, y);
	if (status == MagickFalse) {
		DestroyMagickWand(magick_wand);
		return 0;
	}

	/* now it is time to resize to to_szie */
	status = MagickResizeImage(magick_wand, to_size.w, to_size.h, LanczosFilter, 0);
	if (status == MagickFalse) {
		DestroyMagickWand(magick_wand);
		return 0;
	}

	return magick_wand;
}

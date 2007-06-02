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
#include <string.h>

#include "cgiscaler.h"
#include "config.h"
#include "geometry_math.h"
#include "cache.h"
#include "serve.h"
#include "debug.h"


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

/* TODO: Zero sized image will do orginal size? may by configuralble? :D */
/* TODO: Performace tests... profiler? :D */

MagickWand *load_image(char *file_name) {
	char *path;
	MagickWand *magick_wand;
	MagickBooleanType status;
	PixelWand *pixel_wand;

	path = malloc(strlen(MEDIA_PATH) + strlen(file_name) + 1);
	strcpy(path, MEDIA_PATH);
	strcat(path, file_name);

	debug(DEB,"Loading image: '%s'", path);

	magick_wand = NewMagickWand();
	if (!magick_wand)
		return 0;

	status = MagickReadImage(magick_wand, path);
	if (status == MagickFalse) {
		debug(WARN,"Loading image '%s' failed", path);
		DestroyMagickWand(magick_wand);
		return 0;
	}

	debug(DEB, "Setting background color to '%s'", DEFAULT_BACKGROUND_COLOR);

	/* Set background to DEFAULT_BACKGROUND_COLOR - in case of transparent gifs or png */
	pixel_wand = NewPixelWand();
	PixelSetColor(pixel_wand, DEFAULT_BACKGROUND_COLOR);
	if (status == MagickFalse) {
		debug(ERR,"Failed to set Pixel Wand Color to '%s'", DEFAULT_BACKGROUND_COLOR);
		DestroyPixelWand(pixel_wand);
		DestroyMagickWand(magick_wand);
		return 0;
	}
	MagickSetBackgroundColor(magick_wand, pixel_wand);
	if (status == MagickFalse) {
		debug(ERR,"Failed to set background color to '%s'", DEFAULT_BACKGROUND_COLOR);
		DestroyPixelWand(pixel_wand);
		DestroyMagickWand(magick_wand);
		return 0;
	}

	MagickSetImageMatteColor(magick_wand, pixel_wand);
	MagickSetImageMatte(magick_wand, MagickFalse);
	DestroyPixelWand(pixel_wand);

	return magick_wand;
}

/* creates blob from wand according to quality param in params and returns it's size */
unsigned char *prepare_blob(MagickWand *magick_wand, struct query_params *params, size_t *blob_len) {
	unsigned char *blob;
	MagickBooleanType status;

	debug(DEB,"Preparing BLOB");

	/* this will remove meta data - this is very important as photos have loads of it */
	status = MagickStripImage(magick_wand);
	if (status == MagickFalse) {
		debug(ERR,"Failed to Strip Image");
		DestroyMagickWand(magick_wand);
		return 0;
	}


	status = MagickSetFormat(magick_wand, OUT_FORMAT);
	if (status == MagickFalse) {
		debug(ERR,"Failed to set output Format");
		DestroyMagickWand(magick_wand);
		return 0;
	}

	if (params->lowq)
		status = MagickSetCompressionQuality(magick_wand, LOWQ_QUALITY);
	else
		status = MagickSetCompressionQuality(magick_wand, NORMAL_QUALITY);
	if (status == MagickFalse) {
		debug(ERR,"Failed to set Compression Quality");
		DestroyMagickWand(magick_wand);
		return 0;
	}
	
	blob = MagickGetImageBlob(magick_wand, blob_len);
	if (!blob || !(*blob_len)) {
		debug(ERR,"Failed to Get Image Blob");
		DestroyMagickWand(magick_wand);
		return 0;
	}

	debug(DEB,"Prepared BLOB szie: %u", *blob_len);
	return blob;
}

void free_blob(unsigned char *blob) {
	MagickRelinquishMemory(blob);
}


MagickWand *resize(MagickWand *magick_wand, struct dimmensions to_size) {
	struct dimmensions image_size;
	MagickBooleanType status;

	image_size.w = MagickGetImageWidth(magick_wand);
	image_size.h = MagickGetImageHeight(magick_wand);

	to_size = resize_to_fit_in(image_size, to_size);

	/* we are reducing requested thumbnail resolution to MAX_PIXEL_NO */
	to_size = reduce_filed(to_size, MAX_PIXEL_NO);

	status = MagickResizeImage(magick_wand, to_size.w, to_size.h, RESIZE_FILTER, RESIZE_SMOOTH_FACTOR);
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

	/* we are reducing requested thumbnail resolution to MAX_PIXEL_NO */
	to_size = reduce_filed(to_size, MAX_PIXEL_NO);

	/* now it is time to resize to to_szie */
	status = MagickResizeImage(magick_wand, to_size.w, to_size.h, RESIZE_FILTER, RESIZE_SMOOTH_FACTOR);
	if (status == MagickFalse) {
		DestroyMagickWand(magick_wand);
		return 0;
	}

	return magick_wand;
}

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

MagickWand *remove_transparency(MagickWand *image);

/* TODO: Zero sized image will do orginal size? may by configuralble? :D */
/* TODO: Performace tests... profiler? :D */

/* Loads image, strips metadata and sets it's default bg color */
MagickWand *load_image(char *file_name) {
	char *path;
	MagickWand *image;
	MagickBooleanType status;

	path = malloc(strlen(MEDIA_PATH) + strlen(file_name) + 1);
	strcpy(path, MEDIA_PATH);
	strcat(path, file_name);

	debug(DEB,"Loading image: '%s'", path);

	image = NewMagickWand();
	if (!image) {
		debug(ERR, "Creating new magick wand failed!");
		return 0;
	}

	status = MagickReadImage(image, path);
	if (status == MagickFalse) {
		debug(WARN,"Loading image '%s' failed", path);
		DestroyMagickWand(image);
		return 0;
	}

	/* this will remove meta data - this is very important as photos have loads of it */
	status = MagickStripImage(image);
	if (status == MagickFalse) {
		debug(ERR,"Failed to Strip Image");
		DestroyMagickWand(image);
		return 0;
	}

#ifdef REMOVE_TRANSPARENCY
	image = remove_transparency(image);
	if (!image) {
		debug(ERR, "Removing transparency failed!");
		return 0;
	}
#endif

	return image;
}

/* If image has transparency/mate/alpha channel will replace transparent places with DEFAULT_BACKGROUND_COLOR */
MagickWand *remove_transparency(MagickWand *image) {
	MagickBooleanType status;
	MagickWand *new_image;
	PixelWand *bg_color;

	/* If image dosn't have alpha/transparency/mate channel we do nothing here */
	if (MagickGetImageMatte(image) == MagickFalse)
		return image;

	debug(DEB, "Removing transparency and colloring it to background color '%s'", DEFAULT_BACKGROUND_COLOR);

	/* Set background to DEFAULT_BACKGROUND_COLOR - in case of transparent gifs or png */
	bg_color = NewPixelWand();
	if (!bg_color) {
		debug(ERR, "Creating new pixel wand failed!");
		DestroyMagickWand(image);
		return 0;
	}

	status = PixelSetColor(bg_color, DEFAULT_BACKGROUND_COLOR);
	if (status == MagickFalse) {
		debug(ERR, "Failed to set Pixel Wand Color to '%s'", DEFAULT_BACKGROUND_COLOR);
		DestroyPixelWand(bg_color);
		DestroyMagickWand(image);
		return 0;
	}

	new_image = NewMagickWand();
	if (!new_image) {
		debug(ERR, "Creating new magick wand failed!");
		DestroyPixelWand(bg_color);
		DestroyMagickWand(image);
		return 0;
	}

	status = MagickNewImage(new_image, MagickGetImageWidth(image), MagickGetImageHeight(image), bg_color);
	if (status == MagickFalse) {
		debug(ERR, "Failed to create new image");
		DestroyPixelWand(bg_color);
		DestroyMagickWand(new_image);
		DestroyMagickWand(image);
		return 0;
	}

	DestroyPixelWand(bg_color);

	MagickCompositeImage(new_image, image, OverCompositeOp, 0, 0);
	if (status == MagickFalse) {
		debug(ERR, "Composite image failed");
		DestroyMagickWand(new_image);
		DestroyMagickWand(image);
		return 0;
	}

	DestroyMagickWand(image);

	return new_image;
}

void free_image(MagickWand *image) {
	DestroyMagickWand(image);
}

/* creates blob from wand according to quality param in params and returns it's size */
unsigned char *prepare_blob(MagickWand *image, int quality, size_t *blob_len,const char *format) {
	unsigned char *blob;
	MagickBooleanType status;

	debug(DEB,"Preparing BLOB");

	status = MagickSetFormat(image, format);
	if (status == MagickFalse) {
		debug(ERR,"Failed to set output Format");
		DestroyMagickWand(image);
		return 0;
	}

	status = MagickSetCompressionQuality(image, quality);
	if (status == MagickFalse) {
		debug(ERR,"Failed to set Compression Quality");
		DestroyMagickWand(image);
		return 0;
	}
	
	blob = MagickGetImageBlob(image, blob_len);
	if (!blob || !(*blob_len)) {
		debug(ERR,"Failed to Get Image Blob");
		DestroyMagickWand(image);
		return 0;
	}

	debug(DEB,"Prepared BLOB szie: %u", *blob_len);
	return blob;
}

void free_blob(unsigned char *blob) {
	MagickRelinquishMemory(blob);
}

/* TODO: implement non aspect ratio keeping resize */

/* Resize the image to to_size dimmensons keeping aspect ration and fitting into to_size dimmensions effectivly using to_size width and height as the limits */
MagickWand *fit_resize(MagickWand *image, struct dimmensions to_size) {
	struct dimmensions image_size;
	MagickBooleanType status;

	image_size.w = MagickGetImageWidth(image);
	image_size.h = MagickGetImageHeight(image);

	to_size = resize_to_fit_in(image_size, to_size);

	/* we are reducing requested thumbnail resolution to MAX_PIXEL_NO */
	to_size = reduce_filed(to_size, MAX_PIXEL_NO);

	status = MagickResizeImage(image, to_size.w, to_size.h, RESIZE_FILTER, RESIZE_SMOOTH_FACTOR);
	if (status == MagickFalse) {
		DestroyMagickWand(image);
		return 0;
	}

	return image;
}

/* This will do so called strict scaling. It will resize the image to to_size dimmensions cutting off image regions to keep constant aspect ratio */
MagickWand *strict_resize(MagickWand *image, struct dimmensions to_size) {
	struct dimmensions image_size, crop_size;
	int x, y;
	MagickBooleanType status;

	image_size.w = MagickGetImageWidth(image);
	image_size.h = MagickGetImageHeight(image);

	debug(DEB, "Doing CropResize: orginal: %d x %d to: %d x %d", image_size.w, image_size.h, to_size.w, to_size.h);

	/* fit to_szie into image_size */
	crop_size = resize_to_fit_in(to_size, image_size);
	debug(DEB, "Crop size: %d x %d", crop_size.w, crop_size.h);

	/* calculate to center crop */
	x = (image_size.w - crop_size.w) / 2;
	y = (image_size.h - crop_size.h) / 2;
	debug(DEB, "Crop center: %d x %d", x, y);

	status = MagickCropImage(image, crop_size.w, crop_size.h, x, y);
	if (status == MagickFalse) {
		DestroyMagickWand(image);
		return 0;
	}

	/* we are reducing requested thumbnail resolution to MAX_PIXEL_NO */
	to_size = reduce_filed(to_size, MAX_PIXEL_NO);

	/* now it is time to resize to to_szie */
	status = MagickResizeImage(image, to_size.w, to_size.h, RESIZE_FILTER, RESIZE_SMOOTH_FACTOR);
	if (status == MagickFalse) {
		DestroyMagickWand(image);
		return 0;
	}

	return image;
}

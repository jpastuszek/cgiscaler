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


#include <string.h>

#include "scaler.h"
#include "config.h"
#include "file_utils.h"
#include "geometry_math.h"
#include "cache.h"
#include "serve.h"
#include "debug.h"

#ifdef DEBUG
#include <sys/time.h>
#endif


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

/* TODO: Zero sized image will do original size? may by configurable? :D */
/* TODO: Performance tests... profiler? :D */

/* Loads image, strips meta-data and sets it's default bg color */
MagickWand *load_image(media_fpath *media_file_path, struct dimensions to_size) {
	MagickWand *image;
	abs_fpath *absolute_media_file_path;
	MagickBooleanType status;
	struct timer timing;
	Image *_image;
	ImageInfo *image_info;
	ExceptionInfo *exception;
	char *size;

	absolute_media_file_path = create_absolute_media_file_path(media_file_path);

	image_info = CloneImageInfo((ImageInfo *) NULL);

	strncpy(image_info->filename, absolute_media_file_path, MaxTextExtent);

	free_fpath(absolute_media_file_path);

	if (to_size.w != 0 && to_size.h != 0) {
		size = malloc(24); /* this will get removed on DestroyImageInfo */
		snprintf(size, 24, "%ux%u", to_size.w, to_size.h);
		image_info->size = size;
		debug(DEB, "Trying image size: %ux%u", to_size.w, to_size.h);
	}

	debug(DEB,"Loading image: '%s'", media_file_path);
	timer_start(&timing);

	exception = AcquireExceptionInfo();

	_image = ReadImage(image_info, exception);
	if (_image == (Image *) NULL) {
		DestroyExceptionInfo(exception);
		DestroyImageInfo(image_info);
		debug(WARN,"Loading image '%s' failed", image_info->filename);
		return 0;
	}

	DestroyExceptionInfo(exception);
	DestroyImageInfo(image_info);

	image = NewMagickWandFromImage(_image);
	if (!image) {
		debug(ERR, "Creating new magick wand failed!");
		return 0;
	}

	DestroyImage(_image);

	debug(PROF, "Loading took %.3f s",  timer_stop(&timing));

	#ifdef DEBUG
		to_size = get_image_size(image);
		debug(DEB, "Resoulting image size: %ux%u", to_size.w, to_size.h);
	#endif

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
	struct timer timeing;	


	/* If image doesn't have alpha/transparency/mate channel we do nothing here */
	if (MagickGetImageMatte(image) == MagickFalse)
		return image;

	debug(DEB, "Removing transparency and colloring it to background color '%s'", DEFAULT_BACKGROUND_COLOR);
	timer_start(&timeing);

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
	debug(PROF, "Removing transparency took %.3f s",  timer_stop(&timeing));

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

	debug(DEB,"Prepared BLOB size: %u", *blob_len);
	return blob;
}

void free_blob(unsigned char *blob) {
	MagickRelinquishMemory(blob);
}

/* TODO: implement non aspect ratio keeping re-size */

/* Re-size the image to resize_to dimensions keeping aspect ration and fitting into resize_to dimensions effectively using resize_to width and height as the limits */
MagickWand *fit_resize(media_fpath *media_file_path, struct dimensions resize_to) {
	MagickWand *image_ping;
	MagickWand *image;
	struct dimensions image_size;
	struct dimensions load_size;

	/* this will ping the image to get it's size */
	image_ping = ping_image(media_file_path);
	if (!image_ping)
		return 0;

	image_size = get_image_size(image_ping);

	/* we don't need our ping any more */
	/* TODO: This is not releasing all the memory */
	free_image(image_ping);

	/* this will calculate target size for aspect ratio keeping resize method */
	resize_to = resize_to_fit_in(image_size, resize_to);

	/* we are reducing requested thumbnail resolution to MAX_PIXEL_NO */
	resize_to = reduce_filed(resize_to, MAX_PIXEL_NO);

	load_size = resize_to;
	load_size.w *= 2;
	load_size.h *= 2;

	/* loading image... if it fails wand will be 0 */
	image = load_image(media_file_path, load_size);
	if (!image) {
		return 0;
	}

	image = resize(image, resize_to, image_size);
	if (!image)
		return 0;

	return image;
}

unsigned char *fit_resize_to_blob(media_fpath *media_file_path, struct dimensions resize_to, int quality, size_t *blob_len, const char *format) {
	MagickWand *image;
	unsigned char *blob;

	image = fit_resize(media_file_path, resize_to);
	if (!image)
		return 0;

	blob = prepare_blob(image, quality, blob_len, format);
	if (!blob) {
		free_image(image);
		return 0;
	}

	free_image(image);

	return blob;
}

/* This will do so called strict scaling. It will re-size the image to resize_to dimensions cutting off image regions to keep constant aspect ratio */
MagickWand *strict_resize(media_fpath *media_file_path, struct dimensions resize_to) {
	MagickWand *image;
	struct dimensions image_size;
	struct dimensions load_size;
	struct dimensions crop_to;

	/* we are reducing requested thumbnail resolution to MAX_PIXEL_NO */
	resize_to = reduce_filed(resize_to, MAX_PIXEL_NO);

	load_size = resize_to;
	load_size.w *= 2;
	load_size.h *= 2;

	/* loading image... if it fails wand will be 0 */
	image = load_image(media_file_path, load_size);
	if (!image)
		return 0;

	image_size = get_image_size(image);
	debug(DEB, "Doing CropResize: original: %d x %d to: %d x %d", image_size.w, image_size.h, resize_to.w, resize_to.h);

	/* fit to_size into image_size */
	crop_to = resize_to_fit_in(resize_to, image_size);
	debug(DEB, "Crop to: %d x %d", crop_to.w, crop_to.h);

	image = crop(image, crop_to, (image_size.w - crop_to.w) / 2, (image_size.h - crop_to.h) / 2);
	if (!image)
		return 0;

	image = resize(image, resize_to, image_size);
	if (!image)
		return 0;

	return image;
}

unsigned char *strict_resize_to_blob(media_fpath *media_file_path, struct dimensions resize_to, int quality, size_t *blob_len, const char *format) {
	MagickWand *image;
	unsigned char *blob;

	image = strict_resize(media_file_path, resize_to);
	if (!image)
		return 0;

	blob = prepare_blob(image, quality, blob_len, format);
	if (!blob) {
		free_image(image);
		return 0;
	}

	free_image(image);

	return blob;
}


struct dimensions get_image_size(MagickWand *image) {
	struct dimensions image_size;

	image_size.w = MagickGetImageWidth(image);
	image_size.h = MagickGetImageHeight(image);

	return image_size;
}

MagickWand *ping_image(media_fpath *media_file_path) {
	abs_fpath *absolute_media_file_path;
	MagickBooleanType status;
	MagickWand *image;

	image = NewMagickWand();
	if (!image) {
		debug(ERR, "Creating new magick wand failed!");
		return 0;
	}

	absolute_media_file_path = create_absolute_media_file_path(media_file_path);

	status = MagickPingImage(image, absolute_media_file_path);
	if (status == MagickFalse) {
		debug(ERR, "Pinging image (obtaining size and format) failed!");
		free_fpath(absolute_media_file_path);
		DestroyMagickWand(image);
		return 0;
	}

	free_fpath(absolute_media_file_path);
	return image;
}

/* TODO: implement something better... */
int apply_pre_resize_factor(int orginal, int target) {
	if (target * 5 > orginal)
		return orginal;

	return target * 5;
}

MagickWand *resize(MagickWand *image, struct dimensions to_size, struct dimensions image_size) {
	MagickBooleanType status;
	struct timer timeing;

	timer_start(&timeing);

	status = MagickAdaptiveResizeImage(image, apply_pre_resize_factor(image_size.w, to_size.w), apply_pre_resize_factor(image_size.h, to_size.h));
	if (status == MagickFalse) {
		DestroyMagickWand(image);
		return 0;
	}

	status = MagickResizeImage(image, to_size.w, to_size.h, RESIZE_FILTER, RESIZE_SMOOTH_FACTOR);
	if (status == MagickFalse) {
		DestroyMagickWand(image);
		return 0;
	}

	debug(PROF, "Resize took %.3f s",  timer_stop(&timeing));

	return image;
}

MagickWand *crop(MagickWand *image, struct dimensions to_size, int x, int y) {
	MagickBooleanType status;
	struct timer timeing;

	timer_start(&timeing);

	status = MagickCropImage(image, to_size.w, to_size.h, x, y);
	if (status == MagickFalse) {
		DestroyMagickWand(image);
		return 0;
	}

	debug(PROF, "Crop took %.3f s",  timer_stop(&timeing));

	return image;
}

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

#include <string.h>

#include "scaler.h"
#include "runtime_config.h"
#include "file_utils.h"
#include "geometry_math.h"
#include "cache.h"
#include "serve.h"
#include "debug.h"

#ifdef DEBUG
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#endif

#define INFINIT_SIZE 0x0FFFFFFF

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

//TODO: Performance tests... profiler? :D

/** Structure that maps resize filters to their names */
struct _resize_filters resize_filters[11] =
{
	{BesselFilter,		"BesselFilter"},
	{BlackmanFilter,	"BlackmanFilter"},
	{BoxFilter,		"BoxFilter"},
	{CatromFilter,		"CatromFilter"},
	//{CubicGaussianFilter, "CubicGaussianFilter"},
	{HanningFilter,		"HanningFilter"},
	{HermiteFilter,		"HermiteFilter"},
	{LanczosFilter,		"LanczosFilter"},
	{MitchellFilter,	"MitchellFilter"},
	//{PointQuandraticFilter, "PointQuandraticFilter"},
	{SincFilter,		"SincFilter"},
	{TriangleFilter,	"TriangleFilter"},
	{-1,0}
};

/** Loads image, strips meta-data and sets it's default background color.
* This function will load image from file, strip meta-data and if configured it will replace transparent pixels with solid color.
* @param media_file_path path to image file to load
* @param to_size maximum size of image to read from file if storage format supports this - JPG does
* @return allocated new MagickWand object representing loaded image
*/
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
		debug(DEB, "Trying to load sub image of size: %ux%u", to_size.w, to_size.h);
	}

	debug(DEB,"Loading image: '%s'", image_info->filename);
	timer_start(&timing);

	exception = AcquireExceptionInfo();

	_image = ReadImage(image_info, exception);
	if (_image == (Image *) NULL) {
		DestroyExceptionInfo(exception);
		debug(WARN,"Loading image '%s' failed", image_info->filename ? image_info->filename : "<null>");
		DestroyImageInfo(image_info);
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
		debug(DEB, "Loaded image of size: %ux%u", to_size.w, to_size.h);
	#endif

	/* this will remove meta data - this is very important as photos have loads of it */
	status = MagickStripImage(image);
	if (status == MagickFalse) {
		debug(ERR,"Failed to Strip Image");
		DestroyMagickWand(image);
		return 0;
	}

	/* this will flip image with new image that has transparent areas colored */
	if (output_config->remove_transparency)
		image = remove_transparency(image);

	return image;
}

/** If image has transparency/mate/alpha channel this function will replace transparent places with TRANSPARENCY_COLOUR.
* @param image image to remove transparency from
* @return original image if no need of removal or on failure, processed image on success
*/
MagickWand *remove_transparency(MagickWand *image) {
	MagickBooleanType status;
	MagickWand *new_image;
	PixelWand *bg_color;
	struct timer timeing;	


	/* If image doesn't have alpha/transparency/mate channel we do nothing here */
	if (MagickGetImageMatte(image) == MagickFalse) {
		debug(DEB, "No need to remove transparency");
		return image;
	}

	debug(DEB, "Removing transparency and colloring it to background color '%s'", output_config->transparency_replacement_color);
	timer_start(&timeing);

	/* Set background to TRANSPARENCY_COLOUR - in case of transparent gifs or png */
	bg_color = NewPixelWand();
	if (!bg_color) {
		debug(ERR, "Creating new pixel wand failed!");
		return image;
	}

	status = PixelSetColor(bg_color, output_config->transparency_replacement_color);
	if (status == MagickFalse) {
		debug(ERR, "Failed to set Pixel Wand Color to '%s'", output_config->transparency_replacement_color);
		DestroyPixelWand(bg_color);
		return image;
	}

	new_image = NewMagickWand();
	if (!new_image) {
		debug(ERR, "Creating new magick wand failed!");
		DestroyPixelWand(bg_color);
		return image;
	}

	status = MagickNewImage(new_image, MagickGetImageWidth(image), MagickGetImageHeight(image), bg_color);
	if (status == MagickFalse) {
		debug(ERR, "Failed to create new image");
		DestroyPixelWand(bg_color);
		DestroyMagickWand(new_image);
		return image;
	}

	DestroyPixelWand(bg_color);

	MagickCompositeImage(new_image, image, OverCompositeOp, 0, 0);
	if (status == MagickFalse) {
		debug(ERR, "Composite image failed");
		DestroyMagickWand(new_image);
		return image;
	}

	DestroyMagickWand(image);
	debug(PROF, "Removing transparency took %.3f s",  timer_stop(&timeing));

	return new_image;
}

/** Release memory allocated for image object */
void free_image(MagickWand *image) {
	DestroyMagickWand(image);
}

/** This function will provide image byte representation in specified format.
* Possible formats are all formats supported by ImageMagick. For example "JPG" or "GIF" are valid values.
* @param image image from which generate data
* @param quality used to set quality parameter for compression method used - for JPG 100 is best 0 is worst quality
* @param blob_len pointer to size_t variable that will store size of returned data array
* @param format format of data that will be returned by this function
* @return pointer to allocated data or 0 on failure
*/
unsigned char *prepare_blob(MagickWand *image, int quality, size_t *blob_len,const char *format) {
	unsigned char *blob;
	MagickBooleanType status;

	debug(DEB,"Preparing BLOB");

	status = MagickSetFormat(image, format);
	if (status == MagickFalse) {
		debug(ERR,"Failed to set output Format");
		*blob_len = 0;
		return 0;
	}

	status = MagickSetCompressionQuality(image, quality);
	if (status == MagickFalse) {
		debug(ERR,"Failed to set Compression Quality");
		*blob_len = 0;
		return 0;
	}
	
	blob = MagickGetImageBlob(image, blob_len);
	if (!blob || !(*blob_len)) {
		debug(ERR,"Failed to Get Image Blob");
		*blob_len = 0;
		return 0;
	}

	debug(DEB,"Prepared BLOB size: %u", *blob_len);
	return blob;
}

/** Releases allocated memory. */
void free_blob(unsigned char *blob) {
	MagickRelinquishMemory(blob);
}

//TODO: implement non aspect ratio keeping re-size
/** Re-sizes image without loosing it's aspect ratio by fitting image in specified dimensions.
* Re-size the image to resize_to dimensions keeping aspect ration and fitting into resize_to dimensions effectively using resize_to width and height as the limits.
* @param media_file_path path to file that stores input image
* @param resize_to dimensions to which image should be re-sized
* @return allocated new MagickWand containing re-sized image or 0 on failure
* @see fit_resize()
*/
MagickWand *fit_resize(media_fpath *media_file_path, struct dimensions resize_to) {
	MagickWand *image_ping;
	MagickWand *image;
	struct dimensions image_size;
	struct dimensions load_size;

	debug(INFO, "Fit resize to: %dx%d", resize_to.w, resize_to.h);

	if (resize_to.w < 0 || resize_to.h < 0) {
		debug(ERR, "Requested invalid image size: %dx%d", resize_to.w, resize_to.h);
		return 0;
	}

	if (resize_to.w == 0 && resize_to.h == 0) {
		debug(ERR, "Requested invalid image size: 0x0");
		return 0;
	}

	if (resize_to.w == 0)
		resize_to.w = INFINIT_SIZE;

	if (resize_to.h == 0)
		resize_to.h = INFINIT_SIZE;

	/* this will ping the image to get it's size */
	image_ping = ping_image(media_file_path);
	if (!image_ping)
		return 0;

	image_size = get_image_size(image_ping);

	/* we don't need our ping any more */
	free_image(image_ping);

	/* this will calculate target size for aspect ratio keeping resize method */
	resize_to = resize_to_fit_in(image_size, resize_to);

	/* we are reducing requested thumbnail resolution to MAX_OUT_PIXELS */
	resize_to = reduce_filed(resize_to, resource_limit_config->max_pixel_no);

	load_size = resize_to;
	load_size.w *= 2;
	load_size.h *= 2;

	/* loading image... if it fails wand will be 0 */
	image = load_image(media_file_path, load_size);
	if (!image) {
		return 0;
	}

	if (!resize(image, resize_to)) {
		/* Freeing loaded image */
		debug(ERR, "Image re-size failed in fit re-size");
		DestroyMagickWand(image);
		return 0;
	}

	return image;
}

/** Re-sizes image without loosing it's aspect ratio by fitting image in dimension's and returns it's byte representation.
* This function will re-size image using fit-re-size method and then will provide byte representation in format specified.
* Possible formats are all formats supported by ImageMagick. For example "JPG" or "GIF" are valid values.
* @param media_file_path path to file that stores input image
* @param resize_to dimensions to which image should be re-sized
* @param quality used to set quality parameter for compression method used - for JPG 100 is best 0 is worst quality
* @param blob_len pointer to size_t variable that will store size of returned data array
* @param format format of data that will be returned by this function
* @return pointer to allocated data or 0 on failure
* @see fit_resize()
*/
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


/** Re-sizes image without loosing it's aspect ratio by cutting off image regions.
* @param media_file_path path to file that stores input image
* @param resize_to dimensions to which image should be re-sized
* @return allocated new MagickWand containing re-sized image or 0 on failure
* @see fit_resize()
*/
MagickWand *strict_resize(media_fpath *media_file_path, struct dimensions resize_to) {
	MagickWand *image;
	struct dimensions image_size;
	struct dimensions load_size;
	struct dimensions crop_to;
	struct point position;

	debug(INFO, "Strict resize to: %dx%d", resize_to.w, resize_to.h);

	if (resize_to.w <= 0 || resize_to.h <= 0) {
		debug(ERR, "Requested invalid image size: %dx%d", resize_to.w, resize_to.h);
		return 0;
	}

	/* we are reducing requested thumbnail resolution to MAX_OUT_PIXELS */
	resize_to = reduce_filed(resize_to, resource_limit_config->max_pixel_no);

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

	position.x = (image_size.w - crop_to.w) / 2;
	position.y = (image_size.h - crop_to.h) / 2;

	if (!crop(image, crop_to, position)) {
		debug(ERR, "Image crop  failed in strict re-size");
		/* Freeing loaded image */
		DestroyMagickWand(image);
		return 0;
	}

	if (!resize(image, resize_to)) {
		debug(ERR, "Image re-size  failed in strict re-size");
		/* Freeing loaded image */
		DestroyMagickWand(image);
		return 0;
	}

	return image;
}

/** Re-sizes image without loosing it's aspect ratio by cropping and returns it's byte representation.
* This function will re-size image using strict method and then will provide byte representation in format specified.
* Possible formats are all formats supported by ImageMagick. For example "JPG" or "GIF" are valid values.
* @param media_file_path path to file that stores input image
* @param resize_to dimensions to which image should be re-sized
* @param quality used to set quality parameter for compression method used - for JPG 100 is best 0 is worst quality
* @param blob_len pointer to size_t variable that will store size of returned data array
* @param format format of data that will be returned by this function
* @return pointer to allocated data or 0 on failure
* @see strict_resize()
*/
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

/** Get image size from MagicWand.
* @param image image to get size from
* @return dimmensions of image
*/
struct dimensions get_image_size(MagickWand *image) {
	struct dimensions image_size;

	image_size.w = MagickGetImageWidth(image);
	image_size.h = MagickGetImageHeight(image);

	return image_size;
}

/** Gets image headers form file given by path. 
* @param media_file_path path to file containing image to ping
* @return pointer to new MagickWand containing image information or 0 on failure
* @see load_image()
*/
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

//TODO: implement something better...
/** Calcuates pre resize size values. */
int apply_pre_resize_factor(int orginal, int target) {
	if (target * 5 > orginal)
		return orginal;

	return target * 5;
}

/** Tries to pre-scale image using fastest scaling method.
* This will pre-scale image so high quality scaler can work on reduced data set.
**/
short int do_pre_scale_if_necessary(MagickWand *image, struct dimensions to_size) {
	MagickBooleanType status;
	struct dimensions image_size;
	struct dimensions image_prescale_size;

	image_size = get_image_size(image);

	/* Try to figure out to what size to do pre-scaling using fast scaling method - this normally be only usefull for big non JPEG images */
	image_prescale_size.w = apply_pre_resize_factor(image_size.w, to_size.w);
	image_prescale_size.h = apply_pre_resize_factor(image_size.h, to_size.h);

	if (image_prescale_size.w != image_size.w || image_prescale_size.h != image_size.h) {
		debug(INFO, "Using fast pre-scale");

		status = MagickSampleImage(image, image_prescale_size.w, image_prescale_size.h);
		if (status == MagickFalse)
			return 0;
	}

	return 1;
}

/** Resize image to given dimensions.
* This function will resize image loosing it's aspect ratio to achieve required size.
* @param image pointer to MagickWand containing image to resize
* @param to_size dimensions of resulting image
* @return 1 on success or 0 on failure
* @see crop()
*/
short int resize(MagickWand *image, struct dimensions to_size) {
	MagickBooleanType status;
	struct timer timeing;

	if (to_size.w <= 0 || to_size.h <= 0) {
		debug(ERR, "Requested invalid image size: %dx%d", to_size.w, to_size.h);
		return 0;
	}

	timer_start(&timeing);

	if (! do_pre_scale_if_necessary(image, to_size))
		debug(WARN, "Fast pre-scale failed...");

	/* Full filtered resize */
	status = MagickResizeImage(image, to_size.w, to_size.h, output_config->scaling_filter, output_config->blur_factor);
	if (status == MagickFalse) {
		debug(ERR, "Image resize failed!");
		return 0;
	}

	debug(PROF, "Resize took %.3f s",  timer_stop(&timeing));

	return 1;
}

/** Crops image to given dimensions starting at given position from left top corner.
* @param image pointer to MagickWand containing image to crop
* @param to_size dimensions to crop image to
* @param position point from loft top corner to start crop from
* @return 0 on failure, 1 on success
* @see resize()
*/
short int crop(MagickWand *image, struct dimensions to_size, struct point position) {
	MagickBooleanType status;
	struct timer timeing;

	if (to_size.w <= 0 || to_size.h <= 0) {
		debug(ERR, "Requested invalid image size: %ux%u", to_size.w, to_size.h);
		return 0;
	}

	timer_start(&timeing);

	status = MagickCropImage(image, to_size.w, to_size.h, position.x, position.y);
	if (status == MagickFalse) {
		debug(ERR, "Image crop failed!");
		return 0;
	}

	status = MagickSetImagePage(image, to_size.w, to_size.h, 0, 0);
	if (status == MagickFalse) {
		debug(ERR, "Image crop failed while setting image page!");
		return 0;
	}

	debug(PROF, "Crop took %.3f s",  timer_stop(&timeing));

	return 1;
}

/** Sets ImageMagick resource limit
* @param file maximum number of open pixel cache files
* @param disk maximum amount of disk space permitted for use by the pixel cache  in GB
* @param map maximum amount of memory map to allocate for the pixel cache in MB - when this limit is exceeded, the  image pixels are cached to disk
* @param memory maximum amount of memory to allocate for the pixel cache from the heap in MB - when this limit is exceeded, the image pixels are cached to memory-mapped disk 
* @param area maximum amount of memory to allocate for image from in MB - images that exceed the area limit are cached to disk
* @return 1 on success 0 on failure
*/
int set_resource_limits(int disk, int map, int file, int memory, int area) {
	MagickBooleanType status;

	debug(DEB, "Setting Magick resource limit: disk %i, map %i, file %i, memory %i", disk, map, file, memory);

	status = SetMagickResourceLimit(DiskResource, disk);
	if (status == MagickFalse) {
		debug(ERR, "Setting Magick resource limit for disk failed!");
		return 0;
	}

	status = SetMagickResourceLimit(MapResource, map);
	if (status == MagickFalse) {
		debug(ERR, "Setting Magick resource limit for map failed!");
		return 0;
	}

	status = SetMagickResourceLimit(FileResource, file);
	if (status == MagickFalse) {
		debug(ERR, "Setting Magick resource limit for file failed!");
		return 0;
	}

	status = SetMagickResourceLimit(MemoryResource, memory);
	if (status == MagickFalse) {
		debug(ERR, "Setting Magick resource limit for memory failed!");
		return 0;
	}

	status = SetMagickResourceLimit(AreaResource, area);
	if (status == MagickFalse) {
		debug(ERR, "Setting Magick resource limit for area failed!");
		return 0;
	}

#ifdef DEBUG
	FILE *debug_file_file;
	debug_file_file =  fdopen(get_debug_file_fd(), "a");
	if (!debug_file_file)
		debug(DEB, "Getting FILE stream for debug file fd failed: %s", strerror(errno));
	else
		ListMagickResourceInfo(debug_file_file, 0);
#endif

	return 1;
}

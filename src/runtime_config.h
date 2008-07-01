/***************************************************************************
 *   Copyright (C) 2007, 2008 by Jakub Pastuszek   *
 *   jpastuszek@gmail.com   *
 *	                                                                 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.	                           *
 *	                                                                 *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of	*
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	 *
 *   GNU General Public License for more details.	                  *
 *	                                                                 *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the	                 *
 *   Free Software Foundation, Inc.,	                               *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.	     *
 ***************************************************************************/

#ifndef RUNTIME_CONFIG_H
#define RUNTIME_CONFIG_H

#include "geometry_math.h"
#include "format_info.h"

/* All global configuration structures */
struct output_config *output_config;
struct operation_config *operation_config;
struct logging_config *logging_config;

struct simple_query_string_config *simple_query_string_config;

struct storage_config *storage_config;
struct error_handling_config *error_handling_config;
struct resource_limit_config *resource_limit_config;

char **scale_method_names;

/** Possible scaling methods */
enum scale_methods {
	SM_FIT,
	SM_STRICT,
	SM_FREE
};

/** Configuration of output data format */
struct output_config {
	/** File name (relative path) received from CGI to produce output from */
	char *file_name;

	/** Image format information */
	struct format_info *format;
	/** If mime type cannot be determined this default will be used */
	char *fail_mime_type;
	/** Requested thumbnail dimensions */
	struct dimensions size;
	/** Requested thumbail quality */
	unsigned short int quality;
	/** Scaling mode - strict, fit, free, reduction */
	unsigned short int scale_method;

	/** Filter method to use */
	int scaling_filter;
	/** "Blur factor where > 1 is blurry, < 1 is sharp" */
	float blur_factor;

	/** When transparent image is scaled it's transparency will be replaced by transparency_color */
	unsigned short int remove_transparency;
	/** Color to fill transparency when converting from transparent formats */
	char *transparency_replacement_color;
};

/** Possible query string parsing modes */
enum query_string_modes {
	QSM_SIMPLE,
	QSM_CLASS,
	QSM_FULL
};

/** General operation settings */
struct operation_config {
	/** Which query string parsing method to use */
	unsigned short int query_string_mode;
	/** Do not use cache */
	unsigned short int no_cache;
	/** Do not serve image to STDOUT */
	unsigned short int no_serve;
	/** Do not produce HTTP headers to STDOUT */
	unsigned short int no_headers;
};

/** Logging and debug related dynamic configuration */
struct logging_config {
	/** Path to log file */
	char *log_file;
	/** Log level to filter log entries */
	unsigned short int log_level;
};

/** CGI query string matching patterns */
struct simple_query_string_config {
	/** String to match width parameter in CGI query string */
	char *query_width_param;
	/** String to match height parameter in CGI query string */
	char *query_height_param;
	/** String to match strict enable parameter in CGI query string */
	char *query_strict_param;
	/** String to mach log quality enable parameter in CGI query string */
	char *query_lowq_param;

	/** String to mach true value in CGI query string */
	char *query_true_param;
	/** String to mach false value in CGI query string */
	char *query_false_param;

	/** If true low quality value will be used for output quality setting */
	unsigned short int use_loq_quality;

	/** Use this quality when low quality is enabled */
	unsigned short int low_quality_value;

	/** Use this quality when low quality is disabled */
	unsigned short int default_quality_value;
};

/*
struct class_query_string_config {
};

struct full_query_string_config {
};
*/

/** Storage configuration */
struct storage_config {
	/** Path to media directory that  images will be served from */
	char *media_directory;
	/** Path to cache directory where generated thumbnails will be stored and served from */
	char *cache_directory;
};

/** Defines configuration for error handling */
struct error_handling_config {
	/** Path to image file that will be served on errors */
	char *error_image_file;
	/** In case error image is not available this text will be served */
	char *error_message;
};

/** Defines resource limiting configuration */
struct resource_limit_config {
	/** Maximum number of pixels that output image will be limited to */
	unsigned long int max_pixel_no;
	/** Maximum number of open pixel cache files  */
	unsigned long int file_limit;
	/** Maximum amount of disk space permitted for use by the pixel cache  in GB */
	unsigned long int disk_limit;
	/** Maximum amount of memory map to allocate for the pixel cache in MB - when this limit is exceeded, the image pixels are cached to disk */
	unsigned long int map_limit;
	/** Maximum amount of memory to allocate for the pixel cache from the heap in MB - when this limit is exceeded, the image pixels are cached to memory-mapped disk */
	unsigned long int memory_limit;
	/** Maximum amount of memory to allocate for image from in MB - images that exceed the area limit are cached to disk  */
	unsigned long int area_limit;
};

#endif

void alloc_default_config();
void free_config();

struct output_config *alloc_default_output_config();
void free_output_config(struct output_config *config);

struct operation_config *alloc_default_operation_config();
void free_operation_config(struct operation_config *config);

struct logging_config *alloc_default_logging_config();
void free_logging_config(struct logging_config *config);

struct simple_query_string_config* alloc_default_simple_query_string_config();
void free_simple_query_string_config(struct simple_query_string_config *config);

struct storage_config* alloc_default_storage_config();
void free_storage_config(struct storage_config *config);

struct error_handling_config* alloc_default_error_handling_config();
void free_error_handling_config(struct error_handling_config *config);

struct resource_limit_config* alloc_default_resource_limit_config();
void free_resource_limit_config(struct resource_limit_config *config);

void dump_runtime_configuration() ;

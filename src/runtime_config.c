/***************************************************************************
 *   Copyright (C) 2007, 2008 by Jakub Pastuszek   *
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

#include "runtime_config.h"

#include <stdlib.h>
#include <string.h>
#include <wand/MagickWand.h>
#include "debug.h"
#include "defaults.h"


char *scale_method_names_tmp[] = {"FIT", "STRICT", "FREE"};
char **scale_method_names = scale_method_names_tmp;

/** Allocate all coniguration structures and initializes them to default values.
* All defaults are defined in config.h.
*/
void alloc_default_config() {
	output_config = alloc_default_output_config();
	operation_config = alloc_default_operation_config();
	logging_config = alloc_default_logging_config();
	storage_config = alloc_default_storage_config();
	error_handling_config = alloc_default_error_handling_config();
	resource_limit_config = alloc_default_resource_limit_config();

	simple_query_string_config = alloc_default_simple_query_string_config();
}

/** Releases memory allocated to runtime configuration. */
void free_config() {
	free_resource_limit_config(resource_limit_config);
	free_error_handling_config(error_handling_config);
	free_storage_config(storage_config);
	free_logging_config(logging_config);
	free_operation_config(operation_config);
	free_output_config(output_config);

	free_simple_query_string_config(simple_query_string_config);
}

/** Allocates default output configuration.
* It will use constants defined in config.h as defaults.
* @return allocated structure initialized with default values
* @see free_output_config()
* @see runtime_config.h
*/
struct output_config *alloc_default_output_config() {
	struct output_config *config;
	config = malloc(sizeof(struct output_config));
	if (!config)
		exit(66);

	config->file_name = 0;

	config->format = format_to_format_info(OUT_FORMAT);
	config->fail_mime_type = strdup(FAIL_BACK_MIME_TYPE);

	config->size.w = WIDTH;
	config->size.h = HEIGHT;

	config->quality = NORMAL_QUALITY_VALUE;

	config->scale_method = SCALE_METHOD;

	config->scaling_filter = SCALING_FILTER;
	config->blur_factor = BLUR_FACTOR;

#ifdef REMOVE_TRANSPARENCY
	config->remove_transparency = 1;
#else
	config->remove_transparency = 0;
#endif

	config->transparency_replacement_color = strdup(TRANSPARENCY_COLOUR);

	return config;
}

/** Frees memory resources allocated for configuration structure. */
void free_output_config(struct output_config *config) {
	if (config->file_name)
		free(config->file_name);
	if (config->format)
		free_format_info(config->format);
	if (config->fail_mime_type)
		free(config->fail_mime_type);
	if (config->transparency_replacement_color)
		free(config->transparency_replacement_color);

	free(config);
}

/** Allocates default operation configuration.
* It will use constants defined in config.h as defaults.
* @return allocated structure initialized with default values
* @see free_operation_config()
*/
struct operation_config *alloc_default_operation_config() {
	struct operation_config *config;
	config = malloc(sizeof(struct operation_config));
	if (!config)
		exit(66);

	config->query_string_mode = QSM_SIMPLE;

	config->no_cache = NO_CACHE;
	config->no_serve = NO_SERVE;
	config->no_headers = NO_HEADERS;

	return config;	
}

/** Frees memory resources allocated for configuration structure. */
void free_operation_config(struct operation_config *config) {
	free(config);
}

/** Allocates default logging configuration.
* It will use constants defined in config.h as defaults.
* @return allocated structure initialized with default values
* @see free_logging_config()
*/
struct logging_config *alloc_default_logging_config() {
	struct logging_config *config;
	config = malloc(sizeof(struct logging_config));
	if (!config)
		exit(66);

	config->log_file = strdup(LOG_FILE);
	//TODO: Implement log levels
	config->log_level = 0;

	return config;	
}

/** Frees memory resources allocated for configuration structure. */
void free_logging_config(struct logging_config *config) {
	if (config->log_file)
		free(config->log_file);

	free(config);
}

//TODO: This should be somehow extensible so it would allow proper evolution of CGI interface. Current state is mostly for compability with product that is main reason of this code existence.
/** Allocates default query string parameters configuration.
* It will use constants defined in config.h as defaults.
* @return allocated structure initialized with default values
* @see free_query_string_config()
*/
struct simple_query_string_config* alloc_default_simple_query_string_config() {
	struct simple_query_string_config *config;
	config = malloc(sizeof(struct simple_query_string_config));
	if (!config)
		exit(66);

	config->query_width_param = strdup(CGI_WIDTH);
	config->query_height_param = strdup(CGI_HEIGHT);
	config->query_strict_param = strdup(CGI_STRICT);
	config->query_lowq_param = strdup(CGI_LOW_QUALITY);

	config->query_true_param = strdup(CGI_TRUE);
	config->query_false_param = strdup(CGI_FALSE);

	config->use_loq_quality = 0;
	config->low_quality_value = LOW_QUALITY_VALUE;
	config->default_quality_value = NORMAL_QUALITY_VALUE;

	return config;
}

/** Frees memory resources allocated for configuration structure. */
void free_simple_query_string_config(struct simple_query_string_config *config) {
	if (config->query_width_param)
		free(config->query_width_param);
	if (config->query_height_param)
		free(config->query_height_param);
	if (config->query_strict_param)
		free(config->query_strict_param);
	if (config->query_lowq_param)
		free(config->query_lowq_param);

	if (config->query_true_param)
		free(config->query_true_param);
	if (config->query_false_param)
		free(config->query_false_param);

	free(config);
}

/**  Allocates default storage configuration.
* It will use constants defined in config.h as defaults.
* @return allocated structure initialized with default values
* @see free_storage_config()
*/
struct storage_config* alloc_default_storage_config() {
	struct storage_config *config;
	config = malloc(sizeof(struct storage_config));
	if (!config)
		exit(66);

	config->media_directory = strdup(MEDIA_DIR);
	config->cache_directory = strdup(CACHE_DIR);

	return config;
}

/** Frees memory resources allocated for configuration structure. */
void free_storage_config(struct storage_config *config) {
	if (config->media_directory)
		free(config->media_directory);
	if (config->cache_directory)
		free(config->cache_directory);

	free(config);
}

/**  Allocates default error handling configuration.
* It will use constants defined in config.h as defaults.
* @return allocated structure initialized with default values
* @see free_error_handling_config()
*/
struct error_handling_config* alloc_default_error_handling_config() {
	struct error_handling_config *config;
	config = malloc(sizeof(struct error_handling_config));
	if (!config)
		exit(66);

	config->error_image_file = strdup(ERROR_FILE);
	config->error_message = strdup(ERROR_MESSAGE);

	return config;
}

/** Frees memory resources allocated for configuration structure. */
void free_error_handling_config(struct error_handling_config *config) {
	if (config->error_image_file)
		free(config->error_image_file);
	if (config->error_message)
		free(config->error_message);

	free(config);
}

/**  Allocates default resource limit configuration.
* It will use constants defined in config.h as defaults.
* @return allocated structure initialized with default values
* @see free_error_handling_config()
*/
struct resource_limit_config* alloc_default_resource_limit_config() {
	struct resource_limit_config *config;
	config = malloc(sizeof(struct resource_limit_config));
	if (!config)
		exit(66);

	config->max_pixel_no = MAX_OUT_PIXELS;

	config->file_limit = LIMIT_FILES;
	config->disk_limit = LIMIT_DISK;
	config->map_limit = LIMIT_MAP;
	config->memory_limit = LIMIT_MEMORY;
	config->area_limit = LIMIT_AREA;

	return config;
}

/** Frees memory resources allocated for configuration structure. */
void free_resource_limit_config(struct resource_limit_config *config) {
	free(config);
}

/** Dumps running configuration to the log file */
void dump_runtime_configuration() {
#ifdef DEBUG
	debug(DEB, "Run-time config: file: '%s', size w: %d h: %d, scale method: %s quality: %d Operation coifig: no cache: %d, no serve: %d, no headers: %d", output_config->file_name ? output_config->file_name : "<null>", output_config->size.w, output_config->size.h, scale_method_names[output_config->scale_method], output_config->quality, operation_config->no_cache, operation_config->no_serve, operation_config->no_headers);
#endif
}

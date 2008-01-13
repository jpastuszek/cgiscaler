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

#include <stdlib.h>
#include <string.h>
#include "runtime_config.h"
#include "config.h"

/** Allocates default runtime configuration.
* It will use constants defined in config.h as defaults.
* @return allocated structure initialized with default values
*/
struct runtime_config *alloc_default_runtime_config() {
	struct runtime_config *config;
	config = malloc(sizeof(struct runtime_config));
	if (!config)
		exit(66);

	config->file_name = 0;
	config->size.w = DEFAULT_WIDTH;
	config->size.h = DEFAULT_HEIGHT;
	config->strict = DEFAULT_STRICT;
	config->quality = DEFAULT_QUALITY;

	return config;
}

/** Frees memory resources allocated for configuration structure. */
void free_runtime_config(struct runtime_config *config) {
	if (config->file_name)
		free(config->file_name);
	free(config);
}

/** Allocates default operation configuration.
* It will use constants defined in config.h as defaults.
* @return allocated structure initialized with default values
*/
struct operation_config *alloc_default_operation_config() {
	struct operation_config *config;
	config = malloc(sizeof(struct operation_config));
	if (!config)
		exit(66);

	config->no_cache = DEFAULT_NO_CACHE;
	config->no_serve = DEFAULT_NO_SERVE;
	config->no_headers = DEFAULT_NO_HEADERS;

	return config;	
}

/** Frees memory resources allocated for configuration structure. */
void free_operation_config(struct operation_config *config) {
	free(config);
}

/** Allocates default logging configuration.
* It will use constants defined in config.h as defaults.
* @return allocated structure initialized with default values
*/
struct logging_config *alloc_default_logging_config() {
	struct logging_config *config;
	config = malloc(sizeof(struct logging_config));
	if (!config)
		exit(66);

	//TODO: Rename constant and related to "LOG_FILE"
	config->log_file = malloc(strlen(DEBUG_FILE) + 1);
	strcpy(config->log_file, DEBUG_FILE);
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
*/
struct query_string_config* alloc_default_query_string_config() {
	struct query_string_config *config;
	config = malloc(sizeof(struct query_string_config));
	if (!config)
		exit(66);

	config->query_width_param = QUERY_WIDTH_PARAM;
	config->query_height_param = QUERY_HEIGHT_PARAM;
	config->query_strict_param = QUERY_STRICT_PARAM;
	config->query_lowq_param = QUERY_LOWQ_PARAM;

	config->query_true_param = QUERY_TRUE_VAL;
	config->query_false_param = QUERY_FALSE_VAL;

	config->low_quality = LOWQ_QUALITY;
	config->default_quality = DEFAULT_QUALITY;

	return config;
}

/** Frees memory resources allocated for configuration structure. */
void free_query_string_config(struct query_string_config *config) {
	free(config);
}


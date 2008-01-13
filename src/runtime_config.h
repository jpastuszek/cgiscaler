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

#ifndef RUNTIME_CONFIG_H 
#define RUNTIME_CONFIG_H

#include "geometry_math.h"

/** Holds runtime request parameters. */
struct runtime_config {
	/** File name (relative path) received from CGI. */
	char *file_name;
	/** Requested thumbnail dimensions */
	struct dimensions size;
	/** Perform strict scaling */
	unsigned short int strict;
	/** Requested thumbail quality */
	unsigned short int quality;
};

/** General operation settings */
struct operation_config {
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
struct query_string_config {
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

	/** Quality to use when lowq is enabled */
	unsigned short int low_quality;

	/** Quality to use when lowq is disabled */
	unsigned short int default_quality;
};

struct runtime_config *alloc_default_runtime_config();
void free_runtime_config(struct runtime_config *config);

struct operation_config *alloc_default_operation_config();
void free_operation_config(struct operation_config *config);

struct logging_config *alloc_default_logging_config();
void free_logging_config(struct logging_config *config);

struct query_string_config* alloc_default_query_string_config();
void free_query_string_config(struct query_string_config *config);

#endif

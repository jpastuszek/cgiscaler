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

#include "query_string.h"
#include "runtime_config.h"
#include "file_utils.h"
#include "debug.h"

extern struct output_config *output_config;
extern struct simple_query_string_config *simple_query_string_config;

#ifdef DEBUG
extern char **scale_method_names;
#endif

/* TODO: %xx in query string decoding... no need for filename (apache) */
/** Apply configuration changes by reading provided CGI query string and file name.
* @param file_name path to file to update runtime configuration with
* @param query_string string to parse - should be in CGI format
*/
void apply_simple_query_string_config(char *file_name, char *query_string) {
	char *w;
	char *h;
	char *s;
	char *lowq;

	/* strange env... failing */
	if (!output_config || !file_name)
		return;

	debug(DEB, "Processing query file name: '%s'", file_name);

	/* this will allocat the file name */
	file_name = sanitize_file_path(file_name);
	if (file_name) {
		if (output_config->file_name)
			free(output_config->file_name);
		output_config->file_name = file_name;
	}

	debug(DEB, "Processing query string: '%s' target name: '%s'", query_string, file_name);

	w = get_query_string_param(query_string, simple_query_string_config->query_width_param);
	h = get_query_string_param(query_string, simple_query_string_config->query_height_param);
	s = get_query_string_param(query_string, simple_query_string_config->query_strict_param);
	lowq = get_query_string_param(query_string, simple_query_string_config->query_lowq_param);

	if (w) {
		output_config->size.w = atoi(w);
		free(w);
	}

	if (h) {
		output_config->size.h = atoi(h);
		free(h);
	}

	if (s) {
		if (!strcmp(s, simple_query_string_config->query_true_param))
			output_config->scale_method = SM_STRICT;
		else if (!strcmp(s, simple_query_string_config->query_false_param))
			output_config->scale_method = SM_FIT;
		else
			debug(ERR, "Unrecognized parameter for strict: %s", s);

		free(s);
	}

	if (lowq) {
		if (!strcmp(lowq, simple_query_string_config->query_true_param))
			output_config->quality = simple_query_string_config->low_quality_value;
		else if (!strcmp(lowq, simple_query_string_config->query_false_param))
			output_config->quality = simple_query_string_config->default_quality_value;
		else
			debug(ERR, "Unrecognized parameter for lowq: %s", lowq);

		free(lowq);
	}

	/* we don't free file_name as it is used in param structure and will be freed on free_output_config */
}

/* it could be probably implemented with scanf sort of functions */
/** Obtains parameter form CGI query string.
* @param query_string CGI query string to parse
* @param param_name parameter name to look fore
* @return allocated string containing value for requested parameter_name or 0 if parameter_name not found in query_string
*/
char *get_query_string_param(char *query_string, char *param_name) {
	char *start_query_string;
	char *until_equal;
	char *until_amp;
	int name_len;
	int val_len;
	char *name;
	char *val;

	debug(DEB, "Getting '%s' from query string '%s'", param_name, query_string);

	until_equal = until_amp = 0;
	val = name = 0;

	start_query_string = query_string;

	while(1) {
		until_equal = index(query_string, '=');
		if (!until_equal)
			return 0;

		name_len = until_equal - query_string;
//		debug(DEB,"Name len: %i we are at index %d", name_len, query_string - start_query_string);

		name = malloc(name_len + 1);
		if (!name)
			exit(66);
		strncpy(name, query_string, name_len);
		name[name_len] = 0;
//		debug(DEB,"Name: '%s'", name);

		if (strcmp(param_name, name)) {
			/* this is not what we are looking for... cleanup */
			free(name);

			/* we are at the end of string */
			if (!(*until_equal))
				return 0;

			/* position to value token*/
			query_string = until_equal + 1;

			/* skip the value */
			until_amp = index(query_string, '&');
			if (!until_amp)
				return 0;

			/* position to next name token*/
			query_string = until_amp + 1;

			/* we are at the end of string */
			if (!(*until_amp))
				return 0;

			continue;
		}

		/* now we have our param... we don't need name */

		query_string = until_equal + 1;

		until_amp = index(query_string, '&');
		if (!until_amp)
			until_amp = query_string + strlen(query_string);

		val_len = until_amp - query_string;
		// debug(DEB,"Name val: %i we are at index %d", val_len, query_string - start_query_string);
		free(name);

		val = malloc(val_len + 1);
		if (!val)
			exit(66);
		strncpy(val, query_string, val_len);
		val[val_len] = 0;
		// debug(DEB,"Value: '%s'", val);

		return val;
	}
}



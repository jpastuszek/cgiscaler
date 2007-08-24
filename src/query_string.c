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
#include "file_utils.h"
#include "debug.h"
#include "config.h"

/* TODO: %xx in query string decoding... no need for filename? */

void apply_query_string_config(struct runtime_config *config, char *file_name, char *query_string) {
	char *w;
	char *h;
	char *s;
	char *lowq;

	/* strange env... failing */
	if (!config || !file_name)
		return;

	debug(DEB, "Processing query file name: '%s'", file_name);

	/* this will allocat the file name */
	file_name = sanitize_file_path(file_name);
	if (file_name) {
		if (config->file_name)
			free(config->file_name);
		config->file_name = file_name;
	}

	debug(DEB, "Processing query string: '%s' target name: '%s'", query_string, file_name);

	w = get_query_string_param(query_string, QUERY_WIDTH_PARAM);
	h = get_query_string_param(query_string, QUERY_HEIGHT_PARAM);
	s = get_query_string_param(query_string, QUERY_STRICT_PARAM);
	lowq = get_query_string_param(query_string, QUERY_LOWQ_PARAM);

	if (w) {
		config->size.w = atoi(w);
		free(w);
	}

	if (h) {
		config->size.h = atoi(h);
		free(h);
	}

	if (s) {
		if (!strcmp(s, QUERY_TRUE_VAL))
			config->strict = 1;
		else
			config->strict = 0;
		free(s);
	}

	if (lowq) {
		if (!strcmp(lowq, QUERY_TRUE_VAL))
			config->quality = LOWQ_QUALITY;
		else
			config->quality = DEFAULT_QUALITY;

		free(lowq);
	}

	debug(DEB, "Run-time conifg after query string: file: '%s', size w: %d h: %d, strict: %d quality: %d", config->file_name ? config->file_name : "<null>", config->size.w, config->size.h, config->strict, config->quality);

	/* we don't free file_name as it is used in param structure and will be freed on free_runtime_config */
}

/* it could be probably implemented with scanf sort of functions */
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



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

char *get_query_string_param(char *qurey_string, char *param_name);

/* TODO: %xx in query string decoding... no need for filename? */

struct query_params *alloc_default_query_params() {
	struct query_params *params;
	params = malloc(sizeof(struct query_params));
	if (!params)
		exit(66);

	params->file_name = 0;
	params->size.w = DEFAULT_WIDTH;
	params->size.h = DEFAULT_HEIGHT;
	params->strict = DEFAULT_STRICT;
	params->lowq = DEFAULT_LOWQ;

	return params;
}

void free_query_params(struct query_params *query_params) {
	free(query_params->file_name);
	free(query_params);
}

struct query_params *get_query_params(char *file_name, char *query_string) {
	char *w;
	char *h;
	char *s;
	char *lowq;

	struct query_params *params;

	/* strange env... failing */
	if (!query_string || !file_name)
		return 0;

	file_name = make_file_name_relative(file_name);

	/* bad file name */
	if (!file_name)
		return 0;
	/* empty file_name... failing */
	if (file_name[0] == 0)
		return 0;

	if (check_for_double_dot(file_name)) {
		debug(WARN, "Double dot found in file name! failing...");
		free(file_name);
		return 0;
	}

	debug(DEB, "Processing query string: '%s' target name: '%s'", query_string, file_name);

	w = get_query_string_param(query_string, WIDTH_PARAM);
	h = get_query_string_param(query_string, HEIGHT_PARAM);
	s = get_query_string_param(query_string, STRICT_PARAM);
	lowq = get_query_string_param(query_string, LOWQ_PARAM);

	/* now we set all qurey_param fields */
	params = alloc_default_query_params();

	params->file_name = file_name;

	if (w) {
		params->size.w = atoi(w);
		free(w);
	}

	if (h) {
		params->size.h = atoi(h);
		free(h);
	}

	if (s) {
		if (!strcmp(s, TRUE_PARAM_VAL))
			params->strict = 1;
		else
			params->strict = 0;
		free(s);
	}

	if (lowq) {
		if (!strcmp(lowq, TRUE_PARAM_VAL))
			params->lowq = 1;
		else
			params->lowq = 0;
		free(lowq);
	}

	debug(DEB, "Params: file: '%s', size w: %d h: %d, strict: %d lowq: %d", params->file_name, params->size.w, params->size.h, params->strict, params->lowq);

	/* we don't free file_name as it is used in param structure and will be freed on free_query_params */
	return params;
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
		if (name)
			free(name);
		if (val)
			free(val);

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

		query_string += name_len + 1;


		until_amp = index(query_string, '&');
		if (!until_amp)
			until_amp = query_string + strlen(query_string);

		val_len = until_amp - query_string;
//		debug(DEB,"Name val: %i we are at index %d", val_len, query_string - start_query_string);

		val = malloc(val_len + 1);
		if (!val)
			exit(66);
		strncpy(val, query_string, val_len);
		val[val_len] = 0;
//		debug(DEB,"Value: '%s'", val);

		query_string += val_len + 1;

		if (!strcmp(param_name, name)) {
			free(name);
			return val;
		}
	}
}



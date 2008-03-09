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
#include <stdlib.h>

#include "commandline.h"
#include "runtime_config.h"
#include "file_utils.h"
#include "config.h"
#include "debug.h"

extern struct output_config *output_config;
extern struct operation_config *operation_config;

#ifdef DEBUG
extern char **scale_method_names;
#endif

/** Apply configuration specified as command line parameters.
* @param argc argc parameter from main function
* @param argv argv parameter from main function
*/
void apply_commandline_config(int argc, char *argv[]) {
	int i;
	char *file_name = 0;

	if (argc <= 1)
		return;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			/* Runtime config parameters */
			if (!strcmp(argv[i] + 1, COMMANDLINE_WIDTH_SWITCH)) {
				if (argc - i <= 1) {
					debug(ERR, "Expected integer for width parameter");
					return;
				}

				output_config->size.w = atoi(argv[i+1]);

				i++;
				continue;
			}

			if (!strcmp(argv[i] + 1, COMMANDLINE_HEIGHT_SWITCH)) {
				if (argc - i <= 1) {
					debug(ERR, "Expected integer for heigth parameter");
					return;
				}

				output_config->size.h = atoi(argv[i+1]);

				i++;
				continue;
			}

			if (!strcmp(argv[i] + 1, COMMANDLINE_STRICT_SWITCH)) {
				if (argc - i <= 1) {
					debug(ERR, "Expected boolean parameter for strict");
					return;
				}

				if (!strcmp(argv[i+1], COMMANDLINE_TRUE_VAL))
					output_config->scale_method = SM_STRICT;
				else if (!strcmp(argv[i+1], COMMANDLINE_FALSE_VAL))
					output_config->scale_method = SM_FIT;
				else
					debug(ERR, "Unrecognized parameter for strict: %s", argv[i+1]);

				i++;
				continue;
			}

			if (!strcmp(argv[i] + 1, COMMANDLINE_LOWQ_SWITCH)) {
				if (argc - i <= 1) {
					debug(ERR, "Expected boolean parameter for lowq");
					return;
				}

				if (!strcmp(argv[i+1], COMMANDLINE_TRUE_VAL))
					output_config->quality = simple_query_string_config->low_quality_value;
				else if (!strcmp(argv[i+1], COMMANDLINE_FALSE_VAL))
					output_config->quality = simple_query_string_config->default_quality_value;
				else
					debug(ERR, "Unrecognized parameter for lowq: %s", argv[i+1]);

				i++;
				continue;
			}

			/* Operation config parameters */
			if (!strcmp(argv[i] + 1, COMMANDLINE_NO_CACHE_SWITCH)) {
				if (argc - i <= 1) {
					debug(ERR, "Expected boolean parameter for no_cache");
					return;
				}

				if (!strcmp(argv[i+1], COMMANDLINE_TRUE_VAL))
					operation_config->no_cache = 1;
				else if (!strcmp(argv[i+1], COMMANDLINE_FALSE_VAL))
					operation_config->no_cache = 0;
				else
					debug(ERR, "Unrecognized parameter for no_cache: %s", argv[i+1]);

				i++;
				continue;
			}

			if (!strcmp(argv[i] + 1, COMMANDLINE_NO_SERVE_SWITCH)) {
				if (argc - i <= 1) {
					debug(ERR, "Expected boolean parameter for no_serve");
					return;
				}

				if (!strcmp(argv[i+1], COMMANDLINE_TRUE_VAL))
					operation_config->no_serve = 1;
				else if (!strcmp(argv[i+1], COMMANDLINE_FALSE_VAL))
					operation_config->no_serve = 0;
				else
					debug(ERR, "Unrecognized parameter for no_serve: %s", argv[i+1]);

				i++;
				continue;
			}

			if (!strcmp(argv[i] + 1, COMMANDLINE_NO_HEADERS_SWITCH)) {
				if (argc - i <= 1) {
					debug(ERR, "Expected boolean parameter for no_headers");
					return;
				}

				if (!strcmp(argv[i+1], COMMANDLINE_TRUE_VAL))
					operation_config->no_headers = 1;
				else if (!strcmp(argv[i+1], COMMANDLINE_FALSE_VAL))
					operation_config->no_headers = 0;
				else
					debug(ERR, "Unrecognized parameter for no_headers: %s", argv[i+1]);

				i++;
				continue;
			}

			debug(ERR, "Unknown parameter %s", argv[i]);
			return;
		}

		if (file_name) {
			debug(ERR, "Non-switch parameter where file name already defined");
			return;
		}

		/* sanitize_file_path will allocate */
		file_name = sanitize_file_path(argv[i]);
		if (file_name) {
			if (output_config->file_name)
				free(output_config->file_name);
			output_config->file_name = file_name;
		}
	}

#ifdef DEBUG
	debug(DEB, "Run-time config after command line: file: '%s', size w: %d h: %d, scale method: %s quality: %d Operation coifig: no cache: %d, no serve: %d, no headers: %d", output_config->file_name ? output_config->file_name : "<null>", output_config->size.w, output_config->size.h, scale_method_names[output_config->scale_method], output_config->quality, operation_config->no_cache, operation_config->no_serve, operation_config->no_headers);
#endif
}

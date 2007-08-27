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
#include "file_utils.h"
#include "config.h"
#include "debug.h"

void apply_commandline_config(struct runtime_config *config, int argc, char *argv[]) {
	int i;
	char *file_name = 0;

	if (argc <= 1)
		return;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (!strcmp(argv[i] + 1, COMMANDLINE_WIDTH_SWITCH)) {
				if (argc - i <= 1) {
					debug(ERR, "Expected integer for width parameter");
					return;
				}

				config->size.w = atoi(argv[i+1]);

				i++;
				continue;
			}

			if (!strcmp(argv[i] + 1, COMMANDLINE_HEIGHT_SWITCH)) {
				if (argc - i <= 1) {
					debug(ERR, "Expected integer for heigth parameter");
					return;
				}

				config->size.h = atoi(argv[i+1]);

				i++;
				continue;
			}

			if (!strcmp(argv[i] + 1, COMMANDLINE_STRICT_SWITCH)) {
				if (argc - i <= 1) {
					debug(ERR, "Expected boolean parameter for strict");
					return;
				}

				if (!strcmp(argv[i+1], COMMANDLINE_TRUE_VAL))
					config->strict = 1;
				else if (!strcmp(argv[i+1], COMMANDLINE_FALSE_VAL))
					config->strict = 0;
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
					config->quality = LOWQ_QUALITY;
				else if (!strcmp(argv[i+1], COMMANDLINE_FALSE_VAL))
					config->quality = DEFAULT_QUALITY;
				else
					debug(ERR, "Unrecognized parameter for lowq: %s", argv[i+1]);

				i++;
				continue;
			}

			if (!strcmp(argv[i] + 1, COMMANDLINE_NO_CACHE_SWITCH)) {
				if (argc - i <= 1) {
					debug(ERR, "Expected boolean parameter for no_cache");
					return;
				}

				if (!strcmp(argv[i+1], COMMANDLINE_TRUE_VAL))
					config->no_cache = 1;
				else if (!strcmp(argv[i+1], COMMANDLINE_FALSE_VAL))
					config->no_cache = 0;
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
					config->no_serve = 1;
				else if (!strcmp(argv[i+1], COMMANDLINE_FALSE_VAL))
					config->no_serve = 0;
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
					config->no_headers = 1;
				else if (!strcmp(argv[i+1], COMMANDLINE_FALSE_VAL))
					config->no_headers = 0;
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
			if (config->file_name)
				free(config->file_name);
			config->file_name = file_name;
		}
	}

	debug(DEB, "Run-time config after command line: file: file: '%s', size w: %d h: %d, strict: %d quality: %d no cache: %d, no serve: %d, no headers: %d", config->file_name ? config->file_name : "<null>", config->size.w, config->size.h, config->strict, config->quality, config->no_cache, config->no_serve, config->no_headers);
}

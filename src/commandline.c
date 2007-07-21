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
	char *file_name;

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
		}

		if (config->file_name) {
			debug(ERR, "Non-switch parameter where file name already defined");
			return;
		}

		file_name = sanitize_file_path(argv[i]);
		if (file_name)
			config->file_name = file_name;
	}

	debug(DEB, "Run-time config after command line: file: file: '%s', size w: %d h: %d, strict: %d quality: %d", config->file_name, config->size.w, config->size.h, config->strict, config->quality);
}

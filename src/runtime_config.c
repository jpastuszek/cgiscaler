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
#include "runtime_config.h"
#include "config.h"

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
	config->no_cache = DEFAULT_NO_CACHE;
	config->no_serve = DEFAULT_NO_SERVE;
	config->no_headers = DEFAULT_NO_HEADERS;

	return config;
}

void free_runtime_config(struct runtime_config *config) {
	if (config->file_name)
		free(config->file_name);
	free(config);
}

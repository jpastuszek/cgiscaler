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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "query_string.h"

void do_on_exit(void);
void serve_error_image_and_exit();

int main(int argc, char *argv[])
{
	struct query_params *params;

	debug_start("/tmp/cgiscaler.deb");
	atexit(do_on_exit);
	
	
	params = get_query_params();
	if (!params)
		serve_error_image_and_exit();
	
	return EXIT_SUCCESS;
}

/* we will stop debug when normal exit */
void do_on_exit(void) {
	debug_stop();
}

void serve_error_image_and_exit() {
	printf("Content-type: text/html\n");
	printf("\n");

	printf("Error: Error image not implemented\n");
	exit(7);
}

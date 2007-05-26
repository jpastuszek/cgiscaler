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
#include <string.h>

#include <wand/MagickWand.h>

#include "debug.h"
#include "query_string.h"
#include "config.h"


#define ThrowWandException(wand) \
{ \
char \
*description; \
\
ExceptionType \
severity; \
\
description=MagickGetException(wand,&severity); \
(void) fprintf(stderr,"%s %s %ld %s\n",GetMagickModule(),description); \
description=(char *) MagickRelinquishMemory(description); \
exit(-1); \
}

void do_on_exit(void);
void serve_error_image_and_exit();
void serve_image(MagickWand *magick_wand);
MagickWand *load_image(char *file_name);


int main(int argc, char *argv[])
{
	struct query_params *params;
	MagickWand *magick_wand;

	debug_start("/tmp/cgiscaler.deb");
	atexit(do_on_exit);
	
	MagickWandGenesis();

	params = get_query_params();
	if (!params)
		serve_error_image_and_exit();
	
	magick_wand = load_image(params->file_name);
	if (!magick_wand)
		serve_error_image_and_exit();
//	if (params->strict)
//		image = crop_and_resize(image, params->size);

	serve_image(magick_wand);

	DestroyMagickWand(magick_wand);

	free_query_params(params);
	return EXIT_SUCCESS;
}

/* we will stop debug when normal exit */
void do_on_exit(void) {
	MagickWandTerminus();
	debug_stop();
}

void serve_error_image_and_exit() {
	printf("Content-type: text/html\n");
	printf("\n");

	printf("Error: Error image not implemented\n");
	exit(7);
}

void serve_image(MagickWand *magick_wand) {

}


MagickWand *load_image(char *file_name) {
	char *path;
	Image *image;
	MagickWand *magick_wand;
	MagickBooleanType status;

	path = malloc(strlen(MEDIA_PATH) + strlen(file_name) + 1);
	strcpy(path, MEDIA_PATH);
	strcat(path, file_name);

	debug(DEB,"Loading image: '%s'", path);

	magick_wand = NewMagickWand();
	if (!magick_wand)
		return 0;

	status = MagickReadImage(magick_wand, path);
	if (status == MagickFalse) {
		DestroyMagickWand(magick_wand);
		return 0;
	}

	return magick_wand;
}


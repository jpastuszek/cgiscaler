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
#include <errno.h>

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
void serve_image(MagickWand *magick_wand, struct query_params *params);
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


	serve_image(magick_wand, params);

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

void serve_image(MagickWand *magick_wand, struct query_params *params) {
	unsigned char *blob;
	size_t blob_len;
	size_t total_blob_written;
	char *content_type;
	char *content_length;
	char content_length_val[10];
	size_t bytes_written;

	debug(DEB,"Serving Image");

	blob = MagickGetImageBlob(magick_wand, &blob_len);
	if (!blob)
		exit(1);

	MagickSetFormat(magick_wand, OUT_FORMAT);
	if (params->lowq)
		MagickSetCompressionQuality(magick_wand, LOWQ_QUALITY);
	else
		MagickSetCompressionQuality(magick_wand, NORMAL_QUALITY);


	/* + 2 stands for \n + \0 */
	content_type = malloc(strlen("Content-type: ") + strlen(OUT_FORMAT_MIME_TYPE) + 2);
	strcpy(content_type, "Content-type: ");
	strcat(content_type, OUT_FORMAT_MIME_TYPE);
	strcat(content_type, "\n");
	printf(content_type);
	free(content_type);

	snprintf(content_length_val, 10, "%d", blob_len);
	content_length = malloc(strlen("Content-Length: ") + strlen(content_length_val) + 2);
	strcpy(content_length, "Content-Length: ");
	strcat(content_length, content_length_val);
	strcat(content_length, "\n");
	//printf(content_length);
	free(content_length);	

	printf("\n");
	fsync(stdout);

	/* I don't beleve that this is optimal... but it works */
	fwrite(blob, blob_len, 1, stdout);

/* freeking mistery why write makes headers fucked... heh */
/*	total_blob_written = 0;
	while(1) {
		debug(DEB, "Writing %d bytes to stdout", blob_len - total_blob_written);
		bytes_written = write(1, blob + total_blob_written, blob_len - total_blob_written);
		debug(DEB, "%d bytes written", bytes_written);
		if (bytes_written == -1) {
			debug(ERR, "Error writting to stdout %s", strerror(errno));
			exit(1);
		}
		
		total_blob_written += bytes_written;

		if (total_blob_written >= blob_len)
			break;
		fsync(stdout);
	}
*/
	MagickRelinquishMemory(blob);
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


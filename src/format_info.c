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

#include "format_info.h"
#include <wand/MagickWand.h>
#include <string.h>
#include "debug.h"

/** Build in format, mime-type and file extension mapping */
static struct format_info builtin_format_info[] = { 
	{ "JPG", "image/jpeg", "jpg" },
	{ "JPEG", "image/jpeg", "jpg" },
	{ "GIF", "image/gif", "gif" },
	{ "PNG", "image/png", "png" },
	{ 0, 0, 0},
};

/** Populates format_info structure from format string.
* @param format ImageMagick format string
* @return Allocated format_info structure or 0 if error occurred
* @see free_format_info()
*/
struct format_info *format_to_format_info(char *format) {
	struct format_info *fi;

	fi = get_format_info_from_builtin(format);
	if (fi)
		return fi;

	return get_format_info_from_magick(format);
}

/** Frees allocated format_info structure. */
void free_format_info(struct format_info *fi) {
	if (!fi)
		return;

	free(fi->format);
	free(fi->mime_type);
	free(fi->file_ext);
	free(fi);
}

/** Populates format_info structure from format string using built-in data.
* @param format ImageMagick format string
* @return Allocated format_info structure or 0 if error occurred
* @see free_format_info()
*/
struct format_info *get_format_info_from_builtin(char *format) {
	struct format_info *fi;
	int i;
	for (i = 0; builtin_format_info[i].format; i++) {
		if (!strcmp(builtin_format_info[i].format, format)) {
			fi = malloc(sizeof(struct format_info));
			fi->format = strdup(builtin_format_info[i].format);
			fi->mime_type = strdup(builtin_format_info[i].mime_type);
			fi->file_ext = strdup(builtin_format_info[i].file_ext);
			return fi;
		}
	}
	return 0;
}


/** Populates format_info structure from format string using Magick.
* @param format ImageMagick format string
* @return Allocated format_info structure or 0 if error occurred
* @see free_format_info()
*/
struct format_info *get_format_info_from_magick(char *format) {
	struct format_info *fi;
	short int i;
	short int format_len;
	char *mime_type;

	mime_type = MagickToMime(format);
	if (!mime_type) {
		debug(ERR, "Failed to obtain mime-type for format string from Magick: %s", format);
		return 0;
	}

	fi = malloc(sizeof(struct format_info));

	fi->format = strdup(format);
	fi->mime_type = mime_type;

	format_len = strlen(format);
	fi->file_ext = malloc(format_len + 1);
	for (i = 0; i < format_len; i++)
		fi->file_ext[i] = tolower(format[i]);
	fi->file_ext[format_len] = 0;

	return fi;
}


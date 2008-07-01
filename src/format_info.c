/***************************************************************************
 *   Copyright (C) 2007, 2008, 2008 by Jakub Pastuszek   *
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
#include <ctype.h>
#include <stdlib.h>
#include "debug.h"

/** Build in format, mime-type and file extension mapping */
static struct format_info builtin_format_info[] = { 
	{ "JPG", "image/jpeg", "jpg" },
	{ "JPEG", "image/jpeg", "jpeg" },
	{ "GIF", "image/gif", "gif" },
	{ "PNG", "image/png", "png" },
	{ 0, 0, 0},
};

/** Populates format_info structure from file extension.
* @param file_ext file extension
* @return Allocated format_info structure or 0 if error occurred
* @see free_format_info()
*/
struct format_info *file_extension_to_format_info(char *file_ext) {
	struct format_info *fi;
	fi = file_extension_to_format_info_from_builtin(file_ext);
	if (fi)
		return fi;

	return file_extension_to_format_info_from_magick(file_ext);
}

/** Populates format_info structure from file extension using built-in data.
* @param file_ext file extension
* @return Allocated format_info structure or 0 if error occurred
* @see free_format_info()
*/
struct format_info *file_extension_to_format_info_from_builtin(char *file_ext) {
	struct format_info *fi;
	int i;
	for (i = 0; builtin_format_info[i].format; i++) {
		if (!strcmp(builtin_format_info[i].file_ext, file_ext)) {
			fi = malloc(sizeof(struct format_info));
			fi->format = strdup(builtin_format_info[i].format);
			fi->mime_type = strdup(builtin_format_info[i].mime_type);
			fi->file_ext = strdup(builtin_format_info[i].file_ext);
			return fi;
		}
	}
	return 0;
}

/** Populates format_info structure from file extension using Magick.
* @param file_ext file extension
* @return Allocated format_info structure or 0 if error occurred
* @see free_format_info()
*/
struct format_info *file_extension_to_format_info_from_magick(char *file_ext) {
	struct format_info *fi;
	short int i;
	char *mime_type;
	char *format;
	short int file_ext_len;

	file_ext_len = strlen(file_ext);
	format = malloc(file_ext_len + 1);

	// we simply up case the extension and use it as format to ask Magick
	for (i = 0; i < file_ext_len; i++)
		format[i] = toupper(file_ext[i]);
	format[file_ext_len] = 0;

	mime_type = MagickToMime(format);
	if (!mime_type) {
		debug(ERR, "Failed to obtain mime-type for format string from Magick: %s", format);
		return 0;
	}

	fi = malloc(sizeof(struct format_info));

	fi->format = format;
	fi->mime_type = mime_type;
	fi->file_ext = strdup(file_ext);

	return fi;
}

/** Populates format_info structure from format string.
* @param format ImageMagick format string
* @return Allocated format_info structure or 0 if error occurred
* @see free_format_info()
*/
struct format_info *format_to_format_info(char *format) {
	struct format_info *fi;

	fi = format_to_format_info_from_builtin(format);
	if (fi)
		return fi;

	return format_to_format_info_from_magick(format);
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
struct format_info *format_to_format_info_from_builtin(char *format) {
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
struct format_info *format_to_format_info_from_magick(char *format) {
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


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

#ifndef FORMAT_INFO_H
#define FORMAT_INFO_H

/** Image format information structure. */
struct format_info {
	char *format;
	char *mime_type;
	char *file_ext;
};

#endif

struct format_info *file_extension_to_format_info(char *file_ext);
struct format_info *format_to_format_info(char *format);
void free_format_info(struct format_info *fi);

/* for testing */
struct format_info *file_extension_to_format_info_from_builtin(char *file_ext);
struct format_info *file_extension_to_format_info_from_magick(char *file_ext);

struct format_info *format_to_format_info_from_builtin(char *format);
struct format_info *format_to_format_info_from_magick(char *format);


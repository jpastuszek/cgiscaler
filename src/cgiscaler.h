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

#include <wand/MagickWand.h>
#include <stdlib.h>

#include "query_string.h"

MagickWand *load_image(char *file_name);
unsigned char *prepare_blob(MagickWand *magick_wand, struct query_params *params, size_t *blob_len);
void free_blob(unsigned char *blob);

MagickWand *resize(MagickWand *magick_wand, struct dimmensions to_size);
MagickWand *crop_and_resize(MagickWand *magick_wand, struct dimmensions size);

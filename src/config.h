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

/* enable debuging - not suitable for production env! */
#define DEBUG
/* sync every line written to debug file... slow */
//#define DEBUG_SYNC

/* debug file path if debugging enabled */
#define DEBUG_FILE "/tmp/cgiscaler.gallery.deb"

/* URL Query parameter names */
#define WIDTH_PARAM "w"
#define HEIGHT_PARAM "h"
#define STRICT_PARAM "s"
#define LOWQ_PARAM "wap"

/* URL Query true value: what string will match as true */
#define TRUE_PARAM_VAL "true"

/* Path to media source directory (note tailing / char)*/
#define MEDIA_PATH "/home/kazuya/Projects/Gallery/public/system/photos/"
/* Path were to write cache files (note tailing / char) */
#define CACHE_PATH "/home/kazuya/Projects/Gallery/public/system/cache/"

/* output format (ex. JPEG, GIF, PNG) */
#define OUT_FORMAT "JPEG"
/* Mime-type to match output format - will be send in HTTP headers */
#define OUT_FORMAT_MIME_TYPE "image/jpeg"

/* Path to error file (under MEDIA_PATH) that will be served in case of error */
#define ERROR_FILE_PATH "error.jpg"
/* Error file mime type */
#define ERROR_FILE_MIME_TYPE "image/jpeg"
/* Message that will be sent as plain text in case of error and no ERROR_FILE_PATH exists */
#define ERROR_FAILBACK_MESSAGE "[Error and error image not found]\n"

/* When LOWQ_PARAM=TRUE_PARAM_VAL output data will use this compression parameter (0-100 best) */
#define LOWQ_QUALITY 20
/* Use this quality when creating thumbnails */
#define NORMAL_QUALITY 80

/* Possible re-size filters
    BesselFilter   BlackmanFilter   BoxFilter
    CatromFilter   CubicGaussianFilter
    HanningFilter  HermiteFilter    LanczosFilter
    MitchellFilter PointQuandraticFilter
    SincFilter     TriangleFilter
*/
#define RESIZE_FILTER LanczosFilter
/* "blur factor where > 1 is blurry, < 1 is sharp" */
#define RESIZE_SMOOTH_FACTOR 1

/* if output format doesn't handle transparency (like JPG) uncomment REMOVE_TRANSPARENCY so when transparent image is scaled it's transparency will be replaced by DEFAULT_BACKGROUND_COLOR */
#define REMOVE_TRANSPARENCY
/* color to fill transparency when converting from transparent gifs/pngs... */
#define DEFAULT_BACKGROUND_COLOR "#ffffff"

/* Maximum number of pixels that output image will be limited to */
#define MAX_PIXEL_NO 200000

/* Disk/cache file serving buffer */
#define WRITE_BUFFER_LEN 8192

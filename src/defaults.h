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


/** debug file path if debugging enabled */
#define DEBUG_FILE "/tmp/cgiscaler.deb"


/** Default Run-time configuration (when applicable 0 means false, 1 true) */
#define DEFAULT_WIDTH 64
#define DEFAULT_HEIGHT 64
#define DEFAULT_SCALE_METHOD SM_FIT
#define DEFAULT_QUALITY 80
#define DEFAULT_NO_CACHE 0
#define DEFAULT_NO_SERVE 0
#define DEFAULT_NO_HEADERS 0

/* CGI Query parameter names */
/** String to match width parameter in CGI query string */
#define QUERY_WIDTH_PARAM "w"
/** String to match height parameter in CGI query string */
#define QUERY_HEIGHT_PARAM "h"
/** String to match strict enable parameter in CGI query string */
#define QUERY_STRICT_PARAM "s"
/** String to mach low quality enable parameter in CGI query string */
#define QUERY_LOWQ_PARAM "wap"

/* CGI Query true/false values: what string will match as true or false*/
/** String to mach true value in CGI query string */
#define QUERY_TRUE_VAL "true"
/** String to mach false value in CGI query string */
#define QUERY_FALSE_VAL "false"

/** Quality to use when lowq is enabled.
*When LOWQ_PARAM/SWITCH=TRUE_VAL output data will use this compression parameter (0-100 best) or else DEFAULT_QUALITY will be used
 */
#define LOWQ_QUALITY 20


/** Path to media source directory */
#define MEDIA_PATH "/photos"
/** Path were to write cache files */
#define CACHE_PATH "/cache"

/** output format (ex. JPG, GIF, PNG) */
#define OUT_FORMAT "JPG"
//#define OUT_FORMAT_EXTENSION "jpg"

/** Path to error file (under MEDIA_PATH) that will be served in case of error */
#define ERROR_FILE_PATH "error.jpg"
/** Message that will be sent as plain text in case of error and no ERROR_FILE_PATH exists */
#define ERROR_FAILBACK_MESSAGE "[Error and error image not found]\n"

/** Mime-type that will be send in HTTP headers if one corresponding to format/extension could not be determined */
#define FAIL_BACK_MIME_TYPE "image/jpeg"

/** Possible re-size filters
*    BesselFilter   BlackmanFilter   BoxFilter
*    CatromFilter   CubicGaussianFilter
*    HanningFilter  HermiteFilter    LanczosFilter
*    MitchellFilter PointQuandraticFilter
*    SincFilter     TriangleFilter
*/
#define RESIZE_FILTER LanczosFilter
/** "Blur factor where > 1 is blurry, < 1 is sharp" */
#define RESIZE_SMOOTH_FACTOR 1

/** If output format doesn't handle transparency (like JPG) uncomment REMOVE_TRANSPARENCY so when transparent image is scaled it's transparency will be replaced by DEFAULT_BACKGROUND_COLOR */
#define REMOVE_TRANSPARENCY
/** Color to fill transparency when converting from transparent gifs/pngs... */
#define DEFAULT_BACKGROUND_COLOR "#ffffff"

/** Maximum number of pixels that output image will be limited to */
#define MAX_PIXEL_NO 200000

// BIG FAT WARNING: units of this valuse are library version dependant!!! test in debug first!
/** Maximum number of open pixel cache files  */
#define RESOURCE_LIMIT_FILE 8
/** Maximum amount of disk space permitted for use by the pixel cache in GB */
#define RESOURCE_LIMIT_DISK 2*1024*1024
/** Maximum amount of memory map to allocate for the pixel cache in MB - when this limit is exceeded, the image pixels are cached to disk */
#define RESOURCE_LIMIT_MAP 0
/** Maximum amount of memory to allocate for the pixel cache from the heap in MB - when this limit is exceeded, the image pixels are cached to memory-mapped disk */
#define RESOURCE_LIMIT_MEMORY 256*1024*1024
/** Maximum amount of memory to allocate for image from in MB - images that exceed the area limit are cached to disk  */
#define RESOURCE_LIMIT_AREA 256*1024*1024


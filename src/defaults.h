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
#define LOG_FILE "/tmp/cgiscaler.deb"


/** Default Run-time configuration (when applicable 0 means false, 1 true) */
#define WIDTH 64
#define HEIGHT 64
#define SCALE_METHOD SM_FIT
#define NORMAL_QUALITY_VALUE 80
#define NO_CACHE 0
#define NO_SERVE 0
#define NO_HEADERS 0

/* CGI Query parameter names */
/** String to match width parameter in CGI query string */
#define CGI_WIDTH "w"
/** String to match height parameter in CGI query string */
#define CGI_HEIGHT "h"
/** String to match strict enable parameter in CGI query string */
#define CGI_STRICT "s"
/** String to mach low quality enable parameter in CGI query string */
#define CGI_LOW_QUALITY "wap"

/* CGI Query true/false values: what string will match as true or false*/
/** String to mach true value in CGI query string */
#define CGI_TRUE "true"
/** String to mach false value in CGI query string */
#define CGI_FALSE "false"

/** Quality to use when lowq is enabled.
*When LOWQ_PARAM/SWITCH=TRUE_VAL output data will use this compression parameter (0-100 best) or else NORMAL_QUALITY_VALUE will be used
 */
#define LOW_QUALITY_VALUE 20


/** Path to media source directory */
#define MEDIA_DIR "/photos"
/** Path were to write cache files */
#define CACHE_DIR "/cache"

/** output format (ex. JPG, GIF, PNG) */
#define OUT_FORMAT "JPG"

/** Path to error file (under MEDIA_DIR) that will be served in case of error */
#define ERROR_FILE "error.jpg"
/** Message that will be sent as plain text in case of error and no ERROR_FILE exists */
#define ERROR_MESSAGE "[Error and error image not found]\n"

/** Mime-type that will be send in HTTP headers if one corresponding to format/extension could not be determined */
#define FAIL_BACK_MIME_TYPE "image/jpeg"

/** Possible re-size filters
*    BesselFilter   BlackmanFilter   BoxFilter
*    CatromFilter   CubicGaussianFilter
*    HanningFilter  HermiteFilter    LanczosFilter
*    MitchellFilter PointQuandraticFilter
*    SincFilter     TriangleFilter
*/
#define SCALING_FILTER LanczosFilter
/** "Blur factor where > 1 is blurry, < 1 is sharp" */
#define BLUR_FACTOR 1

/** If output format doesn't handle transparency (like JPG) uncomment REMOVE_TRANSPARENCY so when transparent image is scaled it's transparency will be replaced by TRANSPARENCY_COLOUR */
#define REMOVE_TRANSPARENCY
/** Color to fill transparency when converting from transparent gifs/pngs... */
#define TRANSPARENCY_COLOUR "#ffffff"

/** Maximum number of pixels that output image will be limited to */
#define MAX_OUT_PIXELS 200000

// BIG FAT WARNING: units of this valuse are library version dependant!!! test in debug first!
/** Maximum number of open pixel cache files  */
#define LIMIT_FILES 8
/** Maximum amount of disk space permitted for use by the pixel cache in bytes */
#define LIMIT_DISK 2*1024*1024
/** Maximum amount of memory map to allocate for the pixel cache in bytes - when this limit is exceeded, the image pixels are cached to disk */
#define LIMIT_MAP 0
/** Maximum amount of memory to allocate for the pixel cache from the heap in bytes - when this limit is exceeded, the image pixels are cached to memory-mapped disk */
#define LIMIT_MEMORY 256*1024*1024
/** Maximum amount of memory to allocate for image from in bytes - images that exceed the area limit are cached to disk  */
#define LIMIT_AREA 256*1024*1024


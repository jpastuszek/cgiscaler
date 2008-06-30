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


#define LOG_FILE "/tmp/cgiscaler_test.deb"


/* Default Run-time configuration (0 means false, 1 true) */
#define WIDTH 64
#define HEIGHT 64
#define DEFAULT_STRICT 0
#define NORMAL_QUALITY_VALUE 80
#define NO_CACHE 0
#define NO_SERVE 0
#define NO_HEADERS 0

#define SCALE_METHOD SM_FIT

/* CGI Query configuration */
/* CGI Query parameter names */
#define CGI_WIDTH "w"
#define CGI_HEIGHT "h"
#define CGI_STRICT "s"
#define CGI_LOW_QUALITY "wap"

/* CGI Query true value: what string will match as true */
#define CGI_TRUE "true"
#define CGI_FALSE "false"

/* When LOWQ_PARAM/SWITCH=TRUE_VAL output data will use this compression parameter (0-100 best) or else NORMAL_QUALITY_VALUE will be used */
#define LOW_QUALITY_VALUE 20

/* Path to media source directory */
#define MEDIA_DIR "test"
/* Path were to write cache files */
#define CACHE_DIR "/tmp"

/* output format (ex. JPG, GIF, PNG) */
#define OUT_FORMAT "JPG"
#define OUT_FORMAT_EXTENSION "jpg"
/* Mime-type to match output format - will be send in HTTP headers */

#define ERROR_FILE "test_dir/a/quick_gimp_pattern_test.gif"
#define ERROR_MESSAGE "[Error and error image not found]\n"

/** Mime-type that will be send in HTTP headers if corresponding to format/extension could not be found */
#define FAIL_BACK_MIME_TYPE "image/jpeg"

/* Possible resize filters 
    BesselFilter   BlackmanFilter   BoxFilter
    CatromFilter   CubicGaussianFilter
    HanningFilter  HermiteFilter    LanczosFilter
    MitchellFilter PointQuandraticFilter
    SincFilter     TriangleFilter
*/
#define SCALING_FILTER LanczosFilter
/* "blur factor where > 1 is blurry, < 1 is sharp" */
#define BLUR_FACTOR 1

/* if output format doesn't handle transparency (like JPG) uncomment REMOVE_TRANSPARENCY so when transparent image is scaled it's transpaency will be replaced by TRANSPARENCY_COLOUR */
#define REMOVE_TRANSPARENCY

/* color to fill transparency when conferting from transparent gifs - yellowishs*/
/* #112233 hex -> rgb17,34,51 */
#define TRANSPARENCY_COLOUR "#112233"

#define MAX_OUT_PIXELS ((450 - 10) * (300 - 10) - 4000)


// BIG FAT WARNING: units of this valuse are library version dependant!!! test in debug first!
/* maximum number of open pixel cache files  */
#define LIMIT_FILES 8
/* maximum amount of disk space in bytes permitted for use by the pixel cache  in GB */
#define LIMIT_DISK 2*1024*1024
/* maximum amount of memory map to allocate for the pixel cache in MB - when this limit is exceeded, the image pixels are cached to disk */
#define LIMIT_MAP 0
/* maximum amount of memory to allocate for the pixel cache from the heap in MB - when this limit is exceeded, the image pixels are cached to memory-mapped disk */
#define LIMIT_MEMORY 256*1024*1024
/* maximum amount of memory to allocate for image from in MB - images that exceed the area limit are cached to disk  */
#define LIMIT_AREA 256*1024*1024

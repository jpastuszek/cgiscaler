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
/* This is test config file */

#define DEBUG_FILE "/tmp/cgiscaler_test.deb"
//#define DEBUG_SYNC


/* Default Run-time configuration (0 means false, 1 true) */
#define DEFAULT_WIDTH 64
#define DEFAULT_HEIGHT 64
#define DEFAULT_STRICT 0
#define DEFAULT_QUALITY 80
#define DEFAULT_NO_CACHE 0
#define DEFAULT_NO_SERVE 0
#define DEFAULT_NO_HEADERS 0


/* Command-line configuration */
#define COMMANDLINE_WIDTH_SWITCH "w"
#define COMMANDLINE_HEIGHT_SWITCH "h"
#define COMMANDLINE_STRICT_SWITCH "s"
#define COMMANDLINE_LOWQ_SWITCH "wap"
#define COMMANDLINE_NO_CACHE_SWITCH "nc"
#define COMMANDLINE_NO_SERVE_SWITCH "ns"
#define COMMANDLINE_NO_HEADERS_SWITCH "nh"

#define COMMANDLINE_TRUE_VAL "true"

/* CGI Query configuration */
/* CGI Query parameter names */
#define QUERY_WIDTH_PARAM "w"
#define QUERY_HEIGHT_PARAM "h"
#define QUERY_STRICT_PARAM "s"
#define QUERY_LOWQ_PARAM "wap"

/* CGI Query true value: what string will match as true */
#define QUERY_TRUE_VAL "true"

/* When LOWQ_PARAM/SWITCH=TRUE_VAL output data will use this compression parameter (0-100 best) or else DEFAULT_QUALITY will be used */
#define LOWQ_QUALITY 20

/* Path to media source directory (note tailing / char)*/
#define MEDIA_PATH "tests/"
/* Path were to write cache files (note tailing / char) */
#define CACHE_PATH "/tmp/"

/* output format (ex. JPEG, GIF, PNG) */
#define OUT_FORMAT "JPEG"
#define OUT_FORMAT_EXTENSION "jpg"
/* Mime-type to match output format - will be send in HTTP headers */
#define OUT_FORMAT_MIME_TYPE "image/jpeg"

#define ERROR_FILE_PATH "quick_gimp_pattern_test.gif"
#define ERROR_FILE_MIME_TYPE "image/jpeg"
#define ERROR_FAILBACK_MESSAGE "[Error and error image not found]\n"

#define IMAGE_TEST_FILE "quick_gimp_pattern_test.png"
#define IMAGE_TEST_FILE_WIDTH 450
#define IMAGE_TEST_FILE_HEIGHT 300

/* Possible resize filters 
    BesselFilter   BlackmanFilter   BoxFilter
    CatromFilter   CubicGaussianFilter
    HanningFilter  HermiteFilter    LanczosFilter
    MitchellFilter PointQuandraticFilter
    SincFilter     TriangleFilter
*/
#define RESIZE_FILTER LanczosFilter
/* "blur factor where > 1 is blurry, < 1 is sharp" */
#define RESIZE_SMOOTH_FACTOR 1

/* if output format doesn't handle transparency (like JPG) uncomment REMOVE_TRANSPARENCY so when transparent image is scaled it's transpaency will be replaced by DEFAULT_BACKGROUND_COLOR */
#define REMOVE_TRANSPARENCY

/* color to fill transparency when conferting from transparent gifs - yellowishs*/
/* #112233 hex -> rgb17,34,51 */
#define DEFAULT_BACKGROUND_COLOR "#112233"
#define DEFAULT_BACKGROUND_COLOR_MAGICK_STR "rgb17,34,51"

#define MAX_PIXEL_NO ((IMAGE_TEST_FILE_WIDTH - 10) * (IMAGE_TEST_FILE_HEIGHT - 10) - 4000)

#define WRITE_BUFFER_LEN 8192

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

#define IMAGE_TEST_FILE "tests/quick_gimp_pattern_test.png"
#define IMAGE_TEST_FILE_WIDTH 450
#define IMAGE_TEST_FILE_HEIGHT 300

#define WIDTH_PARAM "w"
#define HEIGHT_PARAM "h"
#define STRICT_PARAM "s"
#define LOWQ_PARAM "wap"

#define TRUE_PARAM_VAL "t"

#define MEDIA_PATH "./"
#define CACHE_PATH "cache/"


#define OUT_FORMAT "JPEG"
#define OUT_FORMAT_MIME_TYPE "image/jpeg"

#define LOWQ_QUALITY 20
#define NORMAL_QUALITY 80

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

/* if output format doesn't handle transparency (like JPG) uncomment HANDLE_TRANSPARENCY so when transparent image is scaled it's transpaency will be replaced by DEFAULT_BACKGROUND_COLOR */
#define HANDLE_TRANSPARENCY

/* color to fill transparency when conferting from transparent gifs - yellowishs*/
/* #112233 hex -> rgb17,34,51 */
#define DEFAULT_BACKGROUND_COLOR "#112233"
#define DEFAULT_BACKGROUND_COLOR_MAGICK_STR "rgb17,34,51"

#define MAX_PIXEL_NO ((IMAGE_TEST_FILE_WIDTH - 10) * (IMAGE_TEST_FILE_HEIGHT - 10) - 4000)

#define ERROR_FILE_PATH "error.jpg"
#define ERROR_FILE_MIME_TYPE "image/jpeg"

#define WRITE_BUFFER_LEN 8192

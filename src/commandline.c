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

#include <string.h>
#include <stdlib.h>
#include <argp.h>

#include "commandline.h"
#include "runtime_config.h"
#include "file_utils.h"
#include "format_info.h"
#include "scaler.h"
#include "config.h"
#include "defaults.h"
#include "debug.h"

#ifdef DEBUG
extern char **scale_method_names;
#endif

const char *argp_program_version = "cgiscaler v" CGISCALER_VERSION;
const char *argp_program_bug_address = "<jpastuszek@gmail.com>";

/* Program documentation. */
static char doc[] = "CGIScaler is an image thumbnailer. It communicates with a web server using CGI\nThis options will override build in defaults (shown in []). Some of this options may be overridden by a CGI query string.";

#define STR(x) #x
#define DEFAULT(x) " ["STR(x)"]"

static struct argp_option options[] = {
	{0, 0, 0, 0, "Output geometry:\n"},
	{"width",					'w',	"INTEGER",	0, "Width of output image" DEFAULT(DEFAULT_WIDTH) },
	{"height",				'h',	"INTEGER",	0, "Height of output image" DEFAULT(DEFAULT_HEIGHT) },

	{0, 0, 0, 0, "Output format:\n"},
	{"out-format",			'O',	"STRING",	0, "Resulting thumbnail format (ex. JPG, GIF, PNG) " DEFAULT(OUT_FORMAT)},
	{"failback-mime-type",		'b',	"STRING",	0, "Mime-type that will be send in HTTP headers if one corresponding to format/extension could not be determined" DEFAULT(FAIL_BACK_MIME_TYPE)},

	{0, 0, 0, 0, "Simple CGI query parameter names:\n"},
	{"cgi-width",				'W',	"STRING",	0, "String to match width parameter in CGI query string" DEFAULT(QUERY_WIDTH_PARAM)},
	{"cgi-height",				'E',	"STRING",	0, "String to match height parameter in CGI query string" DEFAULT(QUERY_HEIGHT_PARAM)},
	{"cgi-strict",				'R',	"STRING",	0, "String to match strict enable parameter in CGI query string" DEFAULT(QUERY_STRICT_PARAM)},
	{"cgi-low-quality",			'L',	"STRING",	0, "String to mach low quality enable parameter in CGI query string" DEFAULT(QUERY_LOWQ_PARAM)},

	{0, 0, 0, 0, "Simple CGI query parameter values:\n"},
	{"cgi-true",				'T',	"STRING",	0, "String to mach true value in CGI query string" DEFAULT(QUERY_TRUE_VAL)},
	{"cgi-false",				'F',	"STRING",	0, "String to mach false value in CGI query string" DEFAULT(QUERY_FALSE_VAL)},

	{0, 0, 0, 0, "Simple CGI query defaults:\n"},
#if DEFAULT_SCALE_METHOD == SM_FIT
	{"strict-resize", 			's',	0,			0, "Do strict scaling (overwrites fit scaling)" DEFAULT(fit)},
#else
	{"fit-resize",				'f',	0,			0, "Do fit scaling (overwrites strict scaling)" DEFAULT(strict)},
#endif
	{"low-quality",			'l',	0,			0, "Produce more compressed output" DEFAULT(unset)},
	{"file-name",				'i',	"FILE",		0, "Use this file name if no file name passed in CGI query" DEFAULT(show error)},

	{"low-quality-value",		'Q',	"INTEGER",	0, "Image quality (1-100) to use when low-quality is enabled" DEFAULT(LOWQ_QUALITY)},
	{"normal-quality-value",	'N',	"INTEGER",	0, "Image quality (1-100) to use when low-quality is disabled" DEFAULT(DEFAULT_QUALITY)},

	{0, 0, 0, 0, "Storage\n"},
	{"media-dir",				'm',	"DIRECTORY",	0, "Root directory of media store - all file paths are relative to this directory" DEFAULT(MEDIA_PATH)},
	{"cache-dir",				'c',	"DIRECTORY",	0, "Root directory of cache store - all cached thumbnails will go there" DEFAULT(CACHE_PATH)},
	//{"outfile",	'o', "FILE",	0, "Output to file instead of stdout"},

	{0, 0, 0, 0, "Resize filtering:\n"},
	{"scaling-filter",			'G',	"STRING",	0, "Smoothing filter to use when resizing" DEFAULT(RESIZE_FILTER)},
	{"list-scaling-filter",		'Y',	0,			0, "List all possible smoothing filters"},
	{"blur-factor",			'B',	"REAL",		0, "Blur factor where > 1 is blurry, < 1 is sharp" DEFAULT(RESIZE_SMOOTH_FACTOR)},

	{0, 0, 0, 0, "Transparent image handling:\n"},
#ifdef REMOVE_TRANSPARENCY
	{"no-remove-transparency",	'n',	0,			0, "Do not replace transparent image area" DEFAULT(set)},
#else
	{"remove-transparency",	't',	0,			0, "Replace transparent image area with configured colour" DEFAULT(unset)},
#endif
	{"transparency-colour",		'u',	"STRING",	0, "Transparency replacement colour" DEFAULT(DEFAULT_BACKGROUND_COLOR)},

	{0, 0, 0, 0, "General operation:\n"},
	{"no-serve",				'S',	0,			0, "Do not serve the resulting image" DEFAULT(unset)},
	{"no-headers",			'H',	0,			0, "Do not serve HTTP headers" DEFAULT(unset)},
	{"no-cache",				'C',	0,			0, "Do disable cache" DEFAULT(unset)},

	{0, 0, 0, 0, "Error handling:\n"},
	{"error-file",				'e', "FILE",		0, "Serve this file in case of error" DEFAULT(ERROR_FILE_PATH)},
	{"error-message",			'M', "STRING",	0, "Error message to serve in case error file cannot be served" DEFAULT(ERROR_FAILBACK_MESSAGE)},

#ifdef DEBUG
	{0, 0, 0, 0, "Logging:\n"},
	{"log-file",				'g', "STRING",		0, "Error message to serve in case error file cannot be served" DEFAULT(DEBUG_FILE)},
#endif

	{0, 0, 0, 0, "Limits (units may vary between IM versions):\n"},
	{"max-out-pixels",			'P',	"INTEGER",	0, "If image area (in pixels) will be grater than this limit it's size will be reduced proportionally" DEFAULT(MAX_PIXEL_NO)},
	{"limit-files",				'U',	"INTEGER",	0, "Maximum number of open pixel cache files" DEFAULT(RESOURCE_LIMIT_FILE)},
	{"limit-disk",				'D',	"INTEGER",	0, "Maximum amount of disk space permitted for use by the pixel cache in bytes" DEFAULT(RESOURCE_LIMIT_DISK)},
	{"limit-map",				'A',	"INTEGER",	0, "Maximum amount of memory map to allocate for the pixel cache in bytes - when this limit is exceeded, the image pixels are cached to disk" DEFAULT(RESOURCE_LIMIT_MAP)},
	{"limit-memory",			'K',	"INTEGER",	0, "Maximum amount of memory to allocate for the pixel cache from the heap in bytes - when this limit is exceeded, the image pixels are cached to memory-mapped disk" DEFAULT(RESOURCE_LIMIT_MEMORY)},
	{"limit-area",				'J',	"INTEGER",	0, "Maximum amount of memory to allocate for image from in bytes - images that exceed the area limit are cached to disk" DEFAULT(RESOURCE_LIMIT_AREA)},

	{0, 0, 0, 0, "Other options:"},
	{ 0 }
};

//static char args_doc[] = "[-whsflSHC] [-i image_file] [-m media_dir] [-c cache_dir]";

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
	char *file_name;
	int i;

	// struct arguments *arguments = state->input;
	switch (key) {
		case 'w':
			output_config->size.w = atoi(arg);
			break;
		case 'h':
			output_config->size.h = atoi(arg);
			break;

		case 'O':
			output_config->format = format_to_format_info(arg);
			break;
		case 'b':
			output_config->fail_mime_type = strdup(arg);
			break;

		case 'W':
			simple_query_string_config->query_width_param = strdup(arg);
			break;
		case 'E':
			simple_query_string_config->query_height_param = strdup(arg);
			break;
		case 'R':
			simple_query_string_config->query_strict_param = strdup(arg);
			break;
		case 'L':
			simple_query_string_config->query_lowq_param = strdup(arg);
			break;

		case 'T':
			simple_query_string_config->query_true_param = strdup(arg);
			break;
		case 'F':
			simple_query_string_config->query_false_param = strdup(arg);
			break;

		case 's':
			output_config->scale_method = SM_STRICT;
			break;
		case 'f':
			output_config->scale_method = SM_FIT;
			break;
		case 'l':
			simple_query_string_config->use_loq_quality = 1;
			output_config->quality = simple_query_string_config->low_quality_value;
			break;
		case 'i':
			/* sanitize_file_path will allocate */
			file_name = sanitize_file_path(arg);
			if (file_name) {
				if (output_config->file_name)
					free(output_config->file_name);
				output_config->file_name = file_name;
			}
			break;
		case 'Q':
			simple_query_string_config->low_quality_value = atoi(arg);
			// Update output quality when value has changed
			if (simple_query_string_config->use_loq_quality)
				output_config->quality = simple_query_string_config->low_quality_value;
			break;
		case 'N':
			simple_query_string_config->default_quality_value = atoi(arg);
			break;

		case 'm':
			storage_config->media_directory = strdup(arg);
			break;
		case 'c':
			storage_config->cache_directory = strdup(arg);
			break;

		case 'G':
			for (i = 0; resize_filters[i].name; i++) {
				if (!strcmp(resize_filters[i].name, arg)) {
					output_config->scaling_filter = resize_filters[i].value;
					break;
				}
			}

			if (! resize_filters[i].name) {
				argp_error(state, "Unrecognized resiez filter '%s'", arg);
			}
			break;
		case 'Y':
			printf("Possible smoothing filters:\n");
			for (i = 0; resize_filters[i].name; i++) {
				printf("\t%s\n", resize_filters[i].name);
			}
			exit(0);
		case 'B':
			output_config->blur_factor = atof(arg);
			break;

#ifdef REMOVE_TRANSPARENCY
		case 'n':
			output_config->remove_transparency = 0;
			break;
#else
		case 't':
			output_config->remove_transparency = 1;
			break;
#endif
		case 'u':
			output_config->transparency_replacement_color = strdup(arg);
			break;

		case 'S':
			operation_config->no_serve = 1;
			break;
		case 'H':
			operation_config->no_headers = 1;
			break;
		case 'C':
			operation_config->no_cache = 1;
			break;

		case 'e':
			error_handling_config->error_image_file = strdup(arg);
			break;
		case 'M':
			error_handling_config->error_message = strdup(arg);
			break;

#ifdef DEBUG
		case 'g':
			logging_config->log_file = strdup(arg);
			break;
#endif


		case 'P':
			resource_limit_config->max_pixel_no = atoi(arg);
			break;
		case 'U':
			resource_limit_config->file_limit = atoi(arg);
			break;
		case 'D':
			resource_limit_config->disk_limit = atoi(arg);
			break;
		case 'A':
			resource_limit_config->map_limit = atoi(arg);
			break;
		case 'K':
			resource_limit_config->memory_limit = atoi(arg);
			break;
		case 'J':
			resource_limit_config->area_limit = atoi(arg);
			break;


		case ARGP_KEY_ARG:
			// non taged arg
			break;
		case ARGP_KEY_END:
			// done
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, 0, doc };

/** Apply configuration specified as command line parameters.
* @param argc argc parameter from main function
* @param argv argv parameter from main function
*/
void apply_commandline_config(int argc, char *argv[]) {
	argp_parse (&argp, argc, argv, 0, 0, 0);
}


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
	{"failback-mime-type",		'b',	"STRING",	0, "Mime-type that will be send in HTTP headers if one corresponding to format/extension could not be found" DEFAULT(FAIL_BACK_MIME_TYPE)},

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
	{"low-quality",			'l',	0,			0, "Produce more compressed output" DEFAULT(off)},
	{"file-name",				'i',	"FILE",		0, "Use this file name if no file name passed in CGI query" DEFAULT(show error)},

	{"low-quality-value",		'Q',	"INTEGER",	0, "Image quality (1-100) to use when low-quality is enabled" DEFAULT(LOWQ_QUALITY)},
	{"normal-quality-value",	'N',	"INTEGER",	0, "Image quality (1-100) to use when low-quality is disabled" DEFAULT(DEFAULT_QUALITY)},

	{0, 0, 0, 0, "Input and output:\n"},
	{"media-dir",				'm',	"DIRECTORY",	0, "Root directory of media store - all file paths are relative to this directory"},
	{"cache-dir",				'c', 	"DIRECTORY",	0, "Root directory of cache store - all cached thumbnails will go there"},
	//{"outfile",	'o', "FILE",	0, "Output to file instead of stdout"},

	{0, 0, 0, 0, "General operation:\n"},
	{"no-server",				'S',	0,			0, "Do not serve the resulting image"},
	{"no-headers",			'H',	0,			0, "Do not serve HTTP headers"},
	{"no-cache",				'C',	0,			0, "Do disable cache"},

	{0, 0, 0, 0, "Error handling:\n"},
	{"error-file",				'e', "FILE",		0, "Serve this file in case of error"},
	{"error-message",			'M', "STRING",	0, "Error message to serve in case error file cannot be served"},

	{0, 0, 0, 0, "Other options:"},
	{ 0 }
};

//static char args_doc[] = "[-whsflSHC] [-i image_file] [-m media_dir] [-c cache_dir]";

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
	char *file_name;
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

		case 'Q':
			simple_query_string_config->low_quality_value = atoi(arg);
			break;
		case 'N':
			simple_query_string_config->default_quality_value = atoi(arg);
			break;

		case 's':
			output_config->scale_method = SM_STRICT;
			break;
		case 'f':
			output_config->scale_method = SM_FIT;
			break;
		case 'l':
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
		case 'm':
			storage_config->media_directory = strdup(arg);
			break;
		case 'c':
			storage_config->cache_directory = strdup(arg);
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

#ifdef DEBUG
	debug(DEB, "Run-time config after command line: file: '%s', size w: %d h: %d, scale method: %s quality: %d Operation coifig: no cache: %d, no serve: %d, no headers: %d", output_config->file_name ? output_config->file_name : "<null>", output_config->size.w, output_config->size.h, scale_method_names[output_config->scale_method], output_config->quality, operation_config->no_cache, operation_config->no_serve, operation_config->no_headers);
#endif
	return;
}


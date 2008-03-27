
# set the include path found by configure
INCLUDES = $(all_includes)

# the library search path.
cgiscalerdir = /var/www/localhost/cgi-bin/scaler
cgiscaler_PROGRAMS = cgiscaler
noinst_HEADERS = build_config.h cache.h cgiscaler.h commandline.h debug.h \
	defaults.h file_utils.h format_info.h geometry_math.h query_string.h \
	runtime_config.h scaler.h serve.h
cgiscaler_SOURCES = build_config.h.tpl cache.c cgiscaler.c commandline.c \
	debug.c file_utils.c format_info.c geometry_math.c main.c query_string.c \
	runtime_config.c scaler.c serve.c
AM_CFLAGS = -Wall -lMagick -lWand

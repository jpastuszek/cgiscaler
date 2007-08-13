
# set the include path found by configure
INCLUDES = $(all_includes)

# the library search path.
cgiscalerdir = /var/www/localhost/cgi-bin/scaler
cgiscaler_PROGRAMS = cgiscaler
noinst_HEADERS = cache.h cgiscaler.h commandline.h debug.h file_utils.h \
	geometry_math.h query_string.h runtime_config.h serve.h
cgiscaler_SOURCES = cache.c commandline.c debug.c file_utils.c geometry_math.c \
	main.c query_string.c runtime_config.c scaler.c serve.c
AM_CFLAGS = -Wall -lMagick -lWand

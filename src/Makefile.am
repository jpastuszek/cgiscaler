
# set the include path found by configure
INCLUDES = $(all_includes)

# the library search path.
cgiscalerdir = /var/www/localhost/cgi-bin/scaler
cgiscaler_PROGRAMS = cgiscaler
noinst_HEADERS = cache.h cgiscaler.h debug.h geometry_math.h query_string.h \
	serve.h
cgiscaler_SOURCES = cache.c cgiscaler.c debug.c geometry_math.c main.c \
	query_string.c serve.c
AM_CFLAGS = -Wall -lMagick -lWand

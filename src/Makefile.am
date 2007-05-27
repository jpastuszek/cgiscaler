bin_PROGRAMS = cgiscaler
cgiscaler_SOURCES = cgiscaler.c debug.c geometry_math.c query_string.c

# set the include path found by configure
INCLUDES = $(all_includes)

# the library search path.
cgiscalerdir = /var/www/localhost/cgi-bin/scaler
cgiscaler_PROGRAMS = cgiscaler
noinst_HEADERS = config.h debug.h geometry_math.h query_string.h
cgiscaler_LDFLAGS = -lMagick -llcms -ltiff -lfreetype -ljpeg -lfontconfig \
	-lXext -lSM -lICE -lX11 -lXt -lbz2 -lz -lpthread -lm -lpthread -lWand -lMagick

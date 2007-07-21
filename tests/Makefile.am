METASOURCES = AUTO
noinst_PROGRAMS = test
AM_CFLAGS = -Wall -lMagick -lWand
noinst_HEADERS = cache.h cgiscaler.h config.h debug.h file_utils.h \
	geometry_math.h query_string.h runtime_config.h serve.h
test_LDADD = $(top_builddir)/cgreen/libcgreen.a
test_SOURCES = cache.c cgiscaler.c debug.c file_utils.c geometry_math.c \
	query_string.c runtime_config.c serve.c test.c

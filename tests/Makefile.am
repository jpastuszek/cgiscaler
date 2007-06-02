METASOURCES = AUTO
noinst_PROGRAMS = test
test_SOURCES = cache.c cgiscaler.c debug.c geometry_math.c query_string.c \
	serve.c test.c
AM_CFLAGS = -Wall -lMagick -lWand
noinst_HEADERS = cache.h cgiscaler.h config.h debug.h geometry_math.h \
	query_string.h serve.h
test_LDADD = $(top_builddir)/cgreen/libcgreen.a

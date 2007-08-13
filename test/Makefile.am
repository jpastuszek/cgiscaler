INCLUDES = 
METASOURCES = AUTO
bin_PROGRAMS = cache file_utils serve
file_utils_LDADD = $(top_builddir)/cgreen/libcgreen.a
file_utils_SOURCES = asserts.c debug.c file_utils.c test_file_utils.c
noinst_HEADERS = asserts.h cache.h cgiscaler.h commandline.h config.h debug.h \
	file_utils.h geometry_math.h query_string.h runtime_config.h serve.h stdio_capture.h \
	test_config.h
serve_LDADD = $(top_builddir)/cgreen/libcgreen.a
serve_SOURCES = asserts.c cache.c debug.c file_utils.c serve.c stdio_capture.c \
	test_serve.c
cache_LDADD = $(top_builddir)/cgreen/libcgreen.a
cache_SOURCES = asserts.c cache.c debug.c file_utils.c serve.c stdio_capture.c \
	test_cache.c

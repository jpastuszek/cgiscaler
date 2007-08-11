INCLUDES = 
METASOURCES = AUTO
bin_PROGRAMS = file_utils serve
file_utils_LDADD = $(top_builddir)/cgreen/libcgreen.a
file_utils_SOURCES = asserts.c debug.c file_utils.c test_file_utils.c
noinst_HEADERS = asserts.h cache.h cgiscaler.h commandline.h config.h debug.h \
	file_utils.h geometry_math.h query_string.h runtime_config.h serve.h test_config.h
serve_LDADD = $(top_builddir)/cgreen/libcgreen.a
serve_SOURCES = asserts.c cache.c debug.c file_utils.c serve.c test_serve.c

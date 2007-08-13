INCLUDES = 
METASOURCES = AUTO
bin_PROGRAMS = cache commandline file_utils geometry_math runtime_config serve
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
geometry_math_SOURCES = asserts.c debug.c geometry_math.c test_geometry_math.c
geometry_math_LDADD = $(top_builddir)/cgreen/libcgreen.a
geometry_math_LDFLAGS = -lm
runtime_config_SOURCES = debug.c runtime_config.c test_runtime_config.c
runtime_config_LDADD = $(top_builddir)/cgreen/libcgreen.a
commandline_SOURCES = asserts.c commandline.c debug.c file_utils.c \
	runtime_config.c test_commandline.c
commandline_LDADD = $(top_builddir)/cgreen/libcgreen.a

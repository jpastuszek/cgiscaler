METASOURCES = AUTO
bin_PROGRAMS = cache commandline file_utils geometry_math main query_string \
	runtime_config scaler serve
file_utils_LDADD = $(top_builddir)/cgreen/libcgreen.a
file_utils_SOURCES = asserts.c debug.c file_utils.c runtime_config.c \
	test_file_utils.c
noinst_HEADERS = asserts.h cache.h commandline.h debug.h file_utils.h \
	geometry_math.h main.h query_string.h runtime_config.h scaler.h serve.h stdio_capture.h \
	test_config.h
serve_LDADD = $(top_builddir)/cgreen/libcgreen.a
serve_SOURCES = asserts.c cache.c debug.c file_utils.c runtime_config.c serve.c \
	stdio_capture.c test_serve.c
cache_LDADD = $(top_builddir)/cgreen/libcgreen.a
cache_SOURCES = asserts.c cache.c debug.c file_utils.c runtime_config.c serve.c \
	stdio_capture.c test_cache.c
geometry_math_SOURCES = asserts.c debug.c geometry_math.c test_geometry_math.c
geometry_math_LDADD = $(top_builddir)/cgreen/libcgreen.a
geometry_math_LDFLAGS = -lm
runtime_config_SOURCES = debug.c runtime_config.c test_runtime_config.c
runtime_config_LDADD = $(top_builddir)/cgreen/libcgreen.a
commandline_SOURCES = asserts.c commandline.c debug.c file_utils.c \
	runtime_config.c test_commandline.c
commandline_LDADD = $(top_builddir)/cgreen/libcgreen.a
scaler_SOURCES = asserts.c debug.c file_utils.c geometry_math.c \
	runtime_config.c scaler.c test_scaler.c
AM_CFLAGS = -Wall -lMagick -lWand
scaler_LDADD = $(top_builddir)/cgreen/libcgreen.a
query_string_SOURCES = debug.c file_utils.c query_string.c runtime_config.c \
	test_query_string.c
query_string_LDADD = $(top_builddir)/cgreen/libcgreen.a
main_LDADD = $(top_builddir)/cgreen/libcgreen.a
main_SOURCES = asserts.c cache.c commandline.c debug.c file_utils.c \
	geometry_math.c main.c query_string.c runtime_config.c scaler.c serve.c stdio_capture.c \
	test_main.c
bin_SCRIPTS = memory_test unit_test

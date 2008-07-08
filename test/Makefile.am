METASOURCES = AUTO

noinst_HEADERS = asserts.h cache.h cgiscaler.h commandline.h debug.h defaults.h \
								         file_utils.h format_info.h geometry_math.h query_string.h runtime_config.h scaler.h \
												         serve.h stdio_capture.h test_config.h test_defaults.h

bin_PROGRAMS = cache commandline file_utils format_info geometry_math main \
							         query_string runtime_config scaler serve

noinst_LIBRARIES = libasserts.a

libasserts_a_SOURCES = asserts.c
libasserts_a_LIBADD = $(top_builddir)/cgreen/libcgreen.a $(top_builddir)/lib/libgnu.la

file_utils_SOURCES = debug.c file_utils.c format_info.c \
										         runtime_config.c test_file_utils.c
file_utils_LDADD = $(top_builddir)/cgreen/libcgreen.a $(top_builddir)/lib/libgnu.la libasserts.a

serve_SOURCES = cache.c debug.c file_utils.c format_info.c \
								        runtime_config.c serve.c stdio_capture.c test_serve.c
serve_LDADD = $(top_builddir)/cgreen/libcgreen.a $(top_builddir)/lib/libgnu.la libasserts.a

cache_SOURCES = cache.c debug.c file_utils.c format_info.c \
								        runtime_config.c serve.c stdio_capture.c test_cache.c
cache_LDADD = $(top_builddir)/cgreen/libcgreen.a $(top_builddir)/lib/libgnu.la libasserts.a

geometry_math_SOURCES = debug.c format_info.c geometry_math.c \
												        runtime_config.c test_geometry_math.c
geometry_math_LDADD = $(top_builddir)/cgreen/libcgreen.a $(top_builddir)/lib/libgnu.la libasserts.a

runtime_config_SOURCES = debug.c format_info.c runtime_config.c \
												         test_runtime_config.c
runtime_config_LDADD = $(top_builddir)/cgreen/libcgreen.a $(top_builddir)/lib/libgnu.la

commandline_SOURCES = commandline.c debug.c file_utils.c \
											        format_info.c geometry_math.c runtime_config.c scaler.c test_commandline.c
commandline_LDADD = $(top_builddir)/cgreen/libcgreen.a $(top_builddir)/lib/libgnu.la libasserts.a

scaler_SOURCES = debug.c file_utils.c format_info.c geometry_math.c \
								         runtime_config.c scaler.c test_scaler.c
scaler_LDADD = $(top_builddir)/cgreen/libcgreen.a $(top_builddir)/lib/libgnu.la libasserts.a

query_string_SOURCES = debug.c file_utils.c format_info.c query_string.c \
											         runtime_config.c test_query_string.c
query_string_LDADD = $(top_builddir)/cgreen/libcgreen.a

main_SOURCES = cache.c cgiscaler.c commandline.c debug.c file_utils.c \
							         format_info.c geometry_math.c query_string.c runtime_config.c scaler.c serve.c \
											         stdio_capture.c test_main.c
main_LDADD = $(top_builddir)/cgreen/libcgreen.a $(top_builddir)/lib/libgnu.la libasserts.a


format_info_SOURCES = debug.c format_info.c runtime_config.c test_format_info.c
format_info_LDADD = $(top_builddir)/cgreen/libcgreen.a $(top_builddir)/lib/libgnu.la

bin_SCRIPTS = memory_test unit_test

INCLUDES = -I/usr/local/include -I$(top_builddir)/lib

AM_CFLAGS = -Wall -I$(top_srcdir)/lib



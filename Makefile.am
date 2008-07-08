# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.4

ACLOCAL_AMFLAGS = -I m4 -I libltdl
EXTRA_DIST = m4/gnulib-cache.m4

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
				$(SHELL) ./config.status --recheck

SUBDIRS = lib cgreen src test


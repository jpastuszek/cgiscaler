#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.62)
AC_INIT(cgiscaler, 2.0, Jakub Pastuszek <jpastuszek@gmail.com>)
AC_CONFIG_SRCDIR([link-warning.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AM_INIT_AUTOMAKE
AC_PROG_CXX
AC_PROG_CC
gl_EARLY
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
gl_INIT

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])


# Check for imagemagick
AC_CHECK_PROG(magickconfig, [Magick-config --version], yes, no)

magickold=no
if test "$magickconfig" = "yes"; then
  magickversion=`Magick-config --version`
  magickmajor=`echo $magickversion | cut -d '.' -f 1`
  magickminor=`echo $magickversion | cut -d '.' -f 2`
  magicktiny=`echo $magickversion | cut -d '.' -f 3`

  if test "$magickmajor" -lt 6; then
    magickold="yes";
  fi
fi

if test $magickold = "yes"; then
  AC_MSG_ERROR(*** Old version of ImageMagick ($magickversion) found.  Please upgrade to 6.0.0 or newer. ***)
elif test $magickconfig = "no"; then
  AC_MSG_ERROR(*** Missing Magick-config (or it is not in the path). ***)
else
  CFLAGS="$CFLAGS `Magick-config --cflags`"
  LDFLAGS="$LDFLAGS `Magick-config --ldflags`"
  LIBS="`Magick-config --libs`"
  AC_CHECK_HEADER(wand/MagickWand.h, magickhdr="yes", magickhdr="no")
  if test "$magickhdr" = "no"; then
    oldcflags2="$CFLAGS"
    CFLAGS="$CFLAGS -I`Magick-config --prefix`/include";
    AC_CHECK_HEADER(wand/MagickWand.h, magickhdr="yes", magickhdr="no")
    if test "$magickhdr" = "no"; then
      CFLAGS="$oldcflags2";
      CFLAGS="$CFLAGS -I`Magick-config --prefix`/include/ImageMagick";
      AC_CHECK_HEADER(wand/MagickWand.h, magickhdr="yes", magickhdr="no")
      if test "$magickhdr" = "no"; then
        CFLAGS="$oldcflags2";
        AC_MSG_ERROR(*** Missing ImageMagick. ***)
      fi
    fi
  fi

  AC_DEFINE(HAVE_IMAGEMAGICK, 1, Imagemagick image library)
fi

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([OS.h fcntl.h float.h inttypes.h langinfo.h libintl.h limits.h locale.h malloc.h stddef.h stdint.h stdlib.h string.h strings.h sys/param.h sys/time.h unistd.h utime.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_UID_T
AC_C_INLINE
AC_FUNC_OBSTACK
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_FSEEKO
AC_FUNC_MALLOC
#AC_FUNC_REALLOC
AC_TYPE_SIGNAL
AC_FUNC_STRERROR_R
#AC_FUNC_STRNLEN
AC_FUNC_STRTOD
AC_CHECK_FUNCS([alarm atexit dup2 fchdir ftruncate getcwd getdelim getpagesize gettimeofday lchown localeconv memchr memmove mempcpy memset mkdir nl_langinfo putenv rmdir rpmatch setenv sqrt stpcpy strcasecmp strchr strcspn strdup strerror strncasecmp strndup strpbrk strrchr strspn strstr utime wcwidth])
AC_SUBST([LIBTOOL_DEPS])

AC_CONFIG_FILES([Makefile
                 cgreen/Makefile
                 lib/Makefile
                 src/Makefile
                 test/Makefile])
AC_OUTPUT


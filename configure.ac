# Process this file with autoconf to produce a configure script.

AC_INIT([garmintools],[0.10],[dave@daveb.net])
AC_CONFIG_SRCDIR(src/garmin.h)
AC_CONFIG_HEADERS(src/config.h:src/config.h.in)
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.11 -Wall no-define foreign dist-xz no-dist-gzip tar-ustar])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_AR
PKG_PROG_PKG_CONFIG

LT_PREREQ([2.2])
LT_INIT

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h termios.h unistd.h stdbool.h stdint.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_STRUCT_TM

AC_CHECK_SIZEOF([float])
AS_IF([test "x$ac_cv_sizeof_float" = "x4"],[],
      [
        AC_MSG_ERROR([float is not a 32bit type])
      ])
AC_CHECK_SIZEOF([double])
AS_IF([test "x$ac_cv_sizeof_double" = "x8"],[],
      [
       AC_MSG_ERROR([double is not a 64bit type])
      ])

# Checks for library functions.
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([memset strerror])

# Check endianness
AC_C_BIGENDIAN

# Checks for libusb
PKG_CHECK_MODULES([USB],[libusb-1.0],
        [],
        [AC_MSG_ERROR(cannot build garmintools without libusb)])

# Checks for python
AC_ARG_WITH(python,[  --with-python		  compile python bindings.],
[
if test "${withval}" = "yes"; then
   PYTHON_SUBDIRS="python" 
   AC_CHECK_PROG(HAVE_PYTHON, "python", "yes")
   if test "${HAVE_PYTHON}" != "yes"; then
      AC_MSG_ERROR([No usable python interpreter found])
   fi
fi
])

PROG_LIBS="$USB_LIBS"

AC_SUBST(USB_CFLAGS)
AC_SUBST(USB_LIBS)
AC_SUBST(PROG_LIBS)
AC_SUBST(PYTHON_SUBDIRS)

AC_CONFIG_FILES([Makefile \
  src/Makefile \
  doc/Makefile \
  doc/man/Makefile \
  extras/Makefile])

if test "${HAVE_PYTHON}" = "yes"; then
   AC_CONFIG_FILES([python/Makefile])
fi

AC_OUTPUT

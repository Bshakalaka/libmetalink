# libmetalink
#
# Copyright (c) 2012 Tatsuhiro Tsujikawa
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
AC_PREREQ(2.61)
AC_INIT([libmetalink], [0.1.3], [t-tujikawa@users.sourceforge.net])

LT_PREREQ([2.2.6])
LT_INIT()

dnl See versioning rule:
dnl  http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
AC_SUBST(LT_CURRENT, 4)
AC_SUBST(LT_REVISION, 0)
AC_SUBST(LT_AGE, 1)

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE()

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_MACRO_DIR([m4])

# Special host-dependent stuff
case ${host} in
    *android*|*mingw*)
        ac_cv_func_malloc_0_nonnull=yes
        ac_cv_func_realloc_0_nonnull=yes
        ;;
    *-*solaris*)
        ADDCFLAGS="-std=gnu99 -D_XOPEN_SOURCE=600"
        ;;
    *)
        ;;
esac

# Checks for arguments.
AC_ARG_WITH([libexpat], [  --with-libexpat            use libexpat library if installed. Default: yes], [with_libexpat=$withval], [with_libexpat=yes])
AC_ARG_WITH([libxml2], [  --with-libxml2             use libxml2 library if installed. Default: yes], [with_libxml2=$withval], [with_libxml2=yes])

AC_ARG_ENABLE([werror],
    [AS_HELP_STRING([--enable-werror],
                    [Turn on strict compile time warnings])],
    [werror=$enableval], [werror=no])

AC_ARG_ENABLE([gcc-visibility],
    [AS_HELP_STRING([--enable-gcc-visibility],
                    [Use gcc visibility attribute. This option may be removed in the future release.])],
    [gcc_visibility=$enableval], [gcc_visibility=no])

# Checks for programs
AC_PROG_CC
AC_PROG_INSTALL

# Package version
AC_SUBST([MAJOR_VERSION],
	 [[`echo $PACKAGE_VERSION | sed 's/\([0-9][0-9]*\)\.[0-9][0-9]*\.[0-9][0-9]*.*/\1/'`]])
AC_SUBST([MINOR_VERSION],
         [[`echo $PACKAGE_VERSION | sed 's/[0-9][0-9]*\.\([0-9][0-9]*\)\.[0-9][0-9]*.*/\1/'`]])
AC_SUBST([PATCH_VERSION],
         [[`echo $PACKAGE_VERSION | sed 's/[0-9][0-9]*\.[0-9][0-9]*\.\([0-9][0-9]*\).*/\1/'`]])
AC_SUBST([NUMBER_VERSION], `printf "0x%02x%02x%02x" $MAJOR_VERSION $MINOR_VERSION $PATCH_VERSION`)

# Checks for libraries.

if test "x$with_libexpat" = "xyes"; then
  PKG_CHECK_MODULES([EXPAT], [expat >= 2.1.0], [have_libexpat=yes], [have_libexpat=no])
  if test "x$have_libexpat" != "xyes"; then
    AC_MSG_WARN([$EXPAT_PKG_ERRORS])
    AM_PATH_LIBEXPAT
  fi
fi

if test "x$with_libxml2" = "xyes" && test "x$have_libexpat" != "xyes"; then
   AM_PATH_XML2([2.6.24], [have_libxml2=yes])
   if test "x$have_libxml2" = "xyes"; then
      AC_DEFINE([HAVE_LIBXML2], [1], [Define to 1 if you have libxml2.])
   fi
fi

if test "x$have_libexpat" != "xyes" && test "x$have_libxml2" != "xyes"; then
    AC_MSG_FAILURE([Neither expat nor libxml2 found. Please install one of \
them and run configure again.])
fi

# cunit
PKG_CHECK_MODULES([CUNIT], [cunit >= 2.1], [have_cunit=yes], [have_cunit=no])
# If pkg-config does not find cunit, check it using AC_CHECK_LIB.  We
# do this because Debian (Ubuntu) lacks pkg-config file for cunit.
if test "x${have_cunit}" = "xno"; then
  AC_MSG_WARN([${CUNIT_PKG_ERRORS}])
  AC_CHECK_LIB([cunit], [CU_initialize_registry],
               [have_cunit=yes], [have_cunit=no])
  if test "x${have_cunit}" = "xyes"; then
    CUNIT_LIBS="-lcunit"
    CUNIT_CFLAGS=""
    AC_SUBST([CUNIT_LIBS])
    AC_SUBST([CUNIT_CFLAGS])
  fi
fi

AM_CONDITIONAL([ENABLE_LIBEXPAT], [test "x$have_libexpat" = "xyes"])
AM_CONDITIONAL([ENABLE_LIBXML2], [test "x$have_libxml2" = "xyes"])
AM_CONDITIONAL([HAVE_CUNIT], [ test "x${have_cunit}" = "xyes" ])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([limits.h stdlib.h string.h time64.h alloca.h])

AC_CHECK_HEADER([inttypes.h], [have_inttypes_h=yes], [have_inttypes_h=no])

if test "x$have_inttypes_h" = "xyes"; then
    AC_DEFINE([HAVE_INTTYPES_H], [1],
              [Define to 1 if you have <inttypes.h> header file.])
    CPPFLAGS="-DHAVE_INTTYPES_H ${CPPFLAGS}"
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC

# _mkgmtime is available in Windows, behaves like timegm.
# _mkgmtime is for mingw. mkgmtime is for NetWare.
# Newer Android NDKs have timegm64 in the time64.h header.
AC_CHECK_FUNCS([memset strtol strtoll timegm64 _mkgmtime mkgmtime])
AC_CHECK_FUNC([timegm], [have_timegm=yes], [have_timegm=no])

if test "x$have_timegm" = "xyes"; then
    AC_DEFINE([HAVE_TIMEGM], [1],
              [Define to 1 if you have timegm function.])
fi
AM_CONDITIONAL([HAVE_TIMEGM], [ test "x$have_timegm" = "xyes" ])


AC_CHECK_FUNC([strptime], [have_strptime=yes], [have_strptime=no])

if test "x$have_strptime" = "xyes"; then
    AC_DEFINE([HAVE_STRPTIME], [1],
              [Define to 1 if you have strptime function.])
fi
AM_CONDITIONAL([HAVE_STRPTIME], [ test "x$have_strptime" = "xyes" ])

ac_save_CFLAGS=$CFLAGS
CFLAGS=

if test "x$werror" != "xno"; then
    AX_CHECK_COMPILE_FLAG([-Wall], [CFLAGS="$CFLAGS -Wall"])
    AX_CHECK_COMPILE_FLAG([-Wextra], [CFLAGS="$CFLAGS -Wextra"])
    AX_CHECK_COMPILE_FLAG([-Werror], [CFLAGS="$CFLAGS -Werror"])
    AX_CHECK_COMPILE_FLAG([-Wmissing-prototypes], [CFLAGS="$CFLAGS -Wmissing-prototypes"])
    AX_CHECK_COMPILE_FLAG([-Wstrict-prototypes], [CFLAGS="$CFLAGS -Wstrict-prototypes"])
    AX_CHECK_COMPILE_FLAG([-Wmissing-declarations], [CFLAGS="$CFLAGS -Wmissing-declarations"])
    AX_CHECK_COMPILE_FLAG([-Wpointer-arith], [CFLAGS="$CFLAGS -Wpointer-arith"])
    AX_CHECK_COMPILE_FLAG([-Wdeclaration-after-statement], [CFLAGS="$CFLAGS -Wdeclaration-after-statement"])
    AX_CHECK_COMPILE_FLAG([-Wformat-security], [CFLAGS="$CFLAGS -Wformat-security"])
    AX_CHECK_COMPILE_FLAG([-Wwrite-strings], [CFLAGS="$CFLAGS -Wwrite-strings"])
    AX_CHECK_COMPILE_FLAG([-Wshadow], [CFLAGS="$CFLAGS -Wshadow"])
    AX_CHECK_COMPILE_FLAG([-Winline], [CFLAGS="$CFLAGS -Winline"])
    AX_CHECK_COMPILE_FLAG([-Wnested-externs], [CFLAGS="$CFLAGS -Wnested-externs"])
    AX_CHECK_COMPILE_FLAG([-Wfloat-equal], [CFLAGS="$CFLAGS -Wfloat-equal"])
    AX_CHECK_COMPILE_FLAG([-Wundef], [CFLAGS="$CFLAGS -Wundef"])
    AX_CHECK_COMPILE_FLAG([-Wendif-labels], [CFLAGS="$CFLAGS -Wendif-labels"])
    AX_CHECK_COMPILE_FLAG([-Wempty-body], [CFLAGS="$CFLAGS -Wempty-body"])
    AX_CHECK_COMPILE_FLAG([-Wcast-align], [CFLAGS="$CFLAGS -Wcast-align"])
    AX_CHECK_COMPILE_FLAG([-Wclobbered], [CFLAGS="$CFLAGS -Wclobbered"])
    AX_CHECK_COMPILE_FLAG([-Wvla], [CFLAGS="$CFLAGS -Wvla"])
    AX_CHECK_COMPILE_FLAG([-Wpragmas], [CFLAGS="$CFLAGS -Wpragmas"])
    AX_CHECK_COMPILE_FLAG([-Wunreachable-code], [CFLAGS="$CFLAGS -Wunreachable-code"])
    AX_CHECK_COMPILE_FLAG([-Waddress], [CFLAGS="$CFLAGS -Waddress"])
    AX_CHECK_COMPILE_FLAG([-Wattributes], [CFLAGS="$CFLAGS -Wattributes"])
    AX_CHECK_COMPILE_FLAG([-Wdiv-by-zero], [CFLAGS="$CFLAGS -Wdiv-by-zero"])
    AX_CHECK_COMPILE_FLAG([-Wshorten-64-to-32], [CFLAGS="$CFLAGS -Wshorten-64-to-32"])

    # Only work with Clang for the moment
    AX_CHECK_COMPILE_FLAG([-Wheader-guard], [CFLAGS="$CFLAGS -Wheader-guard"])
fi

WARNCFLAGS=$CFLAGS
CFLAGS=$ac_save_CFLAGS

AC_SUBST([WARNCFLAGS])

if test "x$gcc_visibility" != "xno"; then
    ADDCFLAGS="-fvisibility=hidden $ADDCFLAGS"
    AC_DEFINE([GCC_VISIBILITY], [1], [Define to 1 if you use gcc visibility attribute.])
fi

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile
	lib/Makefile
	lib/libmetalink.pc
        lib/includes/metalink/metalinkver.h
	lib/includes/Makefile
	test/Makefile
	doc/Makefile
	doc/examples/Makefile
	doc/man3/Makefile
	m4/Makefile
])
AC_OUTPUT

AC_MSG_NOTICE([summary of build options:

    Version:        ${VERSION} (${MAJOR_VERSION}:${MINOR_VERSION}:${PATCH_VERSION}), shared $LT_CURRENT:$LT_REVISION:$LT_AGE
    Host type:      ${host}
    Install prefix: ${prefix}
    C compiler:     ${CC}
    CFLAGS:         ${CFLAGS}
    ADDCFLAGS:      ${ADDCFLAGS}
    WARNCFLAGS:     ${WARNCFLAGS}
    LDFLAGS:        ${LDFLAGS}
    LIBS:           ${LIBS}
    CPPFLAGS:       ${CPPFLAGS}
    C preprocessor: ${CPP}
    Library types:  Shared=${enable_shared}, Static=${enable_static}
    Libexpat:       ${have_libexpat} ${EXPAT_CFLAGS} ${EXPAT_LIBS}
    Libxml2:        ${have_libxml2} ${XML_CPPFLAGS} ${XML_LIBS}
    CUnit:          ${have_cunit} ${CUNIT_CFLAGS} ${CUNIT_LIBS}
])

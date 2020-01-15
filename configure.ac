#                                               -*- Autoconf -*-
#  @configure_input@
#
#  Configuration scipt for eCAP EXIF metadata filter
#
# Process this file with autoconf to produce a configure script.

m4_define([VERSION_MAJOR],[0])
m4_define([VERSION_MINOR],[5])
m4_define([VERSION_MICRO],[4])
AC_INIT(eCAP EXIF metadata filter, m4_defn([VERSION_MAJOR]).m4_defn([VERSION_MINOR]).m4_defn([VERSION_MICRO]), [], squid-ecap-exif)

AC_DEFINE([ECAP_SERVICE_NAME], ["ecap://www.thecacheworks.com/exif-filter"], [service name])

LIBADAPTER_CURRENT=5
LIBADAPTER_REVISION=4
LIBADAPTER_AGE=0

# 0.5.3 had 5:3:0
# 0.5.4 had 5:4:0

#   Current  -- the number of the binary API that we're implementing
#   Revision -- which iteration of the implementation of the binary
#               API are we supplying?
#   Age      -- How many previous binary API versions do we also
#               support?
#
# If we release a new version that does not change the binary API,
# increment Revision.
#
# If we release a new version that changes the binary API, but does
# not break programs compiled against the old binary API, increment
# Current and Age.  Set Revision to 0, since this is the first
# implementation of the new API.
#
# Otherwise, we're changing the binary API and breaking backward
# compatibility with old binaries.  Increment Current.  Set Age to 0,
# since we're backward compatible with no previous APIs.  Set Revision
# to 0 too.
AC_SUBST(LIBADAPTER_CURRENT)
AC_SUBST(LIBADAPTER_REVISION)
AC_SUBST(LIBADAPTER_AGE)

AC_CONFIG_AUX_DIR(cfgaux)
AC_CONFIG_MACRO_DIR([acinclude])
AC_CONFIG_SRCDIR([src/])
AC_CONFIG_HEADERS(src/autoconf.h)

AM_INIT_AUTOMAKE([subdir-objects])
AM_MAINTAINER_MODE

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

LT_INIT

dnl http://www.gnu.org/savannah-checkouts/gnu/autoconf/manual/autoconf-2.69/html_node/C_002b_002b-Compiler.html
: ${CXXFLAGS=""}
PRESET_CXXFLAGS="$CXXFLAGS"

# Checks for programs
AC_PROG_CXX([clang++ g++])
AC_LANG([C++])
AX_CXX_HAVE_STL
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_PROG_EGREP
if test "x$ac_cv_path_EGREP" = "xno"; then
  AC_MSG_ERROR([egrep was not found; install it first])
fi

dnl Check STL
if test "x$ax_cv_cxx_have_stl" = "xno"; then
  AC_MSG_ERROR([STL was not found; it is require to build])
fi

dnl If the user did not specify a C++ version.
user_cxx=`echo "$PRESET_CXXFLAGS" | $EGREP -o -E "\-std="`
if test "x$user_cxx" = "x"; then
  dnl Check for C++11 compiler support
  AX_CXX_COMPILE_STDCXX(11, [noext], [mandatory])
fi

dnl If the user did not specify optimization level
user_opt=`echo "$PRESET_CXXFLAGS" | $EGREP -o -E "\-O"`
if test "x$user_opt" = "x"; then
  dnl Set default optimization level
  CXXFLAGS="$CXXFLAGS -O3"
fi

# Checks for libraries
AC_CHECK_LIB([ecap], [main],
    [LIBS="-lecap $LIBS"],
    [AC_MSG_FAILURE([libecap is not found or unusable])]
)

AC_CHECK_LIB([exiv2], [main],
    [LIBS="-lexiv2 $LIBS"],
    [AC_MSG_FAILURE([libexiv is not found or unusable])]
)

AC_CHECK_LIB([tag], [main],
    [LIBS="-ltag -ltag_c $LIBS"],
    [AC_MSG_FAILURE([taglib is not found or unusable])]
)

AC_CHECK_LIB([zip], [main],
    [LIBS="-lzip $LIBS"],
    [AC_MSG_FAILURE([libzip is not found or unusable])]
)

AC_CHECK_LIB([podofo], [main],
    [LIBS="-lpodofo $LIBS"],
    [AC_MSG_FAILURE([podofo is not found or unusable])]
)

# Checks for header files
AC_CHECK_HEADERS([libecap/adapter/service.h])

AC_PATH_TOOL(PKG_CONFIG, pkg-config)

# Libecap compatibility check
if ! $PKG_CONFIG --atleast-version=1.0 libecap; then
    AC_MSG_ERROR(Check PKG_CONFIG_PATH points on libecap.pc dir or get latest libecap from e-cap.org)
fi

AC_CONFIG_FILES([\
        Makefile \
        src/Makefile \
])

AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_LIBS)
AC_MSG_NOTICE([BUILD C++ FLAGS: $CXXFLAGS])
AC_MSG_NOTICE([BUILD LDFLAGS: $LDFLAGS])
AC_MSG_NOTICE([BUILD LIBS: $LIBS])

AC_OUTPUT

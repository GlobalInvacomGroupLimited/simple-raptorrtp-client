#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])


# Init
AC_INIT([raptorrtpClient], [1.0-rc1], [steven.fisher@globalinvacom.com])
AC_CONFIG_SRCDIR([src/raptorrtpClient.cpp])
AM_INIT_AUTOMAKE


# Versioning
RAPTORRTPCLIENT_RELEASE=$PACKAGE_VERSION
RAPTORRTPCLIENT_LIBRARY_VERSION=1:0:0
#                         | | |
#                  +------+ | +---+
#                  |        |     |
#                current:revision:age
#                  |        |     |
#                  |        |     +- increment if interfaces have been added
#                  |        |        set to zero if interfaces have been removed
#                  |        |        or changed
#                  |        +- increment if source code has changed
#                  |           set to zero if current is incremented
#                  +- increment if interfaces have been added, removed or changed
AC_SUBST(RAPTORRTPCLIENT_LIBRARY_VERSION)
AC_SUBST(RAPTORRTPCLIENT_RELEASE)


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h syslog.h])
AC_ARG_WITH(liveMedia,
[  --with-liveMedia[[=DIR]]       use liveMedia in DIR],[
  if test "$withval" != "no" -a "$withval" != "yes"; then
    LIVEMEDIA_DIR=$withval
    CPPFLAGS="${CPPFLAGS} -I$withval"
  fi
])

AC_ARG_WITH(BasicUsageEnvironment,
[  --with-BasicUsageEnvironment[[=DIR]]       use BasicUsageEnvironment in DIR],[
  if test "$withval" != "no" -a "$withval" != "yes"; then
    BASICUSAGEENVIRONMENT_DIR=$withval
    CPPFLAGS="${CPPFLAGS} -I$withval"
  fi
])

AC_ARG_WITH(groupsock,
[  --with-groupsock[[=DIR]]       use groupsock in DIR],[
  if test "$withval" != "no" -a "$withval" != "yes"; then
    GROUPSOCK_DIR=$withval
    CPPFLAGS="${CPPFLAGS} -I$withval"
  fi
])

AC_ARG_WITH(UsageEnvironment,
[  --with-UsageEnvironment[[=DIR]]       use UsageEnvironment in DIR],[
  if test "$withval" != "no" -a "$withval" != "yes"; then
    USAGEENVIRONMENT_DIR=$withval
    CPPFLAGS="${CPPFLAGS} -I$withval"
  fi
])



# Check for boost
AC_LANG_PUSH(C++)
AC_CHECK_HEADER([curl/curl.h], [], [AC_MSG_ERROR([You need libcurl. install libcurl4-openssl-dev])])
AC_CHECK_HEADER([groupsock/NetCommon.h], [], [AC_MSG_ERROR([You need Live555.])])
AC_CHECK_HEADER([pvraptor/pvraptor.h], [], [AC_MSG_ERROR([You need raptor.])])
AC_CHECK_HEADER([raptorrtp/RaptorRTPSource.hh], [], [AC_MSG_ERROR([You need raptorrtp.])])
AC_LANG_POP(C++)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
#AC_CHECK_FUNCS([clock_gettime floor memset])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

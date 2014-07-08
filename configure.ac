AC_PREREQ(2.58)

AC_INIT([sharedance],[0.6],[bugs@pureftpd.org])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.9 dist-bzip2])
AC_CONFIG_LIBOBJ_DIR(src)

AC_SUBST(VERSION)

ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC
AC_AIX
AC_ISC_POSIX
AC_USE_SYSTEM_EXTENSIONS

CPPFLAGS="$CPPFLAGS -I/usr/local/include"
LDFLAGS="$LDFLAGS -L/usr/local/lib"

if uname | fgrep SunOS > /dev/null 2> /dev/null ; then
  CPPFLAGS="$CPPFLAGS -D__EXTENSIONS__=1 -D_XPG4_2=1"
fi

dnl Checks for header files

AC_HEADER_STDC
AC_HEADER_STAT
AC_HEADER_TIME
AC_HEADER_DIRENT
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(unistd.h string.h strings.h sys/param.h ioctl.h sys/ioctl.h)
AC_CHECK_HEADERS(sys/time.h sys/resource.h)
AC_CHECK_HEADERS(stddef.h)
AC_CHECK_HEADERS(stdarg.h varargs.h)
AC_CHECK_HEADERS(getopt.h)
AC_CHECK_HEADERS(netinet/in_systm.h netinet/in.h)
AC_CHECK_HEADERS(fcntl.h sys/fcntl.h ioctl.h sys/ioctl.h)

dnl Checks for typedefs, structures, and compiler characteristics.

AC_PROG_GCC_TRADITIONAL
AC_C_INLINE
AC_C_CONST
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_TYPE_SIGNAL
AC_TYPE_UID_T

dnl Socket things

AC_CHECK_FUNC(connect, , [AC_CHECK_LIB(socket, connect)])

AC_CHECK_FUNC(gethostbyname, , [AC_CHECK_LIB(resolv, gethostbyname)])

AC_CHECK_FUNC(gethostbyname, , [AC_CHECK_LIB(nsl, gethostbyname)])

if test "x$ac_cv_lib_nsl_gethostbyname" != "xyes" && test "x$ac_cv_func_gethostbyname" != "xyes" ; then
  AC_CHECK_FUNC(gethostbyname, , [AC_CHECK_LIB(socket, gethostbyname)])
fi

if test "$ac_cv_lib_nsl_gethostbyname" = "$ac_cv_func_gethostbyname" ; then
  AC_MSG_CHECKING([if we can include libnsl + libsocket])
  LIBS="-lnsl -lsocket $LIBS"
  AC_TRY_LINK( , [(void) gethostbyname], my_ac_link_result=yes,
             my_ac_link_result=no )
  if test "$my_ac_link_result" = "no" ; then
    AC_MSG_RESULT([failure])
    AC_MSG_ERROR([unable to use gethostbyname()])
  else
    AC_MSG_RESULT([success])
  fi
fi

dnl Types - continued

AC_CHECK_TYPE(socklen_t, , [AC_DEFINE(socklen_t, int, [socklen_t type])],
[
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
])

AC_CHECK_TYPE(ssize_t, , [AC_DEFINE(ssize_t, long int, [ssize_t type])],
[
#include <stdio.h>
#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/types.h>
])

AC_CHECK_TYPE(sig_atomic_t, , [AC_DEFINE(sig_atomic_t, signed char, [sig_atomic_t type])],
[
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/types.h>
#include <signal.h>
])

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_MMAP
AC_FUNC_MEMCMP
AC_FUNC_STRFTIME
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_FUNC_GETGROUPS

AC_CHECK_FUNCS(initgroups setgroups waitpid setproctitle getopt_long)
AC_CHECK_FUNCS(seteuid setreuid setresuid setegid setregid setresgid)
AC_CHECK_FUNCS(pread)
AC_CHECK_FUNCS(strtoull strtoq)
AC_CHECK_FUNCS(memset munmap strdup)
AC_CHECK_FUNCS(getaddrinfo getnameinfo inet_ntop inet_pton)
AC_CHECK_FUNCS(snprintf vsnprintf vfprintf gethostname setlocale)
AC_CHECK_FUNCS(random srandomdev)

AC_CHECK_LIB(event, bufferevent_new, ,
  AC_MSG_ERROR([** You need to install libevent (see the README file) **]))

dnl Custom checks

AC_MSG_CHECKING(whether snprintf is C99 conformant)
AC_TRY_RUN([
#include <stdio.h>
#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

int main(void)
{
    char buf[4];
    
    (void) fprintf(fopen("conftestval", "wt"), "%d\n",
        (int) snprintf(buf, sizeof buf, "12345678"));
    return 0;
}
],
CONF_SNPRINTF_TYPE=`cat conftestval`
)
AC_MSG_RESULT(done)
if test "x$CONF_SNPRINTF_TYPE" = "x" ; then
  AC_MSG_WARN(your operating system doesn't implement snprintf)
else
  AC_DEFINE_UNQUOTED(CONF_SNPRINTF_TYPE, $CONF_SNPRINTF_TYPE, [return value of an overflowed snprintf])
fi

dnl Options

AM_WITH_DMALLOC

AC_ARG_WITH(debug,
[AC_HELP_STRING([--with-debug],[For maintainers only - please do not use])],
[ if test "x$withval" = "xyes" ; then
    CFLAGS="$CFLAGS -DDEBUG=1 -g -Wall -W -Wcast-align -Wbad-function-cast -Wstrict-prototypes -Wwrite-strings -Wreturn-type "
  fi ])

AC_ARG_WITH(largefile,
[AC_HELP_STRING([--with-largefile],[Support for files > 2 Gb])],
[ if test "x$withval" = "xyes" ; then
    AC_SYS_LARGEFILE
    AC_DEFINE(WITH_LARGE_FILES,,[with largefile])
  fi ])

dnl Output

AC_CONFIG_FILES(Makefile src/Makefile php/Makefile)

AC_OUTPUT


#ifndef __SHAREDANCED_H__
#define __SHAREDANCED_H__ 1

#ifndef __GNUC__
# ifdef __attribute__
#  undef __attribute__
# endif
# define __attribute__(a)
#endif

#include <stdio.h>
#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
# include <stdarg.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#else
# if HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#include <syslog.h>
#ifndef MAX_SYSLOG_LINE
# define MAX_SYSLOG_LINE 4096U
#endif

#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#elif defined(HAVE_SYS_FCNTL_H)
# include <sys/fcntl.h>
#endif
#ifdef HAVE_IOCTL_H
# include <ioctl.h>
#elif defined(HAVE_SYS_IOCTL_H)
# include <sys/ioctl.h>
#endif
#include <sys/socket.h>
#ifdef HAVE_NETINET_IN_SYSTM_H
# include <netinet/in_systm.h>
#endif
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/mman.h>
#include <pwd.h>
#include <grp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <event.h>

#ifndef HAVE_GETOPT_LONG
# include "bsd-getopt_long.h"
#else
# include <getopt.h>
#endif

#ifdef HAVE_ALLOCA
# ifdef HAVE_ALLOCA_H
#  include <alloca.h>
# endif
# define ALLOCA(X) alloca(X)
# define ALLOCA_FREE(X) do { } while (0)
#else
# define ALLOCA(X) malloc(X)
# define ALLOCA_FREE(X) free(X)
#endif

#include "gettext.h"
#define  _(txt) gettext(txt)
#define N_(txt) txt

#include "mysnprintf.h"

#ifndef errno
extern int errno;
#endif

#ifdef TCP_CORK
# define CORK_ON(SK) do { int optval = 1; setsockopt(SK, SOL_TCP, TCP_CORK, \
  &optval, sizeof optval); } while(0)
# define CORK_OFF(SK) do { int optval = 0; setsockopt(SK, SOL_TCP, TCP_CORK, \
  &optval, sizeof optval); } while(0)
#else
# define CORK_ON(SK) do { } while(0)
# define CORK_OFF(SK) do { } while(0)
#endif

#ifndef O_NOFOLLOW
# define O_NOFOLLOW 0
#endif

#ifndef O_DIRECTORY
# define O_DIRECTORY 0
#endif

#ifndef SOL_IP
# define SOL_IP IPPROTO_IP
#endif
#ifndef SOL_TCP
# define SOL_TCP IPPROTO_TCP
#endif

#ifndef INADDR_NONE
# define INADDR_NONE 0
#endif

#if !defined(O_NDELAY) && defined(O_NONBLOCK)
# define O_NDELAY O_NONBLOCK
#endif

#ifndef FNDELAY
# define FNDELAY O_NDELAY
#endif

#ifndef HAVE_STRTOULL
# ifdef HAVE_STRTOQ
#  define strtoull(X, Y, Z) strtoq(X, Y, Z)
# else
#  define strtoull(X, Y, Z) strtoul(X, Y, Z)
# endif
#endif

#ifndef ULONG_LONG_MAX
# define ULONG_LONG_MAX (1ULL << 63)
#endif

#ifdef HAVE_DIRENT_H
# include <dirent.h>
#endif
#ifdef HAVE_SYS_NDIR_H
# include <sys/ndir.h>
#endif
#ifdef HAVE_NDIR_H
# include <ndir.h>
#endif

#ifdef STAT_MACROS_BROKEN
# undef S_ISBLK
# undef S_ISCHR
# undef S_ISDIR
# undef S_ISFIFO
# undef S_ISLNK
# undef S_ISMPB
# undef S_ISMPC
# undef S_ISNWK
# undef S_ISREG
# undef S_ISSOCK
#endif                            /* STAT_MACROS_BROKEN.  */

#ifndef S_IFMT
# define S_IFMT 0170000
#endif
#if !defined(S_ISBLK) && defined(S_IFBLK)
# define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#endif
#if !defined(S_ISCHR) && defined(S_IFCHR)
# define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#endif
#if !defined(S_ISDIR) && defined(S_IFDIR)
# define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
#if !defined(S_ISREG) && defined(S_IFREG)
# define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif
#if !defined(S_ISFIFO) && defined(S_IFIFO)
# define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#endif
#if !defined(S_ISLNK) && defined(S_IFLNK)
# define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif
#if !defined(S_ISSOCK) && defined(S_IFSOCK)
# define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
#endif
#if !defined(S_ISMPB) && defined(S_IFMPB)    /* V7 */
# define S_ISMPB(m) (((m) & S_IFMT) == S_IFMPB)
# define S_ISMPC(m) (((m) & S_IFMT) == S_IFMPC)
#endif
#if !defined(S_ISNWK) && defined(S_IFNWK)    /* HP/UX */
# define S_ISNWK(m) (((m) & S_IFMT) == S_IFNWK)
#endif

#ifndef S_IEXEC
# define S_IEXEC S_IXUSR
#endif

#ifndef S_IXUSR
# define S_IXUSR S_IEXEC
#endif
#ifndef S_IXGRP
# define S_IXGRP (S_IEXEC >> 3)
#endif
#ifndef S_IXOTH
# define S_IXOTH (S_IEXEC >> 6)
#endif
#ifndef S_IXUGO
# define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#endif

#ifndef STDIN_FILENO
# define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
# define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
# define STDERR_FILENO 2
#endif

#ifndef HAVE_SETEUID
# ifdef HAVE_SETREUID
#  define seteuid(X) setreuid(-1, (X))
# elif defined(HAVE_SETRESUID)
#  define seteuid(X) setresuid(-1, (X), -1)
# else
#  define seteuid(X) (-1)
# endif
#endif
#ifndef HAVE_SETEGID
# ifdef HAVE_SETREGID
#  define setegid(X) setregid(-1, (X))
# elif defined(HAVE_SETRESGID)
#  define setegid(X) setresgid(-1, (X), -1)
# else
#  define setegid(X) (-1)
# endif
#endif

#define DEFAULT_PORT_S "1042"
#define DEFAULT_FACILITY LOG_DAEMON
#define DEFAULT_BACKLOG 100
#define DEFAULT_STORAGE_DIR "/tmp/sharedance"
#define DEFAULT_READ_CHUNK_SIZE 4096U
#define DEFAULT_MAX_READ_SIZE (32U * 1024U * 1024U)
#define DEFAULT_TIMEOUT 60
#define DEFAULT_EXPIRATION (30 * 60)

#define EXPIRE_PAUSE 60
#define EXPIRE_NICENESS 15

#define RETURNCODE_OK "OK\n"

#define STOREFILE_TMP_PREFIX "."

typedef enum ClientCommand_ {
    CC_UNDEF = 0,
        CC_STORE = 'S',
        CC_FETCH = 'F',
        CC_DELETE = 'D'
} ClientCommand;

typedef struct Client_ {
    unsigned char *read_buf;
    size_t sizeof_read_buf;
    size_t offset_read_buf;
    struct event ev_client_read;
    int client_fd;
    ClientCommand client_command;
    size_t key_len;
    size_t data_len;
    size_t total_len;
    int write_fd;
    int read_fd;
    unsigned char *read_mapped_zone;
    size_t read_mapped_zone_length;
    struct bufferevent *returncode_bufev;
} Client;

#include "globals.h"

#endif

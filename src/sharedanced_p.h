#ifndef __SHAREDANCED_P_H__
#define __SHAREDANCED_P_H__

static const char *GETOPT_OPTIONS = "b:d:e:g:hi:p:r:t:u:BDR:";

static struct option long_options[] = {
    { "help", 0, NULL, 'h' },
    { "daemonize", 0, NULL, 'B' },
    { "debug", 0, NULL, 'D' },
    { "ip", 1, NULL, 'i' },
    { "port", 1, NULL, 'p' },
    { "backlog", 1, NULL, 'b' },
    { "directory", 1, NULL, 'd' },
    { "readsize", 1, NULL, 'r' },
    { "maxreadsize", 1, NULL, 'R' },
    { "expiration", 1, NULL, 'e' },
    { "timeout", 1, NULL, 't' },
    { "gid", 1, NULL, 'g' },
    { "uid", 1, NULL, 'u' },
    { NULL, 0, NULL, 0 }
};

#if !defined(NO_PROCNAME_CHANGE) && defined(__linux__) && !defined(HAVE_SETPROCTITLE)
static char **argv0;
static int argv_lth;
#endif

#endif

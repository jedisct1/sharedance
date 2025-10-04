
#define DEFINE_GLOBALS 1

#include <config.h>
#include "sharedanced.h"
#ifdef HAVE_SETLOCALE
# include <locale.h>
#endif
#include "log.h"
#include "daemonize.h"
#include "expire.h"
#include "process.h"
#include "sharedanced_p.h"

#ifdef WITH_DMALLOC
# include <dmalloc.h>
#endif

static void die_mem(void)
{
    logfile(LOG_ERR, _("Out of memory"));

    exit(EXIT_FAILURE);
}

static void sigchld(int fd, short event, void *ev_)
{
    struct event *ev = ev_;

    (void) fd;
    (void) event;
    signal_del(ev);
    _exit(EXIT_SUCCESS);
}

static void sigterm(int fd, short event, void *ev_)
{
    struct event *ev = ev_;

    (void) fd;
    (void) event;
    if (expire_pid >= (pid_t) 1) {
        (void) kill(expire_pid, SIGTERM);
    }
    signal_del(ev);
}

static int changeuidgid(void)
{
    if (uid == (uid_t) 0) {
        return 0;
    }
    if (
#ifdef HAVE_SETGROUPS
        setgroups(1U, &gid) ||
#endif
        setgid(gid) || setegid(gid) ||
        setuid(uid) || seteuid(uid) || chdir("/")) {
        return -1;
    }
    return 0;
}

static void clearargs(int argc, char **argv)
{
#ifndef NO_PROCNAME_CHANGE
# if defined(__linux__) && !defined(HAVE_SETPROCTITLE)
    int i;

    for (i = 0; environ[i] != NULL; i++);
    argv0 = argv;
    if (i > 0) {
        argv_lth = environ[i-1] + strlen(environ[i-1]) - argv0[0];
    } else {
        argv_lth = argv0[argc-1] + strlen(argv0[argc-1]) - argv0[0];
    }
    if (environ != NULL) {
        char **new_environ;
        unsigned int env_nb = 0U;

        while (environ[env_nb] != NULL) {
            env_nb++;
        }
        if ((new_environ = malloc((1U + env_nb) * sizeof (char *))) == NULL) {
            abort();
        }
        new_environ[env_nb] = NULL;
        while (env_nb > 0U) {
            env_nb--;
            /* Can any bad thing happen if strdup() ever fails? */
            if ((new_environ[env_nb] = strdup(environ[env_nb])) == NULL) {
                unsigned int temp_env_nb = env_nb;
                unsigned int orig_env_nb = env_nb;
                while (temp_env_nb < (1U + orig_env_nb)) {
                    free(new_environ[temp_env_nb]);
                    temp_env_nb++;
                }
                free(new_environ);
                abort();
            }
        }
        environ = new_environ;
    }
# else
    (void) argc;
    (void) argv;
# endif
#endif
}

static void set_progname(const char * const title)
{
#ifndef NO_PROCNAME_CHANGE
# ifdef HAVE_SETPROCTITLE
    setproctitle("-%s", title);
# elif defined(__linux__)
    if (argv0 != NULL) {
        memset(argv0[0], 0, argv_lth);
        strncpy(argv0[0], title, argv_lth - 2);
        argv0[1] = NULL;
    }
# elif defined(__hpux__)
    union pstun pst;

    pst.pst_command = title;
    pstat(PSTAT_SETCMD, pst, strlen(title), 0, 0);
# endif
#endif
    (void) title;
}

static void usage(void)
{
    puts("\n" PACKAGE_STRING " - " __DATE__ "\n");
    fputs(_(
            "--ip=xxx          (-i) listen to this address (default=any)\n"
            "--port=xxx        (-p) listen to that port (default=1042)\n"
            "--backlog=xxx     (-b) change the backlog\n"
            "--directory=xxx   (-d) directory where the data should be stored\n"
            "--readsize=xxx    (-r) read buffer size\n"
            "--maxreadsize=xxx (-R) max read size\n"
            "--expiration=xxx  (-e) expiration of data in seconds\n"
            "--timeout=xxx     (-t) disconnect after xxx seconds without data\n"
            "--debug           (-D) verbose debug mode\n"
            "--daemonize       (-B) operate in background\n"
            "--uid=xxx         (-u) change user id\n"
            "--gid=xxx         (-g) change group id\n"
            "\n"
            "Report bugs and suggestions to "
            ), stdout);
    puts(PACKAGE_BUGREPORT ".\n");

    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    struct addrinfo hints, *res;
    struct event ev;
    struct event sigchld_ev;
    struct event sigterm_ev;
    int on;
    int listen_fd;
    int fodder;
    int option_index;

#ifdef HAVE_SETLOCALE
    setlocale(LC_ALL, "");
#endif
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    if (argc <= 1) {
        usage();
    }

#ifndef SAVE_DESCRIPTORS
    openlog("sharedanced", LOG_PID, syslog_facility);
#endif

    timeout.tv_sec = (time_t) DEFAULT_TIMEOUT;
    timeout.tv_usec = 0U;

    while ((fodder = getopt_long(argc, argv, GETOPT_OPTIONS, long_options,
                                 &option_index)) != -1) {
        switch (fodder) {
        case 'h': {
            usage();
        }
        case 'B': {
            daemonize = 1;
            break;
        }
        case 'D': {
            debug = 1;
            break;
        }
        case 'i': {
            if ((listen_address_s = strdup(optarg)) == NULL) {
                die_mem();
            }
            break;
        }
        case 'p': {
            if ((port_s = strdup(optarg)) == NULL) {
                die_mem();
            }
            break;
        }
        case 'b': {
            if ((backlog = atoi(optarg)) < 1) {
                logfile(LOG_ERR, _("Invalid backlog"));
                return 1;
            }
            break;
        }
        case 'r': {
            if ((read_chunk_size = (size_t) strtoul(optarg, NULL, 10))
                < (size_t) 1U) {
                logfile(LOG_ERR, _("Invalid read chunk size"));
                return 1;
            }
            break;
        }
        case 'R': {
            if ((max_read_size = (size_t) strtoul(optarg, NULL, 10))
                < (size_t) 10U) {
                logfile(LOG_ERR, _("Invalid max read size"));
                return 1;
            }
            break;
        }
        case 'e': {
            if ((expiration = (time_t) strtoul(optarg, NULL, 10))
                < (time_t) 1) {
                logfile(LOG_ERR, _("Invalid expiration time"));
                return 1;
            }
            break;
        }
        case 't': {
            if ((timeout.tv_sec = (time_t) strtoul(optarg, NULL, 10))
                < (time_t) 1) {
                logfile(LOG_ERR, _("Invalid timeout"));
                return 1;
            }
            break;
        }
        case 'd': {
            if ((storage_dir = strdup(optarg)) == NULL) {
                die_mem();
            }
            break;
        }
        case 'g': {
            const char *nptr;
            char *endptr;

            nptr = optarg;
            endptr = NULL;
            gid = (gid_t) strtoul(nptr, &endptr, 10);
            if (!nptr || !*nptr || !endptr || *endptr) {
                logfile(LOG_ERR, _("Illegal GID - Must be a number\n"));
                return 1;
            }
            break;
        }
        case 'u': {
            const char *nptr;
            char *endptr;

            nptr = optarg;
            endptr = NULL;
            uid = (uid_t) strtoul(nptr, &endptr, 10);
            if (!nptr || !*nptr || !endptr || *endptr) {
                logfile(LOG_ERR, _("Illegal UID - Must be a number"));
                return 1;
            }
            break;
        }
        default: {
            usage();
        }
        }
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
#ifdef __OpenBSD__
    if (listen_address_s == NULL) {
        hints.ai_family = AF_INET;
    }
#endif
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_addr = NULL;
    if ((on = getaddrinfo(listen_address_s, port_s, &hints, &res)) != 0 ||
        (res->ai_family != AF_INET && res->ai_family != AF_INET6)) {
        logfile(LOG_ERR, _("Unable to create the listening socket: [%s]"),
                gai_strerror(on));
        return 1;
    }
    on = 1;
    if ((listen_fd = socket(res->ai_family, SOCK_STREAM, IPPROTO_TCP)) == -1 ||
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
                   (char *) &on, sizeof on) != 0) {
        logfile(LOG_ERR, _("Unable to create the socket: [%s]"),
                strerror(errno));
        freeaddrinfo(res);
        return 1;
    }
#if defined(IPPROTO_IPV6) && defined(IPV6_V6ONLY)
    on = 0;
    (void) setsockopt(listen_fd, IPPROTO_IPV6, IPV6_V6ONLY,
                      (char *) &on, sizeof on);
#endif
    if (bind(listen_fd, (struct sockaddr *) res->ai_addr,
             (socklen_t) res->ai_addrlen) != 0 ||
        listen(listen_fd, backlog) != 0) {
        logfile(LOG_ERR, _("Unable to bind the socket: [%s]"),
                strerror(errno));
        freeaddrinfo(res);
        return 1;
    }
    freeaddrinfo(res);

    tzset();
    dodaemonize();
    if (chroot(storage_dir) != 0 || chdir("/") != 0) {
        logfile(LOG_ERR, _("Unable to chroot to [%s]: [%s]"), storage_dir,
                strerror(errno));
        return 1;
    }
    changeuidgid();
    (void) umask((mode_t) 0);
    clearargs(argc, argv);

    if ((expire_pid = fork()) == (pid_t) -1) {
        logfile(LOG_ERR, _("Unable to fork: [%s]"), strerror(errno));
        (void) close(listen_fd);
        return 1;
    }
    if (expire_pid == (pid_t) 0) {
        (void) close(listen_fd);
        set_progname("sharedanced [CLEANUP]");
        expire();
        _exit(0);
    }
    set_progname("sharedanced [SERVER]");
    event_init();
    signal_set(&sigchld_ev, SIGCHLD, sigchld, &ev);
    signal_add(&sigchld_ev, NULL);
    signal_set(&sigterm_ev, SIGTERM, sigterm, &ev);
    signal_add(&sigterm_ev, NULL);
    event_set(&ev, listen_fd, EV_READ | EV_PERSIST, new_client, &ev);
    event_add(&ev, NULL);
    event_dispatch();
    (void) close(listen_fd);
    (void) kill(expire_pid, SIGTERM);

#ifndef SAVE_DESCRIPTORS
    closelog();
#endif

    return 0;
}

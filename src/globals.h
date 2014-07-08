#ifndef __GLOBALS_H__
#define __GLOBALS_H__ 1

#ifdef DEFINE_GLOBALS
# define GLOBAL0(A) A
# define GLOBAL(A, B) A = B
#else
# define GLOBAL0(A) extern A
# define GLOBAL(A, B) extern A
#endif

GLOBAL0(signed char no_syslog);
GLOBAL0(signed char daemonize);
GLOBAL0(signed char debug);
GLOBAL(int syslog_facility, DEFAULT_FACILITY);
GLOBAL0(pid_t expire_pid);
GLOBAL0(char *listen_address_s);
GLOBAL(const char *port_s, DEFAULT_PORT_S);
GLOBAL(int backlog, DEFAULT_BACKLOG);
GLOBAL(const char *storage_dir, DEFAULT_STORAGE_DIR);
GLOBAL(size_t read_chunk_size, DEFAULT_READ_CHUNK_SIZE);
GLOBAL(size_t max_read_size, DEFAULT_MAX_READ_SIZE);
GLOBAL(time_t expiration, DEFAULT_EXPIRATION);
GLOBAL0(struct timeval timeout);
GLOBAL0(uid_t uid);
GLOBAL0(gid_t gid);

#endif

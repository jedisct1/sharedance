#include <config.h>
#include "sharedanced.h"
#include "log.h"
#include "expire.h"

#ifdef WITH_DMALLOC
# include <dmalloc.h>
#endif

static int cleanup_dir(void)
{
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    time_t now;

    (void) time(&now);
    if ((dir = opendir(".")) == NULL) {
        logfile(LOG_WARNING, _("Unable to opendir : [%s]"),
                strerror(errno));
        return -1;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (stat(entry->d_name, &st) != 0 || !S_ISREG(st.st_mode)) {
            continue;
        }
        if (difftime(now, st.st_mtime) > expiration) {
            logfile(LOG_DEBUG, _("[%s] expired - removing"), entry->d_name);
            unlink(entry->d_name);
        }
    }
    (void) closedir(dir);

    return 0;
}

static void check_parent(void)
{
    pid_t ppid;
    
    if ((ppid = getppid()) < (pid_t) 2 || kill(ppid, 0) != 0) {
        exit(EXIT_SUCCESS);
    }
}

int expire(void)
{
    nice(EXPIRE_NICENESS);
    for (;;) {
        check_parent();
        logfile(LOG_DEBUG, _("Pruning [%s] directory"), storage_dir);
        cleanup_dir();
        sleep(EXPIRE_PAUSE);
    }
}

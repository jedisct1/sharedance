#include <config.h>
#include "sharedanced.h"
#ifdef HAVE_SETLOCALE
# include <locale.h>
#endif
#include "log.h"

#ifdef WITH_DMALLOC
# include <dmalloc.h>
#endif

void fd_nonblock(const int fd)
{
    while (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) != 0 &&
           (errno == EINTR || errno == EAGAIN));
}

int safe_write(const int fd, const void *buf_, size_t count)
{
    ssize_t written;
    register const char *buf = buf_;

    while (count > (size_t) 0U) {
        for (;;) {
            if ((written = write(fd, buf, count)) <= (ssize_t) 0) {
                if (errno == EAGAIN) {
                    sleep(1);
                } else if (errno != EINTR) {
                    return -1;
                }
                continue;
            }
            break;
        }
        buf += written;
        count -= written;
    }
    return 0;
}

static void client_disconnect(Client * const client)
{
    if (client->client_fd != -1) {
        logfile(LOG_DEBUG, _("client_disconnect: closing fd #%d"),
                client->client_fd);
        (void) close(client->client_fd);
        client->client_fd = -1;
    }
    if (client->read_mapped_zone != NULL) {
        (void) munmap(client->read_mapped_zone,
                      client->read_mapped_zone_length);
        client->read_mapped_zone = NULL;
        client->read_mapped_zone_length = (size_t) 0U;
    }
    if (client->read_fd != -1) {
        (void) close(client->read_fd);
        client->read_fd = -1;
    }
    if (client->write_fd != -1) {
        (void) close(client->write_fd);
        client->write_fd = -1;
    }
    if (client->returncode_bufev != NULL) {
        bufferevent_free(client->returncode_bufev);
        client->returncode_bufev = NULL;
    }
    free(client->read_buf);
    client->read_buf = NULL;
    free(client);
}

static void returncode_bufferev_read_cb(struct bufferevent *bufferev,
                                        void *client_)
{
    (void) bufferev;
    (void) client_;
    logfile(LOG_DEBUG, _("returncode_bufferev_read_cb"));
}

static void returncode_bufferev_write_cb(struct bufferevent *bufferev,
                                         void *client_)
{
    Client * const client = client_;

    (void) bufferev;
    logfile(LOG_DEBUG, _("Return code successfully written for fd #%d"),
            client->client_fd);
    client_disconnect(client);
}

static void returncode_bufferev_error_cb(struct bufferevent * const bufferev,
                                         short what, void *client_)
{
    Client * const client = client_;

    (void) bufferev;
    logfile(LOG_DEBUG, _("Error %d when sending return code to fd #%d"),
            (int) what, client->client_fd);
    client_disconnect(client);
}

static int validate_key(const unsigned char *buf, size_t size)
{
    if (size <= (size_t) 0U) {
        return -2;
    }
    do {
        size--;
        if (buf[size] < 32U || buf[size] > 126U ||
            buf[size] == '.' || buf[size] == '/') {
            return -1;
        }
    } while (size > (size_t) 0U);

    return 0;
}

static int client_process_store(Client * const client)
{
    char *store_file;
    char *store_file_tmp;
    char *store_file_pnt;
    char *store_file_tmp_pnt;
    size_t sizeof_store_file;
    size_t sizeof_store_file_tmp;

    logfile(LOG_DEBUG, _("client_process_store, fd #%d"), client->client_fd);
    if (client->offset_read_buf < (size_t) 10U) {
        return -1;
    }
    sizeof_store_file = (sizeof "/") - (size_t) 1U +
        client->key_len + (size_t) 1U;
    if ((store_file = ALLOCA(sizeof_store_file)) == NULL) {
        logfile(LOG_ERR, _("Out of stack for ALLOCA"));
        return -1;
    }
    store_file_pnt = store_file;
    *store_file_pnt++ = '/';
    if (validate_key(client->read_buf + (size_t) 9U, client->key_len) != 0) {
        ALLOCA_FREE(store_file);
        logfile(LOG_WARNING, _("Invalid key name"));
        return -1;
    }
    memcpy(store_file_pnt, client->read_buf + (size_t) 9U, client->key_len);
    store_file_pnt += client->key_len;
    *store_file_pnt = 0;

    sizeof_store_file_tmp = (sizeof "/") - (size_t) 1U +
        sizeof STOREFILE_TMP_PREFIX - (size_t) 1U +
        client->key_len + (size_t) 1U;
    if ((store_file_tmp = ALLOCA(sizeof_store_file_tmp)) == NULL) {
        logfile(LOG_ERR, _("Out of stack for ALLOCA"));
        ALLOCA_FREE(store_file);
        return -1;
    }
    store_file_tmp_pnt = store_file_tmp;
    *store_file_tmp_pnt++ = '/';
    memcpy(store_file_tmp_pnt, STOREFILE_TMP_PREFIX,
           sizeof STOREFILE_TMP_PREFIX - (size_t) 1U);
    store_file_tmp_pnt += sizeof STOREFILE_TMP_PREFIX - (size_t) 1U;
    memcpy(store_file_tmp_pnt,
           client->read_buf + (size_t) 9U, client->key_len);
    store_file_tmp_pnt += client->key_len;
    *store_file_tmp_pnt = 0;

    logfile(LOG_DEBUG, _("Creating [%s]"), store_file_tmp);
    if ((client->write_fd =
         open(store_file_tmp, O_CREAT | O_NOFOLLOW | O_WRONLY | O_TRUNC,
              (mode_t) 0600)) == -1) {
        logfile(LOG_WARNING, _("Unable to create [%s]: [%s]"), store_file,
                strerror(errno));
        ALLOCA_FREE(store_file);
        ALLOCA_FREE(store_file_tmp);
        return -1;
    }
    logfile(LOG_DEBUG, _("[%s] = fd #%d"), store_file, client->write_fd);
    if (safe_write(client->write_fd,
                   client->read_buf + (size_t) 9U + client->key_len,
                   client->data_len) < 0) {
        logfile(LOG_WARNING, _("Write error: [%s]"), strerror(errno));
    }
    (void) close(client->write_fd);
    client->write_fd = -1;
    logfile(LOG_DEBUG,
            _("Renaming [%s] to [%s]"), store_file_tmp, store_file);
    if (rename(store_file_tmp, store_file) != 0) {
        logfile(LOG_WARNING, _("Unable to rename [%s] to [%s]: [%s]"),
                store_file_tmp, store_file, strerror(errno));
        (void) unlink(store_file_tmp);
        ALLOCA_FREE(store_file);
        ALLOCA_FREE(store_file_tmp);
        return -1;
    }
    ALLOCA_FREE(store_file);
    ALLOCA_FREE(store_file_tmp);
    if ((client->returncode_bufev =
         bufferevent_new(client->client_fd,
                         returncode_bufferev_read_cb,
                         returncode_bufferev_write_cb,
                         returncode_bufferev_error_cb,
                         client)) == NULL) {
        logfile(LOG_WARNING, _("Unable to create a bufferevent for fd #%d"),
                client->client_fd);
    }
    (void) bufferevent_write(client->returncode_bufev, (void *) RETURNCODE_OK,
                             sizeof RETURNCODE_OK - (size_t) 1U);

    return 0;
}

static int client_process_fetch(Client * const client)
{
    char *store_file;
    char *store_file_pnt;
    size_t sizeof_store_file;
    struct stat st;

    logfile(LOG_DEBUG, _("client_process_fetch, fd #%d"), client->client_fd);
    if (client->offset_read_buf < (size_t) 6U) {
        return -1;
    }
    sizeof_store_file = (sizeof "/") - (size_t) 1U +
        client->key_len + (size_t) 1U;
    if ((store_file = ALLOCA(sizeof_store_file)) == NULL) {
        logfile(LOG_WARNING, _("Out of stack for ALLOCA"));
        return -1;
    }
    store_file_pnt = store_file;
    *store_file_pnt++ = '/';
    if (validate_key(client->read_buf + (size_t) 5U, client->key_len) != 0) {
        ALLOCA_FREE(store_file);
        logfile(LOG_WARNING, _("Invalid key name"));
        return -1;
    }
    memcpy(store_file_pnt, client->read_buf + (size_t) 5U, client->key_len);
    store_file_pnt += client->key_len;
    *store_file_pnt = 0;
    logfile(LOG_DEBUG, _("Reading [%s]"), store_file);
    if ((client->read_fd = open(store_file, O_RDONLY)) == -1) {
        logfile(LOG_DEBUG, _("Unable to fetch [%s]: [%s]"), store_file,
                strerror(errno));
        ALLOCA_FREE(store_file);
        return -1;
    }
    logfile(LOG_DEBUG, _("[%s] = fd #%d"), store_file, client->read_fd);
    ALLOCA_FREE(store_file);
    if (fstat(client->read_fd, &st) != 0 || st.st_size <= (off_t) 0) {        
        logfile(LOG_DEBUG, _("Unable to stat a previous key (%s) : [%s]"),
                store_file, strerror(errno));
        (void) close(client->read_fd);
        client->read_fd = -1;
        ALLOCA_FREE(store_file);
        return -1;
    }
    if ((client->read_mapped_zone = mmap(NULL, st.st_size, PROT_READ,
                                         MAP_SHARED, client->read_fd,
                                         (off_t) 0)) == NULL) {
        logfile(LOG_WARNING, _("Unable to map in memory: [%s]"),
                strerror(errno));
        (void) close(client->read_fd);
        client->read_fd = -1;
        ALLOCA_FREE(store_file);
        return -1;
    }
    client->read_mapped_zone_length = st.st_size;
    if ((client->returncode_bufev =
         bufferevent_new(client->client_fd,
                         returncode_bufferev_read_cb,
                         returncode_bufferev_write_cb,
                         returncode_bufferev_error_cb,
                         client)) == NULL) {
        logfile(LOG_WARNING, _("Unable to create a bufferevent for fd #%d"),
                client->client_fd);
    }
    (void) bufferevent_write(client->returncode_bufev,
                             client->read_mapped_zone,
                             client->read_mapped_zone_length);

    return 0;
}

static int client_process_delete(Client * const client)
{
    char *store_file;
    char *store_file_pnt;
    size_t sizeof_store_file;

    logfile(LOG_DEBUG, _("client_process_delete, fd #%d"), client->client_fd);
    if (client->offset_read_buf < (size_t) 6U) {
        return -1;
    }
    sizeof_store_file = (sizeof "/") - (size_t) 1U +
        client->key_len + (size_t) 1U;
    if ((store_file = ALLOCA(sizeof_store_file)) == NULL) {
        logfile(LOG_WARNING, _("Out of stack for ALLOCA"));
        return -1;
    }
    store_file_pnt = store_file;
    *store_file_pnt++ = '/';
    if (validate_key(client->read_buf + (size_t) 5U, client->key_len) != 0) {
        ALLOCA_FREE(store_file);
        logfile(LOG_WARNING, _("Invalid key name"));
        return -1;
    }
    memcpy(store_file_pnt, client->read_buf + (size_t) 5U, client->key_len);
    store_file_pnt += client->key_len;
    *store_file_pnt = 0;
    logfile(LOG_DEBUG, _("Deleting [%s]"), store_file);
    if (unlink(store_file) != 0) {
        logfile(LOG_INFO, _("Unable to delete [%s]: [%s]"), store_file,
                strerror(errno));
        ALLOCA_FREE(store_file);
        return -1;
    }
    if ((client->returncode_bufev =
         bufferevent_new(client->client_fd,
                         returncode_bufferev_read_cb,
                         returncode_bufferev_write_cb,
                         returncode_bufferev_error_cb,
                         client)) == NULL) {
        logfile(LOG_WARNING, _("Unable to create a bufferevent for fd #%d"),
                client->client_fd);
    }
    (void) bufferevent_write(client->returncode_bufev, (void *) RETURNCODE_OK,
                             sizeof RETURNCODE_OK - (size_t) 1U);

    return 0;
}

static void client_read(const int client_fd, short event, void *client_)
{
    Client * const client = client_;
    size_t needed_size;
    ssize_t readen;

    if (event == EV_TIMEOUT) {
        logfile(LOG_DEBUG, _("Timeout on descriptor #%d"), client->client_fd);
        client_disconnect(client);
        return;
    }
    needed_size = client->offset_read_buf + read_chunk_size;
    if (client->sizeof_read_buf < needed_size) {
        unsigned char *tmp_buf;

        if ((tmp_buf = realloc(client->read_buf,
                               needed_size)) == NULL) {
            logfile(LOG_ERR, _("Out of memory for client read buffers"));
            client_disconnect(client);
            return;
        }
        client->read_buf = tmp_buf;
        client->sizeof_read_buf = needed_size;
    }
    readen = read
        (client->client_fd,
         client->read_buf + client->offset_read_buf,
         client->sizeof_read_buf - client->offset_read_buf);
    if (readen <= (ssize_t) 0) {
        if (errno == EINTR || errno == EAGAIN) {
            logfile(LOG_DEBUG, _("Interrupted read for fd #%d: [%s]"),
                    client_fd, strerror(errno));
            event_add(&client->ev_client_read, &timeout);
            return;
        }
        if (readen == (ssize_t) 0) {
            logfile(LOG_DEBUG, _("Descriptor #%d has disconnected"),
                    client->client_fd);
        } else {
            logfile(LOG_DEBUG,
                    _("Descriptor #%d has disconnected with error [%s]"),
                    strerror(errno));
        }
        client_disconnect(client);
        return;
    }
    client->offset_read_buf += readen;
    logfile(LOG_DEBUG, _("Activity on client #%d (%d) offset=%lu"),
            client->client_fd, event,
            (unsigned int) client->offset_read_buf);
    if (client->client_command == CC_UNDEF) {
        if (client->read_buf[0] == CC_FETCH &&
            client->offset_read_buf > 5) {
            client->key_len =
                client->read_buf[1] << 24 | client->read_buf[2] << 16 |
                client->read_buf[3] << 8 | client->read_buf[4];
            client->total_len = (size_t) 1U + (size_t) 4U + client->key_len;
            client->client_command = CC_FETCH;
        } else if (client->read_buf[0] == CC_DELETE &&
            client->offset_read_buf > 5) {
            client->key_len =
                client->read_buf[1] << 24 | client->read_buf[2] << 16 |
                client->read_buf[3] << 8 | client->read_buf[4];
            client->total_len = (size_t) 1U + (size_t) 4U + client->key_len;
            client->client_command = CC_DELETE;
        } else if (client->read_buf[0] == CC_STORE &&
            client->offset_read_buf > 9) {
            client->key_len =
                client->read_buf[1] << 24 | client->read_buf[2] << 16 |
                client->read_buf[3] << 8 | client->read_buf[4];
            client->data_len =
                client->read_buf[5] << 24 | client->read_buf[6] << 16 |
                client->read_buf[7] << 8 | client->read_buf[8];
            client->total_len = (size_t) 1U + (size_t) 4U + (size_t) 4U +
                client->key_len + client->data_len;
            client->client_command = CC_STORE;
        } else if (client->offset_read_buf > 0 &&
                   client->read_buf[0] != CC_STORE &&
                   client->read_buf[0] != CC_FETCH) {
            logfile(LOG_WARNING, _("Unknown command [%d], good bye."),
                    (int) client->read_buf[0]);
            client_disconnect(client);
            return;
        }
    }
    if (client->total_len > (size_t) 0U &&
        client->offset_read_buf == client->total_len) {
        if (client->client_command == CC_STORE) {
            if (client_process_store(client) < 0) {
                client_disconnect(client);
            }
            return;
        } else if (client->client_command == CC_DELETE) {
            if (client_process_delete(client) < 0) {
                client_disconnect(client);
            }
            return;
        } else if (client->client_command == CC_FETCH) {
            if (client_process_fetch(client) < 0) {
                client_disconnect(client);
            }
            return;
        } else {
            logfile(LOG_ERR, _("Unknown but ok command?"));
        }
        client_disconnect(client);
        return;
    }
    if (client->total_len > (size_t) 0U &&
        client->offset_read_buf > client->total_len) {
        logfile(LOG_WARNING,
                _("Request too long total len = %lu, now = %lu. Good bye."),
                (unsigned long) client->total_len,
                (unsigned long) client->offset_read_buf);
        client_disconnect(client);
        return;
    }
    if (client->offset_read_buf >= max_read_size) {
        logfile(LOG_WARNING,
                _("Request too long max = %lu, now = %lu. Good bye."),
                (unsigned long) max_read_size,
                (unsigned long) client->offset_read_buf);
        client_disconnect(client);
        return;
    }
    event_add(&client->ev_client_read, &timeout);
}

void new_client(const int listen_fd, short event, void *ev)
{
    Client *client;
    struct sockaddr_storage client_sa;
    socklen_t client_sa_len = sizeof client_sa;
    int client_fd;

    (void) ev;
    if ((event & EV_READ) == 0) {
        logfile(LOG_DEBUG, _("Wrong event %d received on fd #%d"),
                (int) event, listen_fd);
        return;
    }
    memset(&client_sa, 0, sizeof client_sa);
    if ((client_fd = accept(listen_fd, (struct sockaddr *) &client_sa,
                            &client_sa_len)) < 0) {
        logfile(LOG_DEBUG, _("accept(): [%s]"), strerror(errno));
        return;
    }
    if (client_sa_len <= (socklen_t) 0U) {
        (void) close(client_fd);
        return;
    }
    fd_nonblock(client_fd);
    if ((client = malloc(sizeof *client)) == NULL) {
        (void) close(client_fd);
        logfile(LOG_ERR, _("Out of memory to accept a new client"));
        return;
    }
    client->client_fd = client_fd;
    client->read_buf = NULL;
    client->sizeof_read_buf = (size_t) 0U;
    client->offset_read_buf = (size_t) 0U;
    client->client_command = CC_UNDEF;
    client->key_len = (size_t) 0U;
    client->data_len = (size_t) 0U;
    client->total_len = (size_t) 0U;
    client->write_fd = -1;
    client->read_fd = -1;
    client->read_mapped_zone = NULL;
    client->read_mapped_zone_length = (size_t) 0U;
    client->returncode_bufev = NULL;
    event_set(&client->ev_client_read, client_fd, EV_READ,
              client_read, client);
    event_add(&client->ev_client_read, &timeout);
    logfile(LOG_DEBUG, _("New client on fd #%d"), client_fd);
}

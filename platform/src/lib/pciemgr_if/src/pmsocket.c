/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "pciesvc_impl.h"

#define DEFAULT_PORT            50001
#define HOST_ENVAR              "PCIESVC_HOST"
#define SOCK_UN_ENVAR           "PCIESOCK_PATH"
#define SOCK_UN_PATH            "/var/run/pciesock"

static int
make_addr(char *host, int port, struct sockaddr_in *addr)
{
    struct hostent *he;

    he = gethostbyname(host);
    if (he == NULL) {
        return -1;
    }

    memset(addr, 0, sizeof(*addr));
    addr->sin_family = he->h_addrtype;
    addr->sin_port = htons(port);
    memcpy(&addr->sin_addr, he->h_addr, he->h_length);
    return 0;
}

static int
get_addr(const char *addrstr, struct sockaddr_in *a)
{
    char host[128], *env, *colon;
    int port;

    /* first check for given addr in arg str... */
    if (addrstr != NULL) {
        port = DEFAULT_PORT;
        strncpy(host, addrstr, sizeof(host));
        host[sizeof(host) - 1] = '\0';
        colon = strchr(host, ':');
        if (colon) {
            *colon = '\0';
            port = strtoul(colon + 1, NULL, 0);
        }
        return make_addr(host, port, a);
    }

    /* ...next check envariable... */
    env = getenv(HOST_ENVAR);
    if (env != NULL) {
        port = DEFAULT_PORT;
        strncpy(host, env, sizeof(host));
        host[sizeof(host) - 1] = '\0';
        colon = strchr(host, ':');
        if (colon) {
            *colon = '\0';
            port = strtoul(colon + 1, NULL, 0);
        }
        return make_addr(host, port, a);
    }

    /* ...provide default */
    return make_addr("localhost", DEFAULT_PORT, a);
}

static char *
socket_un_default_path(void)
{
    static char path[256];
    char *env, *user __attribute__((unused));

    env = getenv(SOCK_UN_ENVAR);
    if (env != NULL) {
        strncpy(path, env, sizeof(path));
        return path;
    }

#ifdef __aarch64__
    /* aarch64 use "/var/run/pciesock" */
    strncpy(path, SOCK_UN_PATH, sizeof(path));
#else
    /* for x86_64 we use "/tmp/pciesock-$USER" */
    user = NULL;
    if (user == NULL) {
        user = getenv("SUDO_USER");
    }
    if (user == NULL) {
        user = getenv("USER");
    }
    snprintf(path, sizeof(path), "/tmp/pciesock-%s", user);
#endif
    return path;
}

static int
socket_un(const char *path, struct sockaddr_un *a)
{
    int s;

    s = socket(AF_UNIX, SOCK_STREAM, 0);
    if (s < 0) return -1;

    a->sun_family = AF_UNIX;
    strncpy(a->sun_path, path, sizeof(a->sun_path));
    return s;
}

static int
socket_in(const char *addrstr, struct sockaddr_in *a)
{
    int on = 1;
    int s;

    get_addr(addrstr, a);
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) return -1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    return s;
}

int
pmsocket(const char *addrstr, struct pmsockaddr *a)
{
    if (addrstr == NULL) {
        a->sz = sizeof(a->un);
        return socket_un(socket_un_default_path(), &a->un);
    }
    if (strchr(addrstr, '/') != NULL) {
        a->sz = sizeof(a->un);
        return socket_un(addrstr, &a->un);
    }

    a->sz = sizeof(a->in);
    return socket_in(addrstr, &a->in);
}

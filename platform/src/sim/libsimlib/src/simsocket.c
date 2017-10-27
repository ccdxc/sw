/*
 * Copyright (c) 2017, Pensando Systems Inc.
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

#include "simlib_impl.h"

static int
sim_make_addr(char *host, int port, struct sockaddr_in *addr)
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
sim_get_addr(const char *addrstr, struct sockaddr_in *a)
{
    char host[128], *env, *colon;
    int port;

    /* first check for given addr in arg str... */
    if (addrstr != NULL) {
        port = SIM_DEFAULT_PORT;
        strncpy(host, addrstr, sizeof(host));
        host[sizeof(host) - 1] = '\0';
        colon = strchr(host, ':');
        if (colon) {
            *colon = '\0';
            port = strtoul(colon + 1, NULL, 0);
        }
        return sim_make_addr(host, port, a);
    }

    /* ...next check SIM_HOST envariable... */
    env = getenv("SIM_HOST");
    if (env != NULL) {
        port = SIM_DEFAULT_PORT;
        strncpy(host, env, sizeof(host));
        host[sizeof(host) - 1] = '\0';
        colon = strchr(host, ':');
        if (colon) {
            *colon = '\0';
            port = strtoul(colon + 1, NULL, 0);
        }
        return sim_make_addr(host, port, a);
    }

    /* ...provide default */
    return sim_make_addr("localhost", SIM_DEFAULT_PORT, a);
}

static char *
socket_un_default_path(void)
{
    static char path[256];
    char *user;

    user = getenv("USER");
    snprintf(path, sizeof(path), "/tmp/simsock-%s", user);
    return path;
}

static int
sim_socket_un(const char *path, struct sockaddr_un *a)
{
    int s;

    s = socket(AF_UNIX, SOCK_STREAM, 0);
    if (s < 0) return -1;

    a->sun_family = AF_UNIX;
    strncpy(a->sun_path, path, sizeof(a->sun_path));
    return s;
}

static int
sim_socket_in(const char *addrstr, struct sockaddr_in *a)
{
    int on = 1;
    int s;

    sim_get_addr(addrstr, a);
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) return -1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    return s;
}

int
sim_socket(const char *addrstr, struct simsockaddr *a)
{
    if (addrstr == NULL) {
        a->sz = sizeof(a->un);
        return sim_socket_un(socket_un_default_path(), &a->un);
    }
    if (strchr(addrstr, '/') != NULL) {
        a->sz = sizeof(a->un);
        return sim_socket_un(addrstr, &a->un);
    }

    a->sz = sizeof(a->in);
    return sim_socket_in(addrstr, &a->in);
}

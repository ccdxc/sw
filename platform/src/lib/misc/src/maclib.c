/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "src/lib/misc/include/maclib.h"

char *
mac_to_str(const mac_t *m)
{
    static char buf[32];
    return mac_to_str_r(m, buf, sizeof(buf));
}

char *
mac_to_str_r(const mac_t *m, char *buf, const size_t bufsz)
{
    snprintf(buf, bufsz, "%02x:%02x:%02x:%02x:%02x:%02x",
             m->buf[0], m->buf[1], m->buf[2],
             m->buf[3], m->buf[4], m->buf[5]);
    return buf;
}

int 
mac_from_str(mac_t *m, const char *str)
{
    int i, mw[6];

    if (sscanf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
               &mw[0], &mw[1], &mw[2], &mw[3], &mw[4], &mw[5]) == 6) {
        for (i = 0; i < 6; i++) {
            m->buf[i] = mw[i];
        }
        return 0;
    }
    return -1;
}

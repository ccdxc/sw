/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "maclib.h"


char *
mac_to_str(const mac_t *m, char *str, const uint64_t strsz)
{
    uint8_t *p = (uint8_t *)m;
    int r = snprintf(str, strsz, "%02x:%02x:%02x:%02x:%02x:%02x",
                     p[5], p[4], p[3], p[2], p[1], p[0]);
    return (r > 0) ? str : "";
}

int
mac_from_str(mac_t *m, const char *str)
{
    uint8_t *p = (uint8_t *)m;
    unsigned int w[6];
    int r = sscanf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
                   &w[5], &w[4], &w[3], &w[2], &w[1], &w[0]);
    if (r == 6) {
        for (int i = 0; i < 6; i++) {
            p[i] = w[i];
        }
        return 0;
    }
    return -1;
}

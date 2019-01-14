
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include "dtls.h"

static FILE *logfp;

static int
logf_init(void)
{
    static bool init_done;
    static bool enabled;

    if (!init_done) {
        init_done = true;
        enabled = (getenv("CAPVIEWLOG") != NULL);
        if (enabled) {
            static const char path[] = "/tmp/capviewlog";
            logfp = fopen(path, "a");
            if (logfp == NULL) {
                throw std::system_error(errno, std::system_category(), path);
            }
            setlinebuf(logfp);
        }
    }
    return enabled ? 0 : -1;
}

void
logf(const char *fmt, ...)
{
    va_list ap;

    if (logf_init() == 0) {
        va_start(ap, fmt);
        vfprintf(logfp, fmt, ap);
        va_end(ap);
    }
}

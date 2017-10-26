/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <string.h>
#include <ctype.h>
#include "misc.h"

int
strtoargv(char *s, char *argv[], const int maxac)
{
    int argc = 0;

    while (*s && argc < maxac-1) {
        if (isspace(*s)) { s++; continue; }
        argv[argc++] = s;               // save beginning of arg
        while (*s && !isspace(*s)) s++; // find end of arg
        if (*s) *s++ = '\0';            // null-terminate arg
    }
    argv[argc] = NULL;                  // null-terminate list
    return (argc);
}

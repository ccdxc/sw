/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <cinttypes>
#include <sys/types.h>
#include <sys/param.h>

#include "nic/sdk/platform/misc/include/misc.h"
#include "nic/sdk/platform/pal/include/pal.h"
#include "cmd.h"

extern cmd_t __start_cmdtab[];
extern cmd_t __stop_cmdtab[];

static cmd_t *
cmd_lookup(const char *name)
{
    for (cmd_t *c = __start_cmdtab; c < __stop_cmdtab; c++) {
        if (strcmp(name, c->name) == 0) return c;
    }
    return NULL;
}

static int
cmd_cmp(const void *a1, const void *a2)
{
    const cmd_t *c1 = (cmd_t *)a1;
    const cmd_t *c2 = (cmd_t *)a2;

    return strcmp(c1->name, c2->name);
}

static void
help(int argc, char *argv[])
{
    if (argc <= 1) {
        const size_t ncmds = __stop_cmdtab - __start_cmdtab;
        const size_t cmdsz = sizeof(cmd_t);
        cmd_t *cmdtab = __start_cmdtab;

        /* sort cmds alphabetically for help display */
        qsort(cmdtab, ncmds, cmdsz, cmd_cmp);

        printf("Usage: pcieutil <cmd>[args...]\n"
               "Available commands:\n");
        for (cmd_t *c = cmdtab; c < &cmdtab[ncmds]; c++) {
            printf("    %-*s %s\n", 16, c->name, c->desc);
        }
        return;
    }

    cmd_t *c = cmd_lookup(argv[1]);
    if (c == NULL) {
        printf("%s: not found\n", argv[1]);
        return;
    }

    printf("Usage: %s\n", c->usage);
}
CMDFUNC(help,
"display command help",
"help [<cmd>]\n"
"    no args    display all commands and description\n"
"    <cmd>      usage for <cmd>\n");

int
cmd_run(int argc, char *argv[])
{
    cmd_t *c;

    if (argc <= 1) {
        c = cmd_lookup("help");
        c->func(argc, argv);
        return 0;
    }

    c = cmd_lookup(argv[1]);
    if (c == NULL) {
        printf("%s: not found\n", argv[1]);
        return 1;
    }

    argc--;
    argv++;
    c->func(argc, argv);
    return 0;
}

int
cmd_runstr(char *cmdstr)
{
    char *argv[16];
    int argc;

    argc = strtoargv(cmdstr, argv, sizeof(argv) / sizeof(argv[0]));
    return cmd_run(argc, argv);
}

int
main(int argc, char *argv[])
{
    return cmd_run(argc, argv);
}

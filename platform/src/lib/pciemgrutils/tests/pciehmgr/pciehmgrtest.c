/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#ifdef BUILD_OS_Darwin
#include <editline/readline.h>
#endif
#ifdef BUILD_OS_Linux
#include <readline/readline.h>
#endif

#include "pciehost.h"

typedef struct pciehmgrtest_s {
    pciehdev_t *current_dev;
} pciehmgrtest_t;

static pciehmgrtest_t pciehmgrtest;
static int verbose_flag;

extern char *strncpy0(char *d, const char *s, const size_t n);

static void
usage(void)
{
    fprintf(stderr, "Usage: pciehmgrtest [-v]\n");
}

static void
verbose(const char *fmt, ...)
{
    va_list arg;

    if (verbose_flag) {
        va_start(arg, fmt);
        vprintf(fmt, arg);
        va_end(arg);
    }
}

static pciehmgrtest_t *
pciehmgrtest_get(void)
{
    return &pciehmgrtest;
}

/******************************************************************/

static void
cmd_initialize(int argc, char *argv[])
{
    pciehmgr_initialize();
}

static void
cmd_finalize(int argc, char *argv[])
{
    pciehmgr_finalize();
}

static void
cmd_construct(int argc, char *argv[])
{
    pciehmgrtest_t *pmgrt = pciehmgrtest_get();
    pciehdev_t *pdev;
    pciehdev_params_t devparams;
    int opt;
    char *type;

    memset(&devparams, 0, sizeof(devparams));

    if (argc < 2) {
        printf("usage: construct <name> [-t type (default enet)]\n");
        return;
    }
    strncpy0(devparams.name, argv[1], sizeof(devparams.name));
    argc--;
    argv++;

    type = "enet";
    optreset = 1;
    optind = 1;
    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
        case 't':
            type = optarg;
            break;
        default:
            printf("bad argument: %c\n", opt);
            return;
        }
    }
    if (type == NULL) {
        printf("missing type\n");
        return;
    }
    pdev = pciehdev_construct(type, &devparams);
    if (pdev == NULL) {
        printf("construct with type %s failed\n", type);
        return;
    }
    pmgrt->current_dev = pdev;
}

static void
cmd_add(int argc, char *argv[])
{
    pciehmgrtest_t *pmgrt = pciehmgrtest_get();

    if (pmgrt->current_dev) {
        pciehdev_add(pmgrt->current_dev);
        pmgrt->current_dev = NULL;
    }
}

static void cmd_help(int argc, char *argv[]);

typedef struct cmd_s {
    const char *name;
    void (*f)(int argc, char *argv[]);
    const char *desc;
    const char *helpstr;
} cmd_t;

static cmd_t cmdtab[] = {
#define CMDENT(name, desc, helpstr) \
    { #name, cmd_##name, desc, helpstr }
    CMDENT(initialize, "initialize device tree (empty)", ""),
    CMDENT(construct, "construct new dev", "<name> -t <type>"),
    CMDENT(add, "add constructed dev to topology", ""),
    CMDENT(help, "display command help", "[cmd]"),
    CMDENT(finalize, "finalize device tree (load in hw)", ""),
    { NULL, NULL }
};

cmd_t *
cmd_lookup(const char *name)
{
    cmd_t *c;

    for (c = cmdtab; c->name; c++) {
        if (strcmp(c->name, name) == 0) {
            return c;
        }
    }
    return NULL;
}

void
cmd_help(int argc, char *argv[])
{
    cmd_t *c;

    if (argc > 1) {
        c = cmd_lookup(argv[1]);
        if (c == NULL) {
            printf("command not found\n");
            return;
        }
        printf("%s %s\n", c->name, c->helpstr);
        return;
    }

    for (c = cmdtab; c->name; c++) {
        printf("%-20s %s\n", c->name, c->desc);
    }
}

/******************************************************************/

static void
process(int argc, char *argv[])
{
    cmd_t *c = cmd_lookup(argv[0]);
    if (c == NULL) {
        printf("command not found\n");
        return;
    }
    c->f(argc, argv);
}

int
main(int argc, char *argv[])
{
    int opt;
    char *line, *prompt, *av[16];
    int ac;
    extern int strtoargv(char *, char **, const int);
    pciehmgr_params_t mgrparams;

    memset(&mgrparams, 0, sizeof(mgrparams));
    mgrparams.flags |= PCIEHMGR_PARAMF_INITHW;

    while ((opt = getopt(argc, argv, "nv")) != -1) {
        switch (opt) {
        case 'n':
            mgrparams.flags &= ~PCIEHMGR_PARAMF_INITHW;
            break;
        case 'v':
            verbose_flag = 1;
            break;
        case '?':
        default:
            usage();
            exit(1);
        }
    }

    if (pciehmgr_open(&mgrparams) < 0) {
        printf("pciehmgr_open failed\n");
        exit(1);
    }

    prompt = "pciehmgr> ";
    while ((line = readline(prompt)) != NULL) {
        if (line[0] != '\0') {
            ac = strtoargv(line, av, sizeof(av) / sizeof(av[0]));
            process(ac, av);
        }
        free(line);
    }

    pciehmgr_close();
    exit(0);
}

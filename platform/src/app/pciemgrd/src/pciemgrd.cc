/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/param.h>
#ifdef USE_READLINE
#ifdef BUILD_OS_Darwin
#include <editline/readline.h>
#endif
#ifdef BUILD_OS_Linux
#include <readline/readline.h>
#endif
#endif

#include "pci_ids.h"
#include "misc.h"
#include "bdf.h"
#include "cfgspace.h"
#include "pciehost.h"
#include "pciehdevices.h"

typedef struct pciemgrenv_s {
    pciehdev_t *current_dev;
} pciemgrenv_t;

static pciemgrenv_t pciemgrenv;
static int verbose_flag;
static int exit_request;

static void
usage(void)
{
    fprintf(stderr,
"Usage: pciemgrd [-Fnv][-P gen<G>x<W>][-s subdeviceid]\n"
"    -F                 no fake bios scan\n"
"    -n                 no initializing hw\n"
"    -P gen<G>x<W>      spec devices as pcie gen <G>, lane width <W>\n"
"    -s subdeviceid     default subsystem device id\n"
"    -v                 verbose\n");
}

static void verbose(const char *fmt, ...)
    __attribute__((format (printf, 1, 2)));
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

static pciemgrenv_t *
pciemgrenv_get(void)
{
    return &pciemgrenv;
}

static int
parse_linkspec(const char *s, u_int8_t *genp, u_int8_t *widthp)
{
    int gen, width;

    if (sscanf(s, "gen%dx%d", &gen, &width) != 2) {
        return 0;
    }
    if (gen < 1 || gen > 4) {
        return 0;
    }
    if (width < 1 || width > 32) {
        return 0;
    }
    if (width & (width - 1)) {
        return 0;
    }
    *genp = gen;
    *widthp = width;
    return 1;
}

static pciehdev_t *
construct(char *namearg, const char *type, pciehdevice_resources_t *pres)
{
    char lname[32], *name;
    pciehdev_t *pdev;

    if (strcmp(type, "enet") == 0) {
        if (namearg == NULL) {
            static int enet_instance;
            snprintf(lname, sizeof(lname), "enet%d", enet_instance++);
            name = lname;
        } else {
            name = namearg;
        }
        pdev = pciehdev_enet_new(name, pres);
        if (pdev == NULL) {
            printf("pciehdev_enet_new failed\n");
            return NULL;
        }
    } else if (strcmp(type, "enetvf") == 0) {
        pdev = NULL; // XXX
    } else if (strcmp(type, "nvme") == 0) {
        if (namearg == NULL) {
            static int nvme_instance;
            snprintf(lname, sizeof(lname), "nvme%d", nvme_instance++);
            name = lname;
        } else {
            name = namearg;
        }
        pdev = pciehdev_nvme_new(name, pres);
        if (pdev == NULL) {
            printf("pciehdev_nvme_new failed\n");
            return NULL;
        }
    } else if (strcmp(type, "debug") == 0) {
        if (namearg == NULL) {
            static int debug_instance;
            snprintf(lname, sizeof(lname), "debug%d", debug_instance++);
            name = lname;
        } else {
            name = namearg;
        }
        pdev = pciehdev_debug_new(name, pres);
        if (pdev == NULL) {
            printf("pciehdev_debug_new failed\n");
            return NULL;
        }
    } else {
        printf("%s: unknown dev type\n", type);
        return NULL;
    }

    return pdev;
}

/******************************************************************/

static void
getopt_reset(int optind_arg, int optreset_arg)
{
#ifdef BUILD_OS_Linux
    optind = 0;
#endif
#ifdef BUILD_OS_Darwin
    optind = optind_arg;
    optreset = optreset_arg;
#endif
}

static void
cmd_initialize(int argc, char *argv[])
{
    pciehdev_initialize();
}

static void
cmd_finalize(int argc, char *argv[])
{
    pciehdev_finalize();
}

static void
cmd_commit(int argc, char *argv[])
{
}

static void
cmd_add(int argc, char *argv[])
{
    pciehdev_t *pdev;
    pciehdevice_resources_t r;
    int opt;
    char *type, *name;

    if (argc < 2) {
        printf("usage: add <type> [<args>]\n");
        return;
    }
    type = argv[1];

    memset(&r, 0, sizeof(r));
    r.nintrs = 4;
    name = NULL;

    getopt_reset(4, 2);
    while ((opt = getopt(argc, argv, "i:n:")) != -1) {
        switch (opt) {
        case 'i':
            r.nintrs = strtoul(optarg, NULL, 0);
            break;
        case 'N':
            name = optarg;
            break;
        default:
            printf("bad argument: %c\n", opt);
            return;
        }
    }
    pdev = construct(name, type, &r);
    if (pdev == NULL) {
        return;
    }
    pciehdev_add(pdev);
    printf("%s\n", pciehdev_get_name(pdev));
}

static void
cmd_addfn(int argc, char *argv[])
{
    pciehdev_t *pdev, *pfn;
    pciehdevice_resources_t r;
    int opt, nfn;
    char *vdev, *type;

    if (argc < 3) {
        printf("usage: addfn <vdev> <type> [<args>]\n");
        return;
    }
    vdev = argv[1];
    type = argv[2];

    memset(&r, 0, sizeof(r));
    r.nintrs = 4;
    r.fnn = 1;
    nfn = 1;

    getopt_reset(3, 1);
    while ((opt = getopt(argc, argv, "i:n:")) != -1) {
        switch (opt) {
        case 'i':
            r.nintrs = strtoul(optarg, NULL, 0);
            break;
        case 'n':
            nfn = strtoul(optarg, NULL, 0);
            break;
        default:
            printf("bad argument: %c\n", opt);
            return;
        }
    }
    pdev = pciehdev_get_by_name(vdev);
    if (pdev == NULL) {
        printf("%s: no device\n", vdev);
        return;
    }

    for (int fnc = 0; fnc < nfn; fnc++) {
        char name[32];

        snprintf(name, sizeof(name),
                 "%s.%d", pciehdev_get_name(pdev), fnc + 1);
        pfn = construct(name, type, &r);
        if (pfn == NULL) {
            return;
        }
        pciehdev_addfn(pdev, pfn, fnc);

        // XXX increment resources r
    }
}

static void
show_header(void)
{
    printf("%-10s %-7s %-10s %-10s %-10s\n",
           "NAME", "BDF", "PARENT", "PEER", "CHILD");
}

static void
show1(pciehdev_t *p)
{
    pciehdev_t *parent = pciehdev_get_parent(p);
    pciehdev_t *peer   = pciehdev_get_peer(p);
    pciehdev_t *child  = pciehdev_get_child(p);

    printf("%-10s %-7s %-10s %-10s %-10s\n",
           pciehdev_get_name(p),
           bdf_to_str(pciehdev_get_bdf(p)),
           parent ? pciehdev_get_name(parent) : "",
           peer ? pciehdev_get_name(peer) : "",
           child ? pciehdev_get_name(child) : "");
}

static void
foreach_dev(int d, pciehdev_t *pdev, void (*f)(pciehdev_t *pdev))
{
    pciehdev_t *peer, *child;

    f(pdev);

    for (peer = pciehdev_get_peer(pdev);
         peer;
         peer = pciehdev_get_peer(peer)) {
        f(peer);
    }
    for (peer = pdev; peer; peer = pciehdev_get_peer(peer)) {
        child = pciehdev_get_child(peer);
        if (child) {
            foreach_dev(d + 1, child, f);
        }
    }
}

static void
cmd_show(int argc, char *argv[])
{
    pciehdev_t *pdev = pciehdev_get_root();
    if (pdev != NULL) {
        show_header();
        foreach_dev(0, pdev, show1);
    } else {
        printf("no devices\n");
    }
}

static void
cmd_bars(int argc, char *argv[])
{
    int opt;

    if (argc < 2) {
        printf("usage: bars [<args>] <vdev> ...\n");
        return;
    }

    getopt_reset(1, 1);
    while ((opt = getopt(argc, argv, "")) != -1) {
    }

    for (int i = optind; i < argc; i++) {
        pciehdev_t *pdev = pciehdev_get_by_name(argv[i]);
        if (pdev == NULL) {
            printf("%s: no device\n", argv[i]);
            continue;
        }
        printf("%s bars:\n", pciehdev_get_name(pdev));

        pciehbars_t *pbars = pciehdev_get_bars(pdev);
        pciehbar_t *b;

        printf("%-3s %-5s %-6s %-4s %-5s\n",
               "IDX", "TYPE", "SIZE", "NREG", "FLAGS");
        for (b = pciehbars_get_first(pbars);
             b;
             b = pciehbars_get_next(pbars, b)) {
            printf("%3d %-5s 0x%04x %4d %s%s%s\n",
                   b->cfgidx,
                   b->type == PCIEHBARTYPE_MEM   ? "mem" :
                   b->type == PCIEHBARTYPE_MEM64 ? "mem64" :
                   b->type == PCIEHBARTYPE_IO    ? "io" : "????",
                   b->size,
                   b->nregs,
                   b->msix_tbl ? "msix_tbl " : "",
                   b->msix_pba ? "msix_pba " : "",
                   b->rombar   ? "rombar " : "");
        }
    }
}

static void
cmd_cfg(int argc, char *argv[])
{
    int opt, mask;

    if (argc < 2) {
        printf("usage: cfg [-m] <vdev> ...\n"
               "    -m          show mask (default is data)\n");
        return;
    }

    mask = 0;
    getopt_reset(1, 1);
    while ((opt = getopt(argc, argv, "m")) != -1) {
        switch (opt) {
        case 'm':
            mask = 1;
            break;
        default:
            printf("bad argument: %c\n", opt);
            return;
        }
    }

    for (int i = optind; i < argc; i++) {
        pciehdev_t *pdev = pciehdev_get_by_name(argv[i]);
        if (pdev == NULL) {
            printf("%s: no device\n", argv[i]);
            continue;
        }
        printf("%s config%s:\n", pciehdev_get_name(pdev), mask ? " mask" : "");

        pciehcfg_t *pcfg = pciehdev_get_cfg(pdev);
        cfgspace_t cfgspace, *cs = &cfgspace;
        pciehcfg_get_cfgspace(pcfg, cs);
        const int sz = cfgspace_size(cs);
        const u_int8_t *p = mask ? cs->msk : cs->cur;

        for (int offset = 0; offset < sz; offset += 16) {
            char buf[80];
            const char *s = hex_format(buf, sizeof(buf), &p[offset], 16);
            printf("%03x: %s\n", offset, s);
        }
    }
}

static void
cmd_exit(int argc, char *argv[])
{
    exit_request = 1;
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
    CMDENT(add, "add a new dev to topology", "<type> [<args>]"),
    CMDENT(addfn,
           "add multifunctions to function 0",
           "<vdev> <type> [<args>]"),
    CMDENT(help, "display command help", "[cmd]"),
    CMDENT(finalize, "finalize device tree (load in hw)", ""),
    CMDENT(commit, "commit topology (release host)", ""),
    CMDENT(show, "show device info", ""),
    CMDENT(bars, "show bar info for device", "[<args>] <vdev> ..."),
    CMDENT(cfg, "show cfg info for device", "[-m] <vdev> ..."),
    CMDENT(exit, "exit program", ""),
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

static char *
readline(const char *prompt)
{
    char line[80];

    fputs(prompt, stdout);
    fflush(stdout);
    if (fgets(line, sizeof(line), stdin) != NULL) {
        return strdup(line);
    }
    return NULL;
}

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
    char *line, prompt[32], *av[16];
    int ac;
    pciehdev_params_t p;

    memset(&p, 0, sizeof(p));
    p.inithw = 1;
    p.fake_bios_scan = 1;
    p.subdeviceid = PCI_SUBDEVICE_ID_PENSANDO_NAPLES100;

    while ((opt = getopt(argc, argv, "FnP:s:v")) != -1) {
        switch (opt) {
        case 'F':
            p.fake_bios_scan = 0;
            break;
        case 'n':
            p.inithw = 0;
            break;
        case 'P':
            if (!parse_linkspec(optarg, &p.cap_gen, &p.cap_width)) {
                printf("bad pcie spec: want gen%%dx%%d, got %s\n", optarg);
                exit(1);
            }
            break;
        case 's':
            p.subdeviceid = strtoul(optarg, NULL, 0);
            break;
        case 'v':
            verbose_flag = 1;
            verbose("verbose enabled\n");
            break;
        case '?':
        default:
            usage();
            exit(1);
        }
    }

    if (pciehdev_open(&p) < 0) {
        printf("pciehdev_open failed\n");
        exit(1);
    }

    if (pciehdev_initialize() < 0) {
        printf("pciehdev_initialize failed\n");
        exit(1);
    }

    strncpy0(prompt, "pciemgr> ", sizeof(prompt));
    while (!exit_request && (line = readline(prompt)) != NULL) {
        if (line[0] != '\0') {
            ac = strtoargv(line, av, sizeof(av) / sizeof(av[0]));
            process(ac, av);
        }
        free(line);
    }

    pciehdev_close();
    exit(0);
    if (0) pciemgrenv_get();
}

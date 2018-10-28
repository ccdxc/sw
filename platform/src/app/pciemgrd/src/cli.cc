/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <setjmp.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <editline/readline.h>

#include "pci_ids.h"
#include "misc.h"
#include "bdf.h"
#include "pal.h"
#include "cfgspace.h"
#include "pciehdevices.h"
#include "pciemgrutils.h"
#include "pciehw.h"
#include "pciehw_dev.h"
#include "pcieport.h"
#include "pciemgrd_impl.hpp"

static pciehdev_t *
construct(char *namearg, const char *type, pciehdevice_resources_t *pres)
{
    char lname[32], *name;
    pciehdev_t *pdev;

    if (strcmp(type, "eth") == 0) {
        if (namearg == NULL) {
            static int eth_instance;
            snprintf(lname, sizeof(lname), "eth%d", eth_instance++);
            name = lname;
        } else {
            name = namearg;
        }
        pdev = pciehdev_eth_new(name, pres);
        if (pdev == NULL) {
            printf("pciehdev_eth_new failed\n");
            return NULL;
        }
    } else if (strcmp(type, "ethvf") == 0) {
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
    } else if (strcmp(type, "accel") == 0) {
        if (namearg == NULL) {
            static int accel_instance;
            snprintf(lname, sizeof(lname), "accel%d", accel_instance++);
            name = lname;
        } else {
            name = namearg;
        }
        pdev = pciehdev_accel_new(name, pres);
        if (pdev == NULL) {
            printf("pciehdev_accel_new failed\n");
            return NULL;
        }
    } else if (strcmp(type, "virtio") == 0) {
        if (namearg == NULL) {
            static int virtio_instance;
            snprintf(lname, sizeof(lname), "virtio%d", virtio_instance++);
            name = lname;
        } else {
            name = namearg;
        }
        pdev = pciehdev_virtio_new(name, pres);
        if (pdev == NULL) {
            printf("pciehdev_virtio_new failed\n");
            return NULL;
        }
    } else if (strcmp(type, "pciestress") == 0) {
        if (namearg == NULL) {
            static int pciestress_instance;
            snprintf(lname, sizeof(lname), "pciestress%d",
                     pciestress_instance++);
            name = lname;
        } else {
            name = namearg;
        }
        pdev = pciehdev_pciestress_new(name, pres);
        if (pdev == NULL) {
            printf("pciehdev_pciestress_new failed\n");
            return NULL;
        }
    } else if (strcmp(type, "rcdev") == 0) {
        if (namearg == NULL) {
            static int rcdev_instance;
            snprintf(lname, sizeof(lname), "rcdev%d", rcdev_instance++);
            name = lname;
        } else {
            name = namearg;
        }
        pdev = pciehdev_rcdev_new(name, pres);
        if (pdev == NULL) {
            printf("pciehdev_rcdev_new failed\n");
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
    pciemgrenv_t *pme = pciemgrenv_get();

    for (int port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->enabled_ports & (1 << port)) {
            pciehdev_initialize(port);
        }
    }
}

static void
cmd_finalize(int argc, char *argv[])
{
    pciemgrenv_t *pme = pciemgrenv_get();

    for (int port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->enabled_ports & (1 << port)) {
            pciehdev_finalize(port);
            pcieport_crs_off(port);
        }
    }
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
    int opt, bi;
    char *type, *name;

    if (argc < 2) {
        printf("usage: add <type> [<args>]\n");
        return;
    }
    type = argv[1];

    memset(&r, 0, sizeof(r));
    r.intrc = 4;
    r.devcmdpa = 0x13e000000;   /* XXX */
    r.devcmddbpa = r.devcmdpa + 0x1000; /* XXX */
    name = NULL;
    bi = 0;

    getopt_reset(4, 2);
    while ((opt = getopt(argc, argv, "b:B:c:C:d:D:I:L:i:n:p:P:")) != -1) {
        switch (opt) {
        case 'b':
            r.debugbar[bi].barpa = strtoull(optarg, NULL, 0);
            break;
        case 'B':
            r.debugbar[bi].barsz = strtoull(optarg, NULL, 0);
            bi++;       /* -B barsz comes last per bar */
            break;
        case 'c':
            r.cmbsz = strtoull(optarg, NULL, 0);
            break;
        case 'C':
            r.cmbpa = strtoull(optarg, NULL, 0);
            break;
        case 'd':
            r.devcmdpa = strtoull(optarg, NULL, 0);
            break;
        case 'D':
            r.devcmddbpa = strtoull(optarg, NULL, 0);
            break;
        case 'I':
            r.intrb = strtoul(optarg, NULL, 0);
            break;
        case 'L':
            r.lif = strtoul(optarg, NULL, 0);
            r.lif_valid = 1;
            break;
        case 'i':
            r.intrc = strtoul(optarg, NULL, 0);
            break;
        case 'N':
            name = optarg;
            break;
        case 'p':
            r.port = strtoul(optarg, NULL, 0);
            break;
        case 'P':
            r.npids = strtoul(optarg, NULL, 0);
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
    r.intrc = 4;
    r.fnn = 1;
    nfn = 1;

    getopt_reset(3, 1);
    while ((opt = getopt(argc, argv, "i:n:")) != -1) {
        switch (opt) {
        case 'i':
            r.intrc = strtoul(optarg, NULL, 0);
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
    printf("%-16s %-9s %-10s %-10s %-10s\n",
           "NAME", "P:BDF", "PARENT", "PEER", "CHILD");
}

static void
show1(pciehdev_t *p)
{
    pciehdev_t *parent = pciehdev_get_parent(p);
    pciehdev_t *peer   = pciehdev_get_peer(p);
    pciehdev_t *child  = pciehdev_get_child(p);

    printf("%-16s %d:%-7s %-10s %-10s %-10s\n",
           pciehdev_get_name(p),
           pciehdev_get_port(p),
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
    int port;

    show_header();
    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        pciehdev_t *pdev = pciehdev_get_root(port);

        if (pdev != NULL) {
            foreach_dev(0, pdev, show1);
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

static u_int64_t
timestamp(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000 + tv.tv_usec);
}

static int poll_enabled;

static void
polling_sighand(int s)
{
    poll_enabled = 0;
}

static void
cmd_poll(int argc, char *argv[])
{
    pciemgrenv_t *pme = pciemgrenv_get();
    sighandler_t osigint, osigterm, osigquit;
    useconds_t polltm_us = 10000;
    int opt, poll_port, poll_cnt, npolls;
    u_int64_t tm_start, tm_stop, tm_port;

    npolls = 0;
    poll_port = 1;
    poll_cnt = 0;
    getopt_reset(1, 1);
    while ((opt = getopt(argc, argv, "c:Pt:")) != -1) {
        switch (opt) {
        case 'c':
            poll_cnt = strtoul(optarg, NULL, 0);
            break;
        case 'P':
            poll_port = 0;
            break;
        case 't':
            polltm_us = strtoull(optarg, NULL, 0);
            break;
        }
    }

    osigint  = signal(SIGINT,  polling_sighand);
    osigterm = signal(SIGTERM, polling_sighand);
    osigquit = signal(SIGQUIT, polling_sighand);

    printf("Polling enabled every %dus (%d times), ^C to exit...\n",
           polltm_us, poll_cnt);
    poll_enabled = 1;
    while (poll_enabled && (poll_cnt == 0 || npolls < poll_cnt)) {
        tm_start = timestamp();
        if (poll_port) {
            for (int port = 0; port < PCIEPORT_NPORTS; port++) {
                if (pme->enabled_ports & (1 << port)) {
                    pcieport_poll(port);
                }
            }
        }
        tm_port = timestamp();
        pciehdev_poll();
        tm_stop = timestamp();

        if (tm_port - tm_start > 1000000) {
            printf("pcieport_poll: %ldus\n", tm_port - tm_start);
        }
        if (tm_stop - tm_port > 1000000) {
            printf("pciehdev_poll: %ldus\n", tm_stop - tm_port);
        }

        if (polltm_us) usleep(polltm_us);
        npolls++;
    }
    printf("Polling stopped\n");

    signal(SIGINT,  osigint);
    signal(SIGTERM, osigterm);
    signal(SIGQUIT, osigquit);
}

static int exit_request;

static void
cmd_exit(int argc, char *argv[])
{
    exit_request = 1;
}

static void
cmd_quit(int argc, char *argv[])
{
    cmd_exit(argc, argv);
}

static void cmd_help(int argc, char *argv[]);
static void cmd_dbg(int argc, char *argv[]);

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
    CMDENT(cfg, "show cfg info for device", "[-m] <vdev> ..."),
    CMDENT(poll, "poll for intrs", ""),
    CMDENT(dbg, "invoke debug interface", ""),
    CMDENT(exit, "exit program", ""),
    CMDENT(quit, "exit program", ""),
    { NULL, NULL }
};

static cmd_t *
cmd_lookup(cmd_t *cmdtab, const char *name)
{
    cmd_t *c;

    for (c = cmdtab; c->name; c++) {
        if (strcmp(c->name, name) == 0) {
            return c;
        }
    }
    return NULL;
}

static void
cmd_help(int argc, char *argv[])
{
    cmd_t *c;

    if (argc > 1) {
        c = cmd_lookup(cmdtab, argv[1]);
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

static void
dbg_pciehw(int argc, char *argv[])
{
    pciehw_dbg(argc, argv);
}

static void
dbg_pcieport(int argc, char *argv[])
{
    pcieport_dbg(argc, argv);
}

static cmd_t dbgtab[] = {
#define DBGENT(name, desc, helpstr) \
    { #name, dbg_##name, desc, helpstr }
    DBGENT(pciehw, "pciehw debug", ""),
    DBGENT(pcieport, "pcieport debug", ""),
    { NULL, NULL }
};

static void
cmd_dbg(int argc, char *argv[])
{
    cmd_t *c;

    if (argc < 2) {
        printf("Usage: dbg <subcmd>\n");
        return;
    }

    c = cmd_lookup(dbgtab, argv[1]);
    if (c == NULL) {
        printf("%s: %s not found\n", argv[0], argv[1]);
        return;
    }
    c->f(argc - 1, argv + 1);
}

/******************************************************************/

static void
process(int argc, char *argv[])
{
    cmd_t *c = cmd_lookup(cmdtab, argv[0]);
    if (c == NULL) {
        printf("command not found\n");
        return;
    }
    c->f(argc, argv);
}

static jmp_buf prompt_env;

static void
sighand(int s)
{
    longjmp(prompt_env, 1);
}

void
cli_loop(void)
{
    pciemgrenv_t *pme = pciemgrenv_get();
    char *line, prompt[32], *av[16];
    int port, ac;

    /*
     * In interactive mode, pre-initialize all the active ports
     * to save the user some typing.
     */
    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->enabled_ports & (1 << port)) {
            if (pciehdev_initialize(port) < 0) {
                printf("pciehdev_initialize failed\n");
                exit(1);
            }
        }
    }

    if (setjmp(prompt_env) == 0) {
        signal(SIGINT, sighand);
        signal(SIGQUIT, sighand);
    }
    strncpy0(prompt, "pciemgr> ", sizeof(prompt));
    while (!exit_request && (line = readline(prompt)) != NULL) {
        if (line[0] != '\0') {
            add_history(line);
            ac = strtoargv(line, av, sizeof(av) / sizeof(av[0]));
            if (ac) {
                process(ac, av);
            }
        }
        free(line);
    }
}

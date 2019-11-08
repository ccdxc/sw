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
#include <cinttypes>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <editline/readline.h>

#include "platform/misc/include/misc.h"
#include "platform/misc/include/bdf.h"
#include "platform/pal/include/pal.h"
#include "platform/cfgspace/include/cfgspace.h"
#include "platform/pciehdevices/include/pciehdevices.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "platform/evutils/include/evutils.h"
#include "platform/pciemgr/include/pciemgr.h"
#include "platform/pcieport/include/pcieport.h"
#include "nic/sdk/platform/pciemgrd/pciemgrd_impl.hpp"
#include "pciemgrd_impl.hpp"

/******************************************************************/

static void
getopt_reset(int optind_arg, int optreset_arg)
{
    optind = 0;

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

static int
parse_common_pres_opt(int opt, pciehdevice_resources_t *pres)
{
    pciehdev_res_t *pfres = &pres->pfres;

    switch (opt) {
    case 'c':
        pfres->cmbsz = strtoull_ext(optarg);
        break;
    case 'C':
        pfres->cmbpa = strtoull(optarg, NULL, 0);
        break;
    case 'i':
        pfres->intrc = strtoul(optarg, NULL, 0);
        break;
    case 'I':
        pfres->intrb = strtoul(optarg, NULL, 0);
        break;
    case 'l':
        pfres->lifc = strtoul(optarg, NULL, 0);
        break;
    case 'L':
        pfres->lifb = strtoul(optarg, NULL, 0);
        if (pfres->lifc == 0) pfres->lifc = 1;
        break;
    case 'N':
        strncpy0(pfres->name, optarg, sizeof(pfres->name));
        break;
    case 'p':
        pfres->port = strtoul(optarg, NULL, 0);
        break;
    case 'P':
        pfres->npids = strtoul(optarg, NULL, 0);
        break;
    case 'r':
        pfres->romsz = strtoull(optarg, NULL, 0);
        break;
    case 'R':
        pfres->rompa = strtoull(optarg, NULL, 0);
        break;
    case 'v':
        pfres->totalvfs = strtoull(optarg, NULL, 0);
        if (pfres->totalvfs > 0) {
            pciehdev_res_t *vfres = &pres->vfres;
            vfres->is_vf = 1;
            /* XXX just some sample values */
            if (pfres->lifc) {
                vfres->lifb = pfres->lifb + pfres->lifc;
                vfres->lifc = pfres->lifc;
            }
            if (pfres->intrc) {
                vfres->intrb = pfres->intrb + pfres->intrc;
                vfres->intrc = pfres->intrc;
                vfres->intrdmask = pfres->intrdmask;
            }
            vfres->eth.devregspa = pfres->eth.devregspa + 0x1000;
            vfres->eth.devregssz = pfres->eth.devregssz;
            vfres->eth.devregs_stride = vfres->eth.devregssz;
        }
        break;
    default:
        return - 1;
    }
    return 0;
}

static void
add_accel(int argc, char *argv[])
{
    static int instance;
    pciehdevice_resources_t pres;
    int opt;

    memset(&pres, 0, sizeof(pres));
    pres.type = PCIEHDEVICE_ACCEL;
    snprintf(pres.pfres.name, sizeof(pres.pfres.name), "accel%d", instance);
    pres.pfres.intrc = 4;
    pres.pfres.intrdmask = 1;
    pres.pfres.accel.devregspa = 0x13e000000; /* XXX */
    pres.pfres.accel.devregssz = 0x1000;      /* XXX */
    pres.pfres.cmbprefetch = 1;

    getopt_reset(4, 2);
    while ((opt = getopt(argc, argv, "c:C:i:I:l:L:N:p:r:R:")) != -1) {
        switch (opt) {
        default:
            if (parse_common_pres_opt(opt, &pres) < 0) {
                fprintf(stderr, "bad argument: %c\n", opt);
                return;
            }
            break;
        }
    }

    pciehdev_t *pdev = pciehdevice_new(&pres);
    if (pdev == NULL) {
        fprintf(stderr, "pciehdevice_new failed\n");
        return;
    }
    pciehdev_add(pdev);
    printf("%s\n", pciehdev_get_name(pdev));
    instance++;
}

static void
add_debug(int argc, char *argv[])
{
    static int instance;
    pciehdevice_resources_t pres;
    int opt, bi;

    memset(&pres, 0, sizeof(pres));
    pres.type = PCIEHDEVICE_DEBUG;
    snprintf(pres.pfres.name, sizeof(pres.pfres.name), "debug%d", instance);
    pres.pfres.intrc = 4;
    pres.pfres.intrdmask = 1;
    bi = 0;

    getopt_reset(4, 2);
    while ((opt = getopt(argc, argv, "b:B:fI:i:l:L:N:p:r:R:V:D:C:")) != -1) {
        switch (opt) {
        case 'b':
            pres.pfres.debug.bar[bi].barpa = strtoull_ext(optarg);
            break;
        case 'B':
            pres.pfres.debug.bar[bi].barsz = strtoull_ext(optarg);
            bi++;       /* -B barsz comes last per bar */
            break;
        case 'f':
            pres.pfres.debug.bar[bi].prefetch = 1;
            break;
        case 'V':
            pres.pfres.debug.vendorid = strtoul(optarg, NULL, 0);
            break;
        case 'D':
            pres.pfres.debug.deviceid = strtoul(optarg, NULL, 0);
            break;
        case 'C':
            pres.pfres.debug.classcode = strtoul(optarg, NULL, 0);
            break;
        default:
            if (parse_common_pres_opt(opt, &pres) < 0) {
                fprintf(stderr, "bad argument: %c\n", opt);
                return;
            }
            break;
        }
    }

    pciehdev_t *pdev = pciehdevice_new(&pres);
    if (pdev == NULL) {
        fprintf(stderr, "pciehdevice_new failed\n");
        return;
    }
    pciehdev_add(pdev);
    printf("%s\n", pciehdev_get_name(pdev));
    instance++;
}

static void
add_eth_class(const pciehdevice_type_t type, const char *types,
              int argc, char *argv[])
{
    static int instance;
    pciehdevice_resources_t pres;
    pciehdev_t *pdev;
    int opt;

    memset(&pres, 0, sizeof(pres));
    pres.type = type;
    snprintf(pres.pfres.name, sizeof(pres.pfres.name),
             "%s%d", types, instance);
    pres.pfres.intrc = 4;
    pres.pfres.intrdmask = 1;
    pres.pfres.eth.devregspa = 0x13d000000; /* XXX */
    pres.pfres.eth.devregssz = 0x1000;      /* XXX */
    pres.pfres.eth.macaddr = 0x001122334455 + instance; /* XXX */

    getopt_reset(4, 2);
    while ((opt = getopt(argc, argv, "c:C:I:i:l:L:N:p:P:r:R:v:")) != -1) {
        switch (opt) {
        default:
            if (parse_common_pres_opt(opt, &pres) < 0) {
                fprintf(stderr, "bad argument: %c\n", opt);
                return;
            }
            break;
        }
    }

    pdev = pciehdevice_new(&pres);
    if (pdev == NULL) {
        return;
    }
    pciehdev_add(pdev);
    printf("%s\n", pciehdev_get_name(pdev));
    instance++;
}

static void
add_eth(int argc, char *argv[])
{
    add_eth_class(PCIEHDEVICE_ETH, "eth", argc, argv);
}

static void
add_mgmteth(int argc, char *argv[])
{
    add_eth_class(PCIEHDEVICE_MGMTETH, "mgmteth", argc, argv);
}

static void
add_nvme(int argc, char *argv[])
{
    static int instance;
    pciehdevice_resources_t pres;
    pciehdev_res_t *pfres = &pres.pfres;
    int opt;

    memset(&pres, 0, sizeof(pres));
    pres.type = PCIEHDEVICE_NVME;
    snprintf(pfres->name, sizeof(pfres->name), "nvme%d", instance);
    pfres->intrc = 4;
    pfres->intrdmask = 0;
    pfres->nvme.regspa = 0x13d000000; /* XXX */
    pfres->nvme.regssz = 0x1000;
    pfres->nvme.qidc = 64;

    getopt_reset(4, 2);
    while ((opt = getopt(argc, argv, "C:i:I:l:L:N:p:r:R:v:")) != -1) {
        switch (opt) {
        case 'C':
            pfres->nvme.classcode = strtoul(optarg, NULL, 0);
            break;
        case 'v':
            pfres->totalvfs = strtoull(optarg, NULL, 0);
            if (pfres->totalvfs > 0) {
                pciehdev_res_t *vfres = &pres.vfres;
                vfres->is_vf = 1;
                /* XXX just some sample values */
                if (pfres->lifc) {
                    vfres->lifb = pfres->lifb + pfres->lifc;
                    vfres->lifc = pfres->lifc;
                }
                if (pfres->intrc) {
                    vfres->intrb = pfres->intrb + pfres->intrc;
                    vfres->intrc = pfres->intrc;
                    vfres->intrdmask = pfres->intrdmask;
                }
                vfres->nvme.regspa = pfres->nvme.regspa + 0x1000;
                vfres->nvme.regssz = pfres->nvme.regssz;
                vfres->nvme.regs_stride = vfres->nvme.regssz;
                vfres->nvme.qidc = pfres->nvme.qidc;
                vfres->nvme.classcode = pfres->nvme.classcode;
            }
            break;
        default:
            if (parse_common_pres_opt(opt, &pres) < 0) {
                fprintf(stderr, "bad argument: %c\n", opt);
                return;
            }
            break;
        }
    }

    pciehdev_t *pdev = pciehdevice_new(&pres);
    if (pdev == NULL) {
        fprintf(stderr, "pciehdevice_new failed\n");
        return;
    }
    pciehdev_add(pdev);
    printf("%s\n", pciehdev_get_name(pdev));
    instance++;
}

static void
add_pciestress(int argc, char *argv[])
{
    static int instance;
    pciehdevice_resources_t pres;
    int opt;

    memset(&pres, 0, sizeof(pres));
    pres.type = PCIEHDEVICE_PCIESTRESS;
    snprintf(pres.pfres.name, sizeof(pres.pfres.name),
             "pciestress%d", instance);
    pres.pfres.intrc = 4;
    pres.pfres.intrdmask = 1;

    getopt_reset(4, 2);
    while ((opt = getopt(argc, argv, "b:B:fI:i:l:L:N:p:r:R:")) != -1) {
        switch (opt) {
        default:
            if (parse_common_pres_opt(opt, &pres) < 0) {
                fprintf(stderr, "bad argument: %c\n", opt);
                return;
            }
            break;
        }
    }

    pciehdev_t *pdev = pciehdevice_new(&pres);
    if (pdev == NULL) {
        fprintf(stderr, "pciehdevice_new failed\n");
        return;
    }
    pciehdev_add(pdev);
    printf("%s\n", pciehdev_get_name(pdev));
    instance++;
}

static void
add_rcdev(int argc, char *argv[])
{
    static int instance;
    pciehdevice_resources_t pres;
    int opt;

    memset(&pres, 0, sizeof(pres));
    pres.type = PCIEHDEVICE_RCDEV;
    snprintf(pres.pfres.name, sizeof(pres.pfres.name), "rcdev%d", instance);
    pres.pfres.intrc = 4;
    pres.pfres.intrdmask = 0;

    getopt_reset(4, 2);
    while ((opt = getopt(argc, argv, "b:B:fI:i:l:L:N:p:r:R:")) != -1) {
        switch (opt) {
        default:
            if (parse_common_pres_opt(opt, &pres) < 0) {
                fprintf(stderr, "bad argument: %c\n", opt);
                return;
            }
            break;
        }
    }

    pciehdev_t *pdev = pciehdevice_new(&pres);
    if (pdev == NULL) {
        fprintf(stderr, "pciehdevice_new failed\n");
        return;
    }
    pciehdev_add(pdev);
    printf("%s\n", pciehdev_get_name(pdev));
    instance++;
}

static void
add_virtio(int argc, char *argv[])
{
    static int instance;
    pciehdevice_resources_t pres;
    int opt;

    memset(&pres, 0, sizeof(pres));
    pres.type = PCIEHDEVICE_VIRTIO;
    snprintf(pres.pfres.name, sizeof(pres.pfres.name), "virtio%d", instance);
    pres.pfres.intrc = 4;
    pres.pfres.intrdmask = 0;
    pres.pfres.virtio.regspa = 0x13e000000; /* XXX */
    pres.pfres.virtio.regssz = 0x1000;      /* XXX */

    getopt_reset(4, 2);
    while ((opt = getopt(argc, argv, "i:I:l:L:N:p:r:R:D:")) != -1) {
        switch (opt) {
        case 'D':
            pres.pfres.virtio.deviceid = strtoul(optarg, NULL, 0);
            break;
        default:
            if (parse_common_pres_opt(opt, &pres) < 0) {
                fprintf(stderr, "bad argument: %c\n", opt);
                return;
            }
            break;
        }
    }

    pciehdev_t *pdev = pciehdevice_new(&pres);
    if (pdev == NULL) {
        fprintf(stderr, "pciehdevice_new failed\n");
        return;
    }
    pciehdev_add(pdev);
    printf("%s\n", pciehdev_get_name(pdev));
    instance++;
}

typedef struct addent_s {
    const char *name;
    void (*addfn)(int argc, char *argv[]);
} addent_t;

static addent_t addtab[] = {
#define ADDENT(name) \
    { #name, add_##name }
    ADDENT(accel),
    ADDENT(debug),
    ADDENT(eth),
    ADDENT(mgmteth),
    ADDENT(nvme),
    ADDENT(pciestress),
    ADDENT(rcdev),
    ADDENT(virtio),
    { NULL, NULL }
};

static addent_t *
add_lookup(const char *name)
{
    addent_t *e;

    for (e = addtab; e->name; e++) {
        if (strcmp(e->name, name) == 0) {
            return e;
        }
    }
    return NULL;
}

static void
cmd_add(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: add <type> [<args>]\n");
        return;
    }
    const char *type = argv[1];
    addent_t *addent = add_lookup(type);
    if (addent == NULL) {
        fprintf(stderr, "type %s not found\n", type);
        return;
    }
    argc--;
    argv++;
    addent->addfn(argc, argv);
}

static void
cmd_addfn(int argc, char *argv[])
{
#if 0
    // XXX this isn't the way to do this anymore

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
    r.pfres.intrc = 4;
    /* r.pfres.fnn = 1; */
    nfn = 1;

    getopt_reset(3, 1);
    while ((opt = getopt(argc, argv, "i:n:")) != -1) {
        switch (opt) {
        case 'n':
            nfn = strtoul(optarg, NULL, 0);
            break;
        default:
            fprintf(stderr, "bad argument: %c\n", opt);
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
        strncpy0(r.pfres.name, name, sizeof(r.pfres.name));
        r.type = PCIEHDEVICE_ETH; // XXX
        pfn = pciehdevice_new(&r);
        if (pfn == NULL) {
            return;
        }
        pciehdev_addfn(pdev, pfn, fnc);

        // XXX increment resources r
    }
#endif
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
            fprintf(stderr, "bad argument: %c\n", opt);
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
    sighandler_t osigint, osigterm;
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

    /* Poll mode for these events. */
    pciehw_notify_poll_init();
    pciehw_indirect_poll_init();

    osigint  = signal(SIGINT,  polling_sighand);
    osigterm = signal(SIGTERM, polling_sighand);

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
}

static void
cmd_powerdown(int argc, char *argv[])
{
    int opt, port;

    port = 0;
    getopt_reset(1, 1);
    while ((opt = getopt(argc, argv, "p")) != -1) {
        switch (opt) {
        case 'p':
            port = strtoul(optarg, NULL, 0);
            break;
        default:
            fprintf(stderr, "bad argument: %c\n", opt);
            break;
        }
    }

    int r = pcieport_powerdown(port);
    if (r < 0) {
        fprintf(stderr, "powerdown failed %d\n", r);
        return;
    }
}

static void
cmd_run(int argc, char *argv[])
{
    int opt;

    getopt_reset(1, 1);
    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
        }
    }

    intr_init();

    printf("Running, ^C to exit...\n");
    evutil_run(EV_DEFAULT);
    printf("Running stopped\n");
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

static void
cmd_upgrade_save(int argc, char *argv[])
{
    upgrade_state_save();
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
    CMDENT(show, "show device info", ""),
    CMDENT(cfg, "show cfg info for device", "[-m] <vdev> ..."),
    CMDENT(poll, "poll for intrs", ""),
    CMDENT(powerdown, "power down port", ""),
    CMDENT(run, "run and wait for intrs", ""),
    CMDENT(dbg, "invoke debug interface", ""),
    CMDENT(exit, "exit program", ""),
    CMDENT(quit, "exit program", ""),
    CMDENT(upgrade_save, "upgrade save state", ""),
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

int
cli_loop(pciemgrenv_t *pme)
{
    char *line, prompt[32], *av[16];
    int r, port, ac;

#ifdef __aarch64__
    // On arm let's log to the usual place,
    // even in interactive mode.
    logger_init();
#endif
    pciemgrd_params(pme);
    pciemgrd_sys_init(pme);
    if ((r = open_hostports()) < 0) {
        goto error_out;
    }
    if ((r = pciehdev_open(&pme->params)) < 0) {
        printf("pciehdev_open failed: %d\n", r);
        goto close_port_error_out;
    }

    /*
     * In interactive mode, pre-initialize all the active ports
     * to save the user some typing.
     */
    if (pme->params.initmode == FORCE_INIT) {
        for (port = 0; port < PCIEPORT_NPORTS; port++) {
            if (pme->enabled_ports & (1 << port)) {
                if ((r = pciehdev_initialize(port)) < 0) {
                    printf("pciehdev_initialize %d failed: %d\n", port, r);
                    goto close_dev_error_out;
                }
            }
        }
    }

    if (setjmp(prompt_env) == 0) {
        signal(SIGINT, sighand);
        signal(SIGTERM, sighand);
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

 close_dev_error_out:
    pciehdev_close();
 close_port_error_out:
    close_hostports();
 error_out:
    return r;
}

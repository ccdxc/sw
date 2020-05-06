/*
 * Copyright (c) 2017-2020, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <cinttypes>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>

#include "nic/sdk/platform/pal/include/pal.h"
#include "nic/sdk/platform/pcieport/include/portmap.h"
#include "nic/sdk/platform/pciemgrd/pciemgrd_impl.hpp"
#include "pciemgrd_impl.hpp"

static void
usage(void)
{
    fprintf(stderr,
"Usage: pciemgrd [-dFinrv][-b <first_bus_num>]\n"
"                [-I <inherit-mode>][-P gen<G>x<W>][-R 0|1]\n"
"                [-V subvendorid][-D subdeviceid]\n"
"    -b <first_bus_num> set first bus used to <first_bus_num>\n"
"    -d                 daemon mode\n"
"    -F                 no fake bios scan\n"
"    -i                 interactive mode\n"
"    -I <inherit-mode>  inherit or re-initialize hw/shmem\n"
"    -p hostH=portP,genGxW   host port spec (multiple ok)\n"
"    -p <portspec>      add portspec to ports, hostH=portP,genGxW\n"
"    -P gen<G>x<W>      spec devices as pcie gen <G>, lane width <W>\n"
"    -r                 restart after upgrade\n"
"    -R <0|1>           reboot_on_hostdn=<0|1>\n"
"    -V subvendorid     default subsystem vendor id\n"
"    -D subdeviceid     default subsystem device id\n");
}

static int
parse_portspec(const char *s, pcieport_spec_t *ps)
{
    int h, p, g, w;

    if (sscanf(s, "host%d=port%d,gen%dx%d", &h, &p, &g, &w) != 4) {
        return 0;
    }
    if (h < 0 || h >= 8) return 0;
    if (p < 0 || p >= 8) return 0;
    if (g < 1 || g > 4) return 0;
    if (w < 1 || w > 32) return 0;
    if (w & (w - 1)) return 0;

    memset(ps, 0, sizeof(*ps));
    ps->host = h;
    ps->port = p;
    ps->gen = g;
    ps->width = w;
    return 1;
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

int
main(int argc, char *argv[])
{
    pciemgrenv_t *pme = pciemgrenv_get();
    pciemgr_params_t *params = &pme->params;
    int r, opt, portmap_inited;

    pme->interactive = 1;
    pme->reboot_on_hostdn = pal_is_asic() ? 1 : 0;
#ifdef ASIC_ELBA
    /* XXX ELBA-TODO make elba interrupts work */
    pme->poll_dev = 1;
#endif
    pme->fifopri = 50;
    pme->poll_port = 1;
    pme->poll_tm = 500000; // 0.5s slow poll for non-essential events

    params->strict_crs = 1;
    params->initmode = INITMODE_NONE;

    /*
     * For aarch64 we can inherit a system in which we get restarted on
     * a running system (mostly for testing).
     * For x86_64 we want to FORCE_INIT to reinitialize hw/shmem on startup.
     *
     * On "real" ARM systems the upstream port bridge
     * is in hw and our first virtual device is bus 0 at 00:00.0.
     *
     * For simulation we want the virtual upstream port bridge
     * at 00:00.0 so our first virtual device is bus 1 at 01:00.0.
     */
#ifdef __aarch64__
    params->first_bus = 0;
#else
    params->first_bus = 1;
    params->fake_bios_scan = 1;         /* simulate bios scan to set bdf's */
#endif

    if (upgrade_in_progress()) {
        params->restart = 1;
        params->initmode = INHERIT_OK;
    }

    /*
     * Get the catalog defaults.
     * For testing, cmdline args can override below if desired.
     */
    pciemgrd_catalog_defaults(pme);

    portmap_inited = 0;
    while ((opt = getopt(argc, argv, "b:CdFGiI:p:P:V:D:rR:")) != -1) {
        switch (opt) {
        case 'b':
            params->first_bus = strtoul(optarg, NULL, 0);
            break;
        case 'C':
            params->compliance = 1;
            break;
        case 'd':
            pme->interactive = 0;
            break;
        case 'F':
            params->fake_bios_scan = 0;
            break;
        case 'G':
            pme->gold = 1;
            break;
        case 'i':
            pme->interactive = 1;
            break;
        case 'I':
            if (strcmp(optarg, "inherit_only") == 0) {
                params->initmode = INHERIT_ONLY;
            } else if (strcmp(optarg, "inherit_ok") == 0) {
                params->initmode = INHERIT_OK;
            } else if (strcmp(optarg, "force_init") == 0) {
                params->initmode = FORCE_INIT;
            } else {
                printf("bad -I arg: inherit_only|inherit_ok|force_init\n");
                exit(1);
            }
            break;
        case 'p':
            pcieport_spec_t ps;
            if (!parse_portspec(optarg, &ps)) {
                printf("bad port spec: want host%%d=port%%d,gen%%dx%%d\n");
                exit(1);
            }
            if (portmap_inited++ == 0) portmap_init();
            if (portmap_addhost(&ps) < 0) {
                printf("portmap_addhost failed\n");
                exit(1);
            }
            break;
        case 'P':
            if (!parse_linkspec(optarg,
                                &params->cap_gen, &params->cap_width)) {
                printf("bad pcie spec: want gen%%dx%%d, got %s\n", optarg);
                exit(1);
            }
            break;
        case 'r':
            params->restart = 1;
            params->initmode = INHERIT_OK;
            break;
        case 'R':
            pme->reboot_on_hostdn = strtoul(optarg, NULL, 0);
            break;
        case 'V':
            params->subvendorid = strtoul(optarg, NULL, 0);
            break;
        case 'D':
            params->subdeviceid = strtoul(optarg, NULL, 0);
            break;
        case '?':
        default:
            usage();
            exit(1);
        }
    }
    if (portmap_inited) {
        pme->enabled_ports = portmap_portmask();
    }

#ifdef PCIEMGRD_GOLD
    {
        int gold_loop(pciemgrenv_t *pme);

        // gold mode always daemon mode, force init
        pme->interactive = 0;
        if (params->initmode == INITMODE_NONE) {
            params->initmode = FORCE_INIT;
        }

        r = gold_loop(pme);
    }
#else
    if (params->initmode == INITMODE_NONE) {
        if (pme->interactive) {
            params->initmode = FORCE_INIT;
        } else {
            params->initmode = INHERIT_OK;
        }
    }

    if (pme->interactive) {
        r = cli_loop(pme);
    } else {
        logger_init();
        r = server_loop(pme);
    }
#endif

    exit(r < 0 ? 1 : 0);
}

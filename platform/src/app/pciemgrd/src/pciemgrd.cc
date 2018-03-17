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
#include "pciehdevices.h"
#include "pciemgrutils.h"
#include "pciehw.h"
#include "pciehw_dev.h"
#include "pcieport.h"
#include "pciemgrd_impl.hpp"

static pciemgrenv_t pciemgrenv;

static void
usage(void)
{
    fprintf(stderr,
"Usage: pciemgrd [-Fnv][-e <enabled_ports>[-b <first_bus_num>]"
            "[-P gen<G>x<W>][-V subvendorid][-D subdeviceid]\n"
"    -b <first_bus_num> set first bus used to <first_bus_num>\n"
"    -d                 daemon mode\n"
"    -e <enabled_ports> mask of enabled pcie ports\n"
"    -F                 no fake bios scan\n"
"    -i                 interactive mode\n"
"    -I <inherit-mode>  inherit or re-initialize hw/shmem\n"
"    -P gen<G>x<W>      spec devices as pcie gen <G>, lane width <W>\n"
"    -V subvendorid     default subsystem vendor id\n"
"    -D subdeviceid     default subsystem device id\n");
}

pciemgrenv_t *
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

int
main(int argc, char *argv[])
{
    pciemgrenv_t *pme = pciemgrenv_get();
    pciehdev_params_t p;
    int opt, port, interactive;

    interactive = 1;

    memset(&p, 0, sizeof(p));
    p.fake_bios_scan = 1;
    p.subdeviceid = PCI_SUBDEVICE_ID_PENSANDO_NAPLES100;

    /*
     * For x86_64 we want to FORCE_INIT to reinitialize hw/shmem on startup.
     */
#ifdef __aarch64__
    p.initmode = FORCE_INIT;
#else
    p.initmode = FORCE_INIT;
#endif

    /*
     * On "real" ARM systems the upstream port bridge
     * is in hw and our first virtual device is bus 0 at 00:00.0.
     *
     * For simulation we want the virtual upstream port bridge
     * at 00:00.0 so our first virtual device is bus 1 at 01:00.0.
     */
#ifdef __aarch64__
    p.first_bus = 0;
#else
    p.first_bus = 1;
#endif

    /* on asic single port, on haps 2 ports enabled */
    pme->enabled_ports = pal_is_asic() ? 0x1 : 0x5;
    while ((opt = getopt(argc, argv, "b:Cde:FiI:P:V:D:")) != -1) {
        switch (opt) {
        case 'b':
            p.first_bus = strtoul(optarg, NULL, 0);
            break;
        case 'C':
            p.compliance = 1;
            break;
        case 'd':
            interactive = 0;
            break;
        case 'e':
            pme->enabled_ports = strtoul(optarg, NULL, 0);
            break;
        case 'F':
            p.fake_bios_scan = 0;
            break;
        case 'i':
            interactive = 1;
            break;
        case 'I':
            if (strcmp(optarg, "inherit_only") == 0) {
                p.initmode = INHERIT_ONLY;
            } else if (strcmp(optarg, "inherit_ok") == 0) {
                p.initmode = INHERIT_OK;
            } else if (strcmp(optarg, "force_init") == 0) {
                p.initmode = FORCE_INIT;
            } else {
                printf("bad -I arg: inherit|inherit_ok|force_init\n");
                exit(1);
            }
            break;
        case 'P':
            if (!parse_linkspec(optarg, &p.cap_gen, &p.cap_width)) {
                printf("bad pcie spec: want gen%%dx%%d, got %s\n", optarg);
                exit(1);
            }
            break;
        case 'V':
            p.subvendorid = strtoul(optarg, NULL, 0);
            break;
        case 'D':
            p.subdeviceid = strtoul(optarg, NULL, 0);
            break;
        case '?':
        default:
            usage();
            exit(1);
        }
    }

    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->enabled_ports & (1 << port)) {
            pcieport_t *pport;

            if ((pport = pcieport_open(port)) == NULL) {
                printf("pcieport_open %d failed\n", port);
                exit(1);
            }
            if (pcieport_hostconfig(pport, &p) < 0) {
                printf("pcieport_hostconfig %d failed\n", port);
                exit(1);
            }

            pme->pport[port] = pport;
        }
    }

    if (pciehdev_open(&p) < 0) {
        printf("pciehdev_open failed\n");
        exit(1);
    }
    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->enabled_ports & (1 << port)) {
            if (pciehdev_initialize(port) < 0) {
                printf("pciehdev_initialize failed\n");
                exit(1);
            }
        }
    }

    if (interactive) {
        cli_loop();
    } else {
        server_loop();
    }

    for (int port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->pport[port]) {
            pcieport_close(pme->pport[port]);
        }
    }
    pciehdev_close();
    exit(0);
}

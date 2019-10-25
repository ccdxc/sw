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

#include "third-party/asic/capri/verif/apis/cap_freq_api.h"
#include "platform/sensor/sensor.hpp"
#include "platform/pciehdevices/include/pci_ids.h"
#include "nic/sdk/platform/pal/include/pal.h"
#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/platform/pciemgrutils/include/pciesys.h"
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "nic/sdk/platform/pcieport/include/pcieport.h"
#include "nic/sdk/lib/catalog/catalog.hpp"
#include "nic/sdk/platform/pciemgrd/pciemgrd_impl.hpp"
#include "pciemgrd_impl.hpp"

using namespace sdk::platform::sensor;

static void
usage(void)
{
    fprintf(stderr,
"Usage: pciemgrd [-dFinrv][-e <enabled_ports>[-b <first_bus_num>]\n"
"                [-I <inherit-mode>][-P gen<G>x<W>][-R 0|1]\n"
"                [-V subvendorid][-D subdeviceid]\n"
"    -b <first_bus_num> set first bus used to <first_bus_num>\n"
"    -d                 daemon mode\n"
"    -e <enabled_ports> mask of enabled pcie ports\n"
"    -F                 no fake bios scan\n"
"    -i                 interactive mode\n"
"    -I <inherit-mode>  inherit or re-initialize hw/shmem\n"
"    -P gen<G>x<W>      spec devices as pcie gen <G>, lane width <W>\n"
"    -r                 restart after upgrade\n"
"    -R <0|1>           reboot_on_hostdn=<0|1>\n"
"    -V subvendorid     default subsystem vendor id\n"
"    -D subdeviceid     default subsystem device id\n");
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
    int r, opt;

    pme->interactive = 1;
    pme->reboot_on_hostdn = pal_is_asic() ? 1 : 0;
    pme->poll_port = 1;
    pme->poll_dev = 0;
    pme->fifopri = 50;

    params->strict_crs = 1;

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
    params->initmode = INHERIT_OK;
    params->first_bus = 0;
#else
    params->initmode = FORCE_INIT;
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

    while ((opt = getopt(argc, argv, "b:Cde:FGiI:P:V:D:rR:")) != -1) {
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
        case 'e':
            pme->enabled_ports = strtoul(optarg, NULL, 0);
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

#ifdef PCIEMGRD_GOLD
    {
        int gold_loop(pciemgrenv_t *pme);
        r = gold_loop(pme);
    }
#else
    if (pme->interactive) {
        r = cli_loop(pme);
    } else {
        logger_init();
        r = server_loop(pme);
    }
#endif

    exit(r < 0 ? 1 : 0);
}

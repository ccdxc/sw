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

#include "platform/include/common/pci_ids.h"
#include "platform/src/lib/pal/include/pal.h"
#include "platform/src/lib/pciemgr/include/pciemgr.h"
#include "platform/src/lib/pciemgrutils/include/pciesys.h"
#include "platform/src/lib/pcieport/include/pcieport.h"

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
"    -R <0|1>           reboot_on_hostdn=<0|1>\n"
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

static void
reboot_the_system(void)
{
    int r = system("/sbin/reboot -f");
    if (r) pciesys_logerror("failed to reboot %d\n", r);
}

static void
port_evhandler(pcieport_event_t *ev, void *arg)
{
    pciemgrenv_t *pme = pciemgrenv_get();

    switch (ev->type) {
    case PCIEPORT_EVENT_HOSTUP: {
        pciesys_loginfo("port%d hostup gen%dx%d\n",
                        ev->port, ev->hostup.gen, ev->hostup.width);
        pciehw_event_hostup(ev->port, ev->hostup.gen, ev->hostup.width);
        break;
    }
    case PCIEPORT_EVENT_HOSTDN: {
        if (pme->reboot_on_hostdn) {
            reboot_the_system();
        }
        pciesys_loginfo("port%d: hostdn\n", ev->port);
        pciehw_event_hostdn(ev->port);
        break;
    }
    case PCIEPORT_EVENT_BUSCHG: {
        const int port = ev->port;
        const u_int8_t secbus = ev->buschg.secbus;
        pciesys_loginfo("port%d: buschg 0x%02x\n", ev->port, secbus);
        pciehw_event_buschg(port, secbus);
        break;
    }
    default:
        /* Some event we don't need to handle. */
        pciesys_loginfo("port%d: event %d ignored\n", ev->port, ev->type);
        break;
    }
}

int
open_hostports(void)
{
    pciemgrenv_t *pme = pciemgrenv_get();
    pciehdev_params_t *p = &pme->params;
    int r, port;

    /*
     * Open and configure all the ports we are going to manage.
     */
    r = 0;
    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->enabled_ports & (1 << port)) {
            if ((r = pcieport_open(port)) < 0) {
                pciesys_logerror("pcieport_open %d failed: %d\n", port, r);
                goto error_out;
            }
            if ((r = pcieport_hostconfig(port, p)) < 0) {
                pciesys_logerror("pcieport_hostconfig %d failed\n", port);
                goto close_error_out;
            }
        }
    }
    if ((r = pcieport_register_event_handler(port_evhandler, NULL)) < 0) {
        goto close_error_out;
    }
    return r;

 close_error_out:
    pcieport_close(port);
 error_out:
    for (port--; port >= 0; port--) {
        pcieport_close(port);
    }
    return r;
}

void
close_hostports(void)
{
    pciemgrenv_t *pme = pciemgrenv_get();
    int port;

    /* close all the ports we opened */
    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->enabled_ports & (1 << port)) {
            pcieport_close(port);
        }
    }
}

static u_int64_t
getenv_override_ull(const char *label, const char *name, const u_int64_t def)
{
    const char *env = getenv(name);
    if (env) {
        u_int64_t val = strtoull(env, NULL, 0);
        pciesys_loginfo("%s: $%s override %" PRIu64 " (%" PRIx64 ")\n",
                        label, name, val, val);
        return val;
    }
    return def;
}

static u_int64_t
pciemgrd_param_ull(const char *name, const u_int64_t def)
{
    return getenv_override_ull("pciemgrd", name, def);
}

void
pciemgrd_params(pciemgrenv_t *pme)
{
    pme->reboot_on_hostdn = pciemgrd_param_ull("PCIE_REBOOT_ON_HOSTDN",
                                               pme->reboot_on_hostdn);
    pme->enabled_ports = pciemgrd_param_ull("PCIE_ENABLED_PORTS",
                                            pme->enabled_ports);
}

int
main(int argc, char *argv[])
{
    pciemgrenv_t *pme = pciemgrenv_get();
    pciehdev_params_t *p = &pme->params;
    int r, opt;

    pme->interactive = 1;
    pme->reboot_on_hostdn = pal_is_asic() ? 1 : 0;

    p->strict_crs = 1;
    p->fake_bios_scan = 1;
    p->subdeviceid = PCI_SUBDEVICE_ID_PENSANDO_NAPLES100;

    /*
     * For x86_64 we want to FORCE_INIT to reinitialize hw/shmem on startup.
     */
#ifdef __aarch64__
    p->initmode = FORCE_INIT;
#else
    p->initmode = FORCE_INIT;
#endif

    /*
     * On "real" ARM systems the upstream port bridge
     * is in hw and our first virtual device is bus 0 at 00:00.0.
     *
     * For simulation we want the virtual upstream port bridge
     * at 00:00.0 so our first virtual device is bus 1 at 01:00.0.
     */
#ifdef __aarch64__
    p->first_bus = 0;
#else
    p->first_bus = 1;
#endif

    /* on asic single port, on haps 2 ports enabled */
    pme->enabled_ports = pal_is_asic() ? 0x1 : 0x5;
    while ((opt = getopt(argc, argv, "b:Cde:FGiI:P:V:D:R:")) != -1) {
        switch (opt) {
        case 'b':
            p->first_bus = strtoul(optarg, NULL, 0);
            break;
        case 'C':
            p->compliance = 1;
            break;
        case 'd':
            pme->interactive = 0;
            break;
        case 'e':
            pme->enabled_ports = strtoul(optarg, NULL, 0);
            break;
        case 'F':
            p->fake_bios_scan = 0;
            break;
        case 'G':
            pme->gold = 1;
            break;
        case 'i':
            pme->interactive = 1;
            break;
        case 'I':
            if (strcmp(optarg, "inherit_only") == 0) {
                p->initmode = INHERIT_ONLY;
            } else if (strcmp(optarg, "inherit_ok") == 0) {
                p->initmode = INHERIT_OK;
            } else if (strcmp(optarg, "force_init") == 0) {
                p->initmode = FORCE_INIT;
            } else {
                printf("bad -I arg: inherit|inherit_ok|force_init\n");
                exit(1);
            }
            break;
        case 'P':
            if (!parse_linkspec(optarg, &p->cap_gen, &p->cap_width)) {
                printf("bad pcie spec: want gen%%dx%%d, got %s\n", optarg);
                exit(1);
            }
            break;
        case 'R':
            pme->reboot_on_hostdn = strtoul(optarg, NULL, 0);
            break;
        case 'V':
            p->subvendorid = strtoul(optarg, NULL, 0);
            break;
        case 'D':
            p->subdeviceid = strtoul(optarg, NULL, 0);
            break;
        case '?':
        default:
            usage();
            exit(1);
        }
    }

#ifdef PCIEMGRD_GOLD
    r = gold_loop();
#else
    if (pme->gold) {
        r = gold_loop();
    } else if (pme->interactive) {
        r = cli_loop();
    } else {
        r = server_loop();
    }
#endif

    exit(r < 0 ? 1 : 0);
}

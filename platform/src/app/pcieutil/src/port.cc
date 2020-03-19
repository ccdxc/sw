/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <cinttypes>
#include <sys/time.h>
#include <sys/param.h>

#include "lib/catalog/catalog.hpp"
#include "nic/sdk/platform/misc/include/bdf.h"
#include "nic/sdk/platform/misc/include/misc.h"
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "nic/sdk/platform/pcieport/include/pcieport.h"
#include "nic/sdk/platform/pcieport/include/portmap.h"
#include "nic/sdk/platform/pcieport/include/portcfg.h"
#include "nic/sdk/platform/pciemgrd/pciemgrd.hpp"

#include "cmd.h"
#include "utils.hpp"

static void
port(int argc, char *argv[])
{
    int opt, port;

    port = default_pcieport();
    optind = 0;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':
            port = strtoul(optarg, NULL, 0);
            break;
        default:
            return;
        }
    }
    if (pcieport_open(port, INHERIT_ONLY) < 0) {
        fprintf(stderr, "pcieport_open failed\n");
        return;
    }
    pcieport_showport(port);
    pcieport_close(port);
}
CMDFUNC(port,
"port information",
"port [-p <port>]\n"
"    -p <port>  poll port <port> (default port 0)\n");

static void
ports(int argc, char *argv[])
{
    uint32_t portmask;
    int port;

    portmap_init_from_catalog();
    portmask = portmap_portmask();

    /* find and open the first active port to map in the shared data */
    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        if (portmask & (1 << port)) {
            if (pcieport_open(port, INHERIT_ONLY) < 0) {
                fprintf(stderr, "pcieport_open %d failed\n", port);
                return;
            }
            break;
        }
    }
    if (port >= PCIEPORT_NPORTS) {
        return;
    }
    pcieport_showports();
    pcieport_close(port);
}
CMDFUNC(ports,
"display port summary",
"ports\n");

static void
portstats(int argc, char *argv[])
{
    int opt, port, do_clear;
    unsigned int flags;

    flags = PSF_NONE;
    port = default_pcieport();
    do_clear = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "acp:")) != -1) {
        switch (opt) {
        case 'a':
            flags |= PSF_ALL;
            break;
        case 'c':
            do_clear = 1;
            break;
        case 'p':
            port = strtoul(optarg, NULL, 0);
            break;
        default:
            return;
        }
    }

    if (pcieport_open(port, INHERIT_ONLY) < 0) {
        fprintf(stderr, "pcieport_open %d failed\n", port);
        return;
    }

    if (do_clear) pcieport_clearportstats(port, flags);
    else          pcieport_showportstats(port, flags);

    pcieport_close(port);
}
CMDFUNC(portstats,
"port statistics",
"portstats [-ac][-p <port>]\n"
"    -a         show all stats, even if 0\n"
"    -c         clear all stats to 0\n"
"    -p <port>  poll port <port> (default port 0)\n");

static void
portmap_host(const int host, void *arg)
{
    const int port = portmap_pcieport(host);
    if (port < 0) {
        fprintf(stderr, "host%d bad port\n", host);
        return;
    }

    pcieport_spec_t ps;
    if (portmap_getspec(port, &ps) < 0) {
        fprintf(stderr, "host%d no spec\n", host);
        return;
    }

    printf("host%d port%d gen%dx%d\n", host, port, ps.gen, ps.width);
}

static void
portmap(int argc, char *argv[])
{
    portmap_init_from_catalog();
    portmap_foreach_host(portmap_host, NULL);
}
CMDFUNC(portmap,
"port map to host ports",
"portmap\n");

static void
portcfg(int argc, char *argv[])
{
#define NDW     4
#define LINESZ  (NDW * sizeof(u_int32_t))
    const int cfgszs[] = { 64, 64, 64, 256, 4096 }; /* match lspci -xxxx */
    const int ncfgszs = sizeof(cfgszs) / sizeof(cfgszs[0]);
    int opt, port, cfgszidx, cfgsz;

    port = default_pcieport();
    cfgszidx = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "p:x")) != -1) {
        switch (opt) {
        case 'p':
            port = strtoul(optarg, NULL, 0);
            break;
        case 'x':
            cfgszidx++;
            break;
        default:
            return;
        }
    }
    cfgsz = cfgszs[MIN(cfgszidx, ncfgszs - 1)];

    if (!pcieport_is_accessible(port)) {
        fprintf(stderr, "port%d not accessible\n", port);
        return;
    }

    u_int8_t pribus;
    portcfg_read_bus(port, &pribus, NULL, NULL);
    const u_int16_t bdf = bdf_make(pribus, 0, 0); // bridge bdf

    printf("%s hwbridge\n", bdf_to_str(bdf));
    for (int cfgoff = 0; cfgoff < cfgsz; cfgoff += LINESZ) {
        char cfgbuf[LINESZ], outbuf[80];

        portcfg_read(port, cfgoff, cfgbuf, LINESZ);
        hex_format(outbuf, sizeof(outbuf), cfgbuf, LINESZ);
        printf("%02x: %s\n", cfgoff, outbuf);
    }
}
CMDFUNC(portcfg,
"port hw bridge cfg space",
"portcfg [-px]\n"
"    -p <port>  show port <port> (default port 0)\n"
"    -x         like lspci -x show more cfgspace (default -x)\n");

//
// Print decoded Uncorrectable Error register from
// Advanced Error Reporting capability.  Used for
//
// UESta: Uncorrectable Error Status   register.
// UEMsk: Uncorrectable Error Mask     register.
// UESev: Uncorrectable Error Severity register.
//
static void
print_ue_reg(const char *regname, const u_int32_t v)
{
    printf("%s: 0x%08x", regname, v);
#define PRBIT(v, pos, name) \
    printf(" %s%c", #name, (v) & (1 << (pos)) ? '+' : '-')
    PRBIT(v,  4, DLP);
    PRBIT(v,  5, SDES);
    PRBIT(v, 12, TLP);
    PRBIT(v, 13, FCP);
    PRBIT(v, 14, CplTO);
    PRBIT(v, 15, CplAbt);
    PRBIT(v, 16, UnxCpl);
    PRBIT(v, 17, RxOF);
    PRBIT(v, 18, MalfTLP);
    PRBIT(v, 19, ECRC);
    PRBIT(v, 20, UR);
    PRBIT(v, 21, ACSViol);
    PRBIT(v, 22, IntErr);
    PRBIT(v, 23, MCBlkTLP);
    PRBIT(v, 24, AtOpBlk);
    PRBIT(v, 25, TLPPrefBlk);
    PRBIT(v, 26, TLPBlk);
    putchar('\n');
#undef PRBIT
}

//
// Print decoded Correctable Error register from
// Advanced Error Reporting capability.  Used for
//
// CESta: Correctable Error Status register.
// CEMsk: Correctable Error Mask   register.
//
static void
print_ce_reg(const char *regname, const u_int32_t v)
{
    printf("%s: 0x%08x", regname, v);
#define PRBIT(v, pos, name) \
    printf(" %s%c", #name, (v) & (1 << (pos)) ? '+' : '-')
    PRBIT(v,  0, RxErr);
    PRBIT(v,  6, BadTLP);
    PRBIT(v,  7, BadDLLP);
    PRBIT(v,  8, Rollover);
    PRBIT(v, 12, Timeout);
    PRBIT(v, 13, NonFatalErr);
    PRBIT(v, 14, IntErr);
    PRBIT(v, 15, LogOF);
    putchar('\n');
#undef PRBIT
}

//
// Dump AER Capability registers in readable format.
// Somewhat like "lspci -vv":
//
// Capabilities: [200 v2] Advanced Error Reporting
//   UESta:  DLP- SDES- TLP- FCP- CmpltTO- CmpltAbrt- UnxCmplt- RxOF-
//           MalfTLP- ECRC- UnsupReq- ACSViol-
//   UEMsk:  DLP- SDES- TLP- FCP- CmpltTO- CmpltAbrt- UnxCmplt- RxOF-
//           MalfTLP- ECRC- UnsupReq- ACSViol-
//   UESvrt: DLP+ SDES- TLP- FCP+ CmpltTO- CmpltAbrt- UnxCmplt- RxOF-
//           MalfTLP+ ECRC- UnsupReq- ACSViol-
//   CESta:  RxErr- BadTLP- BadDLLP- Rollover- Timeout- NonFatalErr-
//   CEMsk:  RxErr- BadTLP- BadDLLP- Rollover- Timeout- NonFatalErr+
//   AERCap: First Error Pointer: 00, GenCap+ CGenEn- ChkCap+ ChkEn-
//
static void
portaer(int argc, char *argv[])
{
#define AER_NDW 6
    int opt, port;
    u_int32_t aerdw[AER_NDW];

    port = default_pcieport();
    optind = 0;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':
            port = strtoul(optarg, NULL, 0);
            break;
        default:
            return;
        }
    }

    if (!pcieport_is_accessible(port)) {
        fprintf(stderr, "port%d not accessible\n", port);
        return;
    }

    portcfg_read(port, PORTCFG_CAP_AER, aerdw, AER_NDW * 4);

    print_ue_reg("UESta", aerdw[1]);
    print_ue_reg("UEMsk", aerdw[2]);
    print_ue_reg("UESev", aerdw[3]);
    print_ce_reg("CESta", aerdw[4]);
    print_ce_reg("CEMsk", aerdw[5]);
}
CMDFUNC(portaer,
"port hw bridge Advanced Error Reporting capability",
"portaer [-p port]\n"
"    -p <port>  show port <port> (default port 0)\n");

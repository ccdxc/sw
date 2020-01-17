/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>
#include <cinttypes>
#include <sys/param.h>

#include "nic/sdk/platform/pal/include/pal.h"
#include "nic/sdk/platform/misc/include/misc.h"
#include "nic/sdk/platform/misc/include/bdf.h"
#include "nic/sdk/platform/pcietlp/include/pcietlp.h"
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "nic/sdk/platform/pcieport/include/pcieport.h"
#include "nic/sdk/platform/pcieport/include/portcfg.h"
#include "nic/sdk/platform/pcieport/include/rawtlp.h"
#include "cmd.h"
#include "utils.hpp"

static void
hexdump(uint64_t addr, char *buf, const size_t size)
{
    for (size_t off = 0; off < size; off += 16, addr += 16, buf += 16) {
        const int n = MIN(16, size - off);
        char outbuf[80], *s;

        s = hex_format(outbuf, sizeof(outbuf), buf, n);
        printf("%08" PRIx64 ": %s\n", addr, s);
    }
}

static int
hex_encode(char *s, char *buf, size_t bufsz)
{
    size_t n;
    char *cp, *bp;
#define HEXC(c) \
    ((c) >= 'a' && (c) <= 'f' ? 10 + (c) - 'a' : \
     (c) >= 'A' && (c) <= 'F' ? 10 + (c) - 'A' : \
     (c) - '0')
#define HEXB(c1, c2) ((HEXC(c1) << 4) | HEXC(c2))

    if (s == NULL) return 0;
    n = 0;
    bp = buf;
    for (cp = s; n < bufsz && cp[0] && cp[1]; cp += 2, bp++, n++) {
        if (!isxdigit(cp[0]) || !isxdigit(cp[1])) return -1;
        *bp = HEXB(cp[0], cp[1]);
    }
    if (cp[0]) return -1;
    return n;
}

static void
memrd(int argc, char *argv[], int memcat)
{
    int opt, port, reqid;
    uint64_t addr;
    uint32_t size;

    port = -1;
    reqid = -1;
    optind = 0;
    while ((opt = getopt(argc, argv, "p:r:")) != -1) {
        switch (opt) {
        case 'p':
            port = strtoul(optarg, NULL, 0);
            break;
        case 'r':
            reqid = strtoul(optarg, NULL, 0);
            break;
        default:
            goto usage;
        }
    }

    if (optind >= argc) {
        goto usage;
    }

    addr = strtoull(argv[optind], NULL, 0);
    size = optind + 1 < argc ? strtoul(argv[optind + 1], NULL, 0) : 4;

    if (port == -1) {
        port = default_pcieport();
    }

    if (reqid == -1) {
        uint8_t pribus;
        portcfg_read_bus(port, &pribus, NULL, NULL);
        reqid = bdf_make(pribus, 0, 0);
    }

    if (memcat && isatty(1)) {
        fprintf(stderr, "redirect stdout for memcat binary data\n");
        return;
    }

    char buf[256];
    while (size) {
        const size_t sz = MIN(size, sizeof(buf));
        int n = hostmem_read(port, reqid, addr, buf, sz);
        if (n < 0) {
            fprintf(stderr, "hostmem_read failed: %d\n", n);
            break;
        }
        if (memcat) {
            fwrite(buf, n, 1, stdout);
        } else {
            hexdump(addr, buf, n);
        }
        addr += n;
        size -= n;
    }
    return;

 usage:
    fprintf(stderr,
"Usage: %s [-p <port>][-r <reqid>] <addr> [<size>]\n"
"    -p <port>  use port <port> (default 0)\n"
"    -r <reqid> use requester id <reqid> (default use hw bridge bdf)\n"
"    <addr>     target read address\n"
"    <size>     target read size (default 4)\n",
            argv[0]);
}

static void
rawtlp_memrd(int argc, char *argv[])
{
    memrd(argc, argv, 0);
}

static void
rawtlp_memcat(int argc, char *argv[])
{
    memrd(argc, argv, 1);
}

static void
rawtlp_memwr(int argc, char *argv[])
{
    int opt, port, reqid;
    uint64_t addr;
    uint32_t size;

    port = -1;
    reqid = -1;
    size = 4;
    optind = 0;
    while ((opt = getopt(argc, argv, "p:r:s:")) != -1) {
        switch (opt) {
        case 'p':
            port = strtoul(optarg, NULL, 0);
            break;
        case 'r':
            reqid = strtoul(optarg, NULL, 0);
            break;
        case 's':
            size = strtoul(optarg, NULL, 0);
            if (size != 4 && size != 8) {
                goto usage;
            }
            break;
        default:
            goto usage;
        }
    }

    if (optind >= argc) {
        goto usage;
    }

    addr = strtoull(argv[optind++], NULL, 0);

    if (port == -1) {
        port = default_pcieport();
    }

    if (reqid == -1) {
        uint8_t pribus;
        portcfg_read_bus(port, &pribus, NULL, NULL);
        reqid = bdf_make(pribus, 0, 0);
    }

    for (int i = 0; i < argc - optind; i++) {
        uint64_t data64;
        uint32_t data32;
        void *data;

        if (size == 8) {
            data64 = strtoull(argv[optind + i], NULL, 0);
            data = &data64;
        } else {
            data32 = strtoul(argv[optind + i], NULL, 0);
            data = &data32;
        }

        int n = hostmem_write(port, reqid, addr, data, size);
        if (n < 0) {
            fprintf(stderr, "hostmem_write failed: %d\n", n);
            break;
        }

        addr += n;
    }
    return;

 usage:
    fprintf(stderr,
"Usage: memwr [-p <port>][-r <reqid>][-s 4|8] <addr> <val1> <val2> ...\n"
"    -p <port>  use port <port> (default 0)\n"
"    -r <reqid> use requester id <reqid> (default use hw bridge bdf)\n"
"    -s 4|8     write size (default 4)\n"
"    <addr>     target read address\n"
"    <val>      value(s) to write\n"
            );
}

static void
rawtlp_raw(int argc, char *argv[])
{
    int opt, port, r, n;

    port = -1;
    optind = 0;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':
            port = strtoul(optarg, NULL, 0);
            break;
        default:
            goto usage;
        }
    }

    if (optind >= argc) {
        goto usage;
    }

    if (port == -1) {
        port = default_pcieport();
    }

    rawtlp_req_t req;
    memset(&req, 0, sizeof(req));
    n = hex_encode(argv[optind], (char *)req.b, sizeof(req.b));
    if (n < 0) {
        fprintf(stderr, "invalid hexbytes\n");
        return;
    }

    rawtlp_rsp_t rsp;
    sta_itr_raw_tlp_t sta;
    r = rawtlp_send(port, req.w, n, rsp.w, sizeof(rsp.w), &sta);
    if (r < 0) {
        fprintf(stderr, "rawtlp%d send failed: %d\n", port, r);
        return;
    }

    r = rawtlp_sta_errs(port, &sta);
    if (r < 0) {
        return;
    }

    // Display all response words.
    hexdump(0, (char *)rsp.b, sizeof(rsp.b));

    return;

 usage:
    fprintf(stderr,
"Usage: raw [-p <port>] <hexbytes>\n"
"    -p <port>  use port <port> (default 0)\n"
"    <hexbytes> hex bytes of raw tlp\n"
            );
}

static void
rawtlp(int argc, char *argv[])
{
    if (argc <= 1) {
        goto usage;
    }
    if (strcmp(argv[1], "memrd") == 0) {
        rawtlp_memrd(--argc, ++argv);
    } else if (strcmp(argv[1], "memcat") == 0) {
        rawtlp_memcat(--argc, ++argv);
    } else if (strcmp(argv[1], "memwr") == 0) {
        rawtlp_memwr(--argc, ++argv);
    } else if (strcmp(argv[1], "raw") == 0) {
        rawtlp_raw(--argc, ++argv);
    } else {
        goto usage;
    }
    return;

 usage:
    fprintf(stderr,
"Usage: %s memrd|memcat|memwr|raw ...\n"
"    memrd  [options]   issue memory read transaction\n"
"    memcat [options]   memory read to stdout for hexdump\n"
"    memwr  [options]   issue memory write transaction\n"
"    raw    [options]   issue raw transaction\n",
            argv[0]);
}
CMDFUNC(rawtlp,
"send raw tlp",
"rawtlp memrd|memcat|memwr|raw ...\n"
"    memrd  [options]   issue memory read transaction\n"
"    memcat [options]   memory read to stdout for hexdump\n"
"    memwr  [options]   issue memory write transaction\n"
"    raw    [options]   issue raw transaction\n");

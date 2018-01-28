/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/param.h>

#include "src/lib/misc/include/bdf.h"
#include "src/lib/misc/include/misc.h"
#include "src/lib/pcietlp/include/pcietlp.h"

static void
usage(void)
{
    fprintf(stderr, "pcietlp [-ed] <file>\n");
}

static int
stlp_type_encode(const char *typestr)
{
    static char *stlp_type_tab[] = {
#define TAB_ENT(N) \
        [PCIE_STLP_##N] = #N
        TAB_ENT(MALFORMED),
        TAB_ENT(CFGRD),
        TAB_ENT(CFGWR),
        TAB_ENT(CFGRD1),
        TAB_ENT(CFGWR1),
        TAB_ENT(MEMRD),
        TAB_ENT(MEMWR),
        TAB_ENT(MEMRD64),
        TAB_ENT(MEMWR64),
        TAB_ENT(IORD),
        TAB_ENT(IOWR),
        TAB_ENT(MSG),
        TAB_ENT(MSGD),
    };
    int i;

    for (i = 0; i < sizeof(stlp_type_tab) / sizeof(stlp_type_tab[0]); i++) {
        if (strcmp(stlp_type_tab[i], typestr) == 0) {
            return i;
        }
    }
    return -1;
}

static int
stlp_from_str(const char *buf, pcie_stlp_t *stlp)
{
    u_int64_t addr, data;
    char bdfstr[16], typestr[24];
    u_int32_t size, tag;
    int n;

    data = 0;
    n = sscanf(buf, "%s %s 0x%"PRIx64" size %d tag %d 0x%"PRIx64"", 
               typestr, bdfstr, &addr, &size, &tag, &data);
    if (n < 5) {
        strncpy(bdfstr, "00:00.0", sizeof(bdfstr));
        n = sscanf(buf, "%s 0x%"PRIx64" size %d tag %d 0x%"PRIx64"", 
                   typestr, &addr, &size, &tag, &data);
        if (n < 4) {
            return -1;
        }
    }
    memset(stlp, 0, sizeof(*stlp));
    stlp->type = stlp_type_encode(typestr);
    stlp->bdf = bdf_from_str(bdfstr);
    stlp->addr = addr;
    stlp->size = size;
    stlp->tag = tag;
    stlp->data = data;
    return 0;
}

static void
stlp_write(FILE *fp, pcie_stlp_t *stlp)
{
    fprintf(fp, "%s\n", pcietlp_str(stlp));
}

static int
is_empty(char *line)
{
    while (isspace(*line)) line++;
    return *line == '\0';
}

static int
is_comment(char *line)
{
    while (isspace(*line)) line++;
    return *line == '#';
}

static int
rtlp_hexinput(FILE *fp, u_int8_t *rtlp, const int rtlpsz)
{
    char line[80];
    u_int32_t v[16];
    int n, nf;

    n = 0;
    while (1) {
        if (fgets(line, sizeof(line), fp) == NULL) break; /* EOF */
        if (is_comment(line)) continue; /* skip #comment lines */
        if (is_empty(line)) {
            if (n) break;       /* if we've seen some data, return */
            continue;           /* else continue until we have some data */
        }

        nf = sscanf(line, "%*s "
                    "%02x %02x %02x %02x %02x %02x %02x %02x"
                    "%02x %02x %02x %02x %02x %02x %02x %02x",
                    &v[ 0], &v[ 1], &v[ 2], &v[ 3], 
                    &v[ 4], &v[ 5], &v[ 6], &v[ 7],
                    &v[ 8], &v[ 9], &v[10], &v[11],
                    &v[12], &v[13], &v[14], &v[15]);
        if (nf == 16 && n + nf < rtlpsz) {
            int i;

            for (i = 0; i < nf; i++) {
                rtlp[n + i] = v[i];
            }
            n += nf;
            if (n + 16 > rtlpsz) break; /* nearly full, better stop */
            continue;
        }

        nf = sscanf(line, "%*s %02x %02x %02x %02x",
                    &v[0], &v[1], &v[2], &v[3]);
        if (nf == 4 && n + nf < rtlpsz) {
            int i;

            for (i = 0; i < nf; i++) {
                rtlp[n + i] = v[i];
            }
            n += nf;
            if (n + 4 > rtlpsz) break; /* nearly full, better stop */
            continue;
        }

        /* unrecognized line format */
        break;
    };
    return n;
}

static void
rtlp_hexdump(const unsigned char *rtlp, const int n)
{
    char buf[80];
    int offset;

    for (offset = 0; offset < n; offset += 4) {
        const int seglen = MIN(4, n - offset);
        hex_format(buf, sizeof(buf), &rtlp[offset], seglen);
        printf("%08x: %s\n", offset, buf);
    }
}

static void
do_encode(FILE *fp)
{
    pcie_stlp_t stlp_buf, *stlp = &stlp_buf;
    char line[80];
    u_int8_t rtlp[64];
    int n, lineno;

    lineno = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        lineno++;
        if (stlp_from_str(line, stlp) < 0) {
            fprintf(stderr, "Invalid input at line %d\n", lineno);
            continue;
        }
        n = pcietlp_encode(stlp, rtlp, sizeof(rtlp));
        if (n < 0) {
            fprintf(stderr, "Line %d: encode error: %s\n",
                    lineno, pcietlp_get_error());
            continue;
        }
        rtlp_hexdump(rtlp, n);
        putchar('\n');
    }
}

static void
do_decode(FILE *fp)
{
    pcie_stlp_t stlp_buf, *stlp = &stlp_buf;
    u_int8_t rtlp[64];
    int n;

    while ((n = rtlp_hexinput(fp, rtlp, sizeof(rtlp))) > 0) {
        n = pcietlp_decode(stlp, rtlp, n);
        if (n < 0) {
            fprintf(stderr, "decode error: %s\n", pcietlp_get_error());
            continue;
        }
        stlp_write(stdout, stlp);
    }
}

int
main(int argc, char *argv[])
{
    int opt;
    int encode = 0;
    FILE *fp;

    while ((opt = getopt(argc, argv, "ed")) != -1) {
        switch (opt) {
        case 'e':
            encode = 1;
            break;
        case 'd':
            encode = 0;
            break;
        default:
            usage();
            exit(1);
        }
    }

    if (argc > optind + 1) {
        usage();
        exit(1);
    }

    if (optind < argc) {
        fp = fopen(argv[optind], "r");
        if (fp == NULL) {
            perror(argv[optind]);
            exit(1);
        }
    } else {
        fp = stdin;
    }

    if (encode) {
        do_encode(fp);
    } else {
        do_decode(fp);
    }
    exit(0);
}

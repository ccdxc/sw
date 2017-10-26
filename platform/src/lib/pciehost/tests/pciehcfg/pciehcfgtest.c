/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>

#include "pciehcfg.h"
#include "../../src/pciehcfg_impl.h"
#include "../../src/pciehcfg_getset.h"

static int verbose_flag;

static void
usage(void)
{
    fprintf(stderr, "Usage: pciehcfgtest [-v]\n");
}

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

static char *
hex_format(char *buf, const size_t bufsz,
           const u_int8_t *data, const size_t datasz)
{
    int i, len;
    char *bp;
    size_t bufleft;

    bp = buf;
    bufleft = bufsz;
    for (i = 0; bufleft > 0 && i < datasz; i++) {
        len = snprintf(bp, bufleft, "%s%2.2x", i > 0 ? " ": "", data[i]);
        if (len < 0 || len > bufleft) {
            // doesn't fit so claim we used up the entire buffer
            len = bufleft;
        }
        bufleft -= len;
        bp += len;
    }
    return buf;
}

static char *
msk_format(char *buf, const size_t bufsz,
           const u_int8_t *msk, const size_t msksz)
{
    int i, len;
    char *bp;
    size_t bufleft;

    bp = buf;
    bufleft = bufsz;
    for (i = 0; bufleft > 0 && i < msksz; i++) {
        if (msk[i]) {
            len = snprintf(bp, bufleft, "%s%2.2x", i > 0 ? " ": "", msk[i]);
        } else {
            len = snprintf(bp, bufleft, "%s..", i > 0 ? " ": "");
        }
        if (len < 0 || len > bufleft) {
            // doesn't fit so claim we used up the entire buffer
            len = bufleft;
        }
        bufleft -= len;
        bp += len;
    }
    return buf;
}

static void
pciehcfg_print_range(pciehcfg_t *pcfg,
                     const u_int16_t startaddr, const u_int16_t len)
{
    const u_int8_t *c = pcfg->cur;
    const u_int8_t *m = pcfg->msk;
    const u_int16_t endaddr = startaddr + len;
    char buf[80];
    int offset;

    for (offset = startaddr; offset < endaddr; offset += 16) {
        const u_int16_t seglen = MIN(16, endaddr - offset);
        hex_format(buf, sizeof(buf), &c[offset], seglen);
        printf("%03x: %s\n", offset, buf);
        msk_format(buf, sizeof(buf), &m[offset], seglen);
        printf("msk: %s\n", buf);
    }
}

static void
pciehcfg_print(pciehcfg_t *pcfg)
{
    pciehcfg_print_range(pcfg, 0, PCIEHCFGSZ);
}

static int
cfg_test_range(pciehcfg_t *pcfg, u_int16_t startaddr, u_int16_t len, int msk)
{
    int addr, size, errors;
    u_int32_t result, expect;

    errors = 0;
    for (size = 1; size <= 4; size <<= 1) { /* 1, 2, 4-byte accesses */
        for (addr = startaddr; addr < startaddr + len - size; addr++) {
            switch (size) {
            case 1:
                result = (msk ?
                          pciehcfg_getb_msk(pcfg, addr) :
                          pciehcfg_getb(pcfg, addr));
                if ((addr & 0x1) == 0) {
                    expect = addr & 0xff;
                } else {
                    expect = ((addr & ~1) >> 8) & 0xff;
                }
                break;
            case 2:
                result = (msk ?
                          pciehcfg_getw_msk(pcfg, addr) :
                          pciehcfg_getw(pcfg, addr));
                if ((addr & 0x1) == 0) {
                    expect = addr;
                } else {
                    expect = ((((addr & ~1) >> 8) & 0xff) |
                              (((addr + 1) & 0xff) << 8));
                }
                break;
            case 4:
                result = (msk ?
                          pciehcfg_getd_msk(pcfg, addr) :
                          pciehcfg_getd(pcfg, addr));
                switch (addr & 0x3) {
                case 0:
                case 2:
                    expect = addr | ((addr + 2) << 16);
                    break;
                case 1:
                    expect = ((((((addr & ~0x3) + 0) >> 8) & 0xff) << 0) |
                              (((((addr & ~0x3) + 2)     ) & 0xff) << 8) |
                              (((((addr & ~0x3) + 2) >> 8) & 0xff) << 16) |
                              (((((addr & ~0x3) + 4)     ) & 0xff) << 24));
                    break;
                case 3:
                    expect = ((((((addr & ~0x1) + 0) >> 8) & 0xff) << 0) |
                              (((((addr & ~0x1) + 2)     ) & 0xff) << 8) |
                              (((((addr & ~0x1) + 2) >> 8) & 0xff) << 16) |
                              (((((addr & ~0x1) + 4)     ) & 0xff) << 24));
                    break;
                }
                break;
            }
            if (result != expect) {
                printf("ERROR: addr 0x%04x sz %d, expect 0x%x got 0x%x\n",
                        addr, size, expect, result);
                errors++;
            }
        }
    }
    return errors;
}

int
main(int argc, char *argv[])
{
    pciehcfg_t *pcfg;
    int opt, i, errors;

    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
        case 'v':
            verbose_flag = 1;
            break;
        case '?':
        default:
            usage();
            exit(1);
        }
    }

    pcfg = pciehcfg_new(NULL);

    /* init config space/mask to config address in words */
    for (i = 0; i < PCIEHCFGSZ; i += 2) {
        pciehcfg_setwm(pcfg, /*addr*/i, /*val*/i, /*msk*/i);
    }

    errors = 0;
#if 0
    pciehcfg_print_range(pcfg, 0, 16);
    pciehcfg_print_range(pcfg, PCIEHCFGSZ / 2, 16);
    pciehcfg_print_range(pcfg, PCIEHCFGSZ - 16, 16);
#endif
    // test beginning
    verbose("checking first 16 bytes, all sizes\n");
    errors += cfg_test_range(pcfg, 0, 16, 0);
    errors += cfg_test_range(pcfg, 0, 16, 1);

    // straddle midpoint (256) where addr overflows into second byte
    verbose("checking middle 16 bytes, all sizes\n");
    errors += cfg_test_range(pcfg, PCIEHCFGSZ / 2 - 8, 16, 0);
    errors += cfg_test_range(pcfg, PCIEHCFGSZ / 2 - 8, 16, 1);
    // test end of implemented range
    verbose("checking final 16 bytes, all sizes\n");
    errors += cfg_test_range(pcfg, PCIEHCFGSZ - 16, 16, 0);
    errors += cfg_test_range(pcfg, PCIEHCFGSZ - 16, 16, 1);

    verbose("test mask\n");
    pciehcfg_setdm(pcfg, 0, 0, 0xa5a5a5a5);
    pciehcfg_writed(pcfg, 0, 0xffffffff);
    if (pciehcfg_readd(pcfg, 0) != 0xa5a5a5a5) {
        printf("mask test failed, expected 0x%x got 0x%x\n",
               0xa5a5a5a5, pciehcfg_readd(pcfg, 0));
        errors++;
    }

#if 0
    pciehcfg_set_vendorid(pcfg, 0x1137);
    pciehcfg_set_deviceid(pcfg, 0x1234);
    pciehcfg_setd_with_mask(pcfg, 0x20, 0x12345678, 0xffffffff);
    verbose("vendor/device and 0x20 w/mask\n");
    pciehcfg_print(pcfg);
#endif

    pciehcfg_delete(pcfg);
    printf("%s\n", errors ? "FAILED" : "passed");
    exit(0);
}

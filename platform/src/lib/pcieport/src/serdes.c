/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdlib.h>
#include <assert.h>

#include "cap_sw_glue.h"
#include "cap_pcie_api.h"

#include "pciehsys.h"
#include "pcieport.h"
#include "pcieport_impl.h"

#define SBUS_ROM_MAGIC 0x53554253

struct sbus_hdr {
    uint32_t magic;
    uint32_t nwords;
};

struct rom_ctx_s {
    const uint8_t *buf;
    int nwords;
    int shift;
};

// 00000000_11111111_22222222_33333333_44444444
// 98765432_10......     9876_543210..
//            987654_3210....       98_76543210
int
romfile_read(void *ctx, unsigned int *datap)
{
    struct rom_ctx_s *p = (struct rom_ctx_s *)ctx;
    int s2 = p->shift * 2;

    if (!p->nwords) {
        return 0;
    }
    --p->nwords;
    *datap = (((uint32_t)p->buf[p->shift] & (0xff >> s2)) << (2 + s2)) |
              (p->buf[p->shift + 1] >> (6 - s2));
    p->shift = (p->shift + 1) & 0x3;
    if (p->shift == 0) {
        p->buf += 5;
    }
    return 1;
}

void *
romfile_open(void *rom_info)
{
    pal_reg_trace("================ romfile_open\n");
    return rom_info;
}

void
romfile_close(void *ctx)
{
    pal_reg_trace("================ romfile_close\n");
}

static int
pcieport_serdes_fw_gen(void)
{
    int gen = 1;
    const char *env = getenv("PCIEPORT_SERDES_FW_GEN");
    if (env) {
        gen = strtoul(env, NULL, 0);
    }
    return gen;
}

void
pcieport_serdes_init(void)
{
    /*
     * Until we figure out how to teach bazel to build with the
     * generated sbus_pcie_rom we'll leave it out of the bazel build
     * and include it in the make build by having make add -DSBUS_PCIE_ROM.
     */
#ifdef SBUS_PCIE_ROM
    extern uint8_t sbus_pcie_rom_start[];
    const struct sbus_hdr *hdr;
    struct rom_ctx_s ctx;
    int gen;

    hdr = (struct sbus_hdr *)sbus_pcie_rom_start;
    pciehsys_log("pcie sbus ROM @ %p", hdr);
    if (hdr->magic != SBUS_ROM_MAGIC) {
        pciehsys_log(", bad magic\n");
        return;
    }
    pciehsys_log(", good magic.  %d words\n", hdr->nwords);

    ctx.buf = (uint8_t *)(hdr + 1);
    ctx.nwords = hdr->nwords;
    ctx.shift = 0;

    gen = pcieport_serdes_fw_gen();
    pal_reg_trace("================ cap_pcie_serdes_setup start\n");
    cap_pcie_serdes_setup(0, 0, gen == 1, &ctx);
    pal_reg_trace("================ cap_pcie_serdes_setup end\n");
#else
    pciehsys_error("pcieport_serdes_init: missing serdes rom!!!\n");
    assert(0);
    if (0) pcieport_serdes_fw_gen();
#endif
}

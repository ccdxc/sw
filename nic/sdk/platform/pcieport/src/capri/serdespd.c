/*
 * Copyright (c) 2018-2020, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <inttypes.h>

#include "cap_sw_glue.h"
#include "cap_pcie_api.h"

#include "platform/pal/include/pal.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "platform/pcieport/include/pcieport.h"
#include "pcieportpd.h"

static uint64_t
pp_pcsd_interrupt_addr(const int lane)
{
    return PP_(CFG_PP_PCSD_INTERRUPT, 0) + lane * 4;
}

static uint16_t
pciesd_poll_interrupt_in_progress(const uint16_t want)
{
    const int maxpolls = 100;
    uint16_t inprog;
    int polls = 0;

    /* check once, if not done immediately then poll loop */
    inprog = pal_reg_rd32(PP_(STA_PP_PCSD_INTERRUPT_IN_PROGRESS, 0));
    while (inprog != want && ++polls < maxpolls) {
        usleep(1000);
        inprog = pal_reg_rd32(PP_(STA_PP_PCSD_INTERRUPT_IN_PROGRESS, 0));
    }

    if (inprog != want) {
        pciesys_loginfo("interrupt timeout (want 0x%04x, got 0x%04x)\n",
                        want, inprog);
        /* continue */
    }

    return inprog;
}

void
pciesd_core_interrupt(const uint16_t lanemask,
                      const uint16_t code,
                      const uint16_t data,
                      laneinfo_t *dataout)
{
    const uint32_t codedata = ((uint32_t)data << 16) | code;

    /* set up interrupt code/data */
    for (int i = 0; i < 16; i++) {
        const uint16_t lanebit = 1 << i;

        if (lanemask & lanebit) {
            pal_reg_wr32(pp_pcsd_interrupt_addr(i), codedata);
        }
    }

    /* issue interrupt request */
    pal_reg_wr32(PP_(CFG_PP_PCSD_INTERRUPT_REQUEST, 0), lanemask);

    /* wait for interrupt-in-progress */
    pciesd_poll_interrupt_in_progress(lanemask);

    /* clear interrupt request */
    pal_reg_wr32(PP_(CFG_PP_PCSD_INTERRUPT_REQUEST, 0), 0);

    /* wait for interrupt-complete */
    pciesd_poll_interrupt_in_progress(0);

    /* read interrupt response data */
    pal_reg_rd32w(PP_(STA_PP_PCSD_INTERRUPT_DATA_OUT, 0), dataout->w, 8);
}

static void
pcie_sta_pp_sd_core_status(laneinfo_t *li)
{
    pal_reg_rd32w(PP_(STA_PP_SD_CORE_STATUS, 0), li->w, 8);
}

/*
 * Read core status and figure out which pcie serdes lanes of lanemask
 * have the "ready" bit set.  See the Avago serdes documentation.
 */
uint16_t
pciesd_lanes_ready(const uint16_t lanemask)
{
    laneinfo_t st;
    uint16_t lanes_ready = 0;

    pcie_sta_pp_sd_core_status(&st);
    for (int i = 0; i < 16; i++) {
        const uint16_t lanebit = 1 << i;

        if (lanemask & lanebit) {
            if (st.lane[i] & (1 << 5)) {
                lanes_ready |= lanebit;
            }
        }
    }
    return lanes_ready;
}

static uint16_t
pcieport_serdes_crc_check(const uint16_t lanemask)
{
    laneinfo_t result;
    uint16_t lanepass;
    int i;

    pciesys_sbus_lock();
    pciesd_core_interrupt(lanemask, 0x3c, 0, &result);
    pciesys_sbus_unlock();

    lanepass = 0;
    for (i = 0; i < 16; i++) {
        const uint16_t lanebit = 1 << i;

        if (lanemask & lanebit) {
            if (result.lane[i] == 0) {
                lanepass |= lanebit;
            }
        }
    }
    return lanepass;
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
    int gen = 4;
    const char *env = getenv("PCIEPORT_SERDES_FW_GEN");
    if (env) {
        gen = strtoul(env, NULL, 0);
    }
    return gen;
}

#define SERDESFW_GEN(mac) \
    mac(0x1094_2347) \
    mac(0x10AA_2347)

static uint8_t *
serdes_lookup(const char *name)
{
    /* generate extern declaration of serdesfw symbol */
#define ext_decl(n) \
    extern uint8_t sbus_pcie_rom_ ## n ## _start[];
    SERDESFW_GEN(ext_decl);

    /* generate serdesfw table */
    static struct serdes_entry {
        const char *name;
        uint8_t *start;
    } serdes_table[] = {
#define serdes_table_ent(n) \
        { #n, sbus_pcie_rom_ ## n ## _start },
        SERDESFW_GEN(serdes_table_ent)
        { NULL, 0 }
    };
    struct serdes_entry *ent;

    /* search serdesfw table for "name" */
    for (ent = serdes_table; ent->name; ent++) {
        if (strcmp(name, ent->name) == 0) {
            return ent->start;
        }
    }
    return NULL;
}

int
pcieportpd_serdes_init(void)
{
    extern uint8_t sbus_pcie_rom_start[];
    const char *s = getenv("PCIE_SERDESFW");
    const struct sbus_hdr {
        uint32_t magic;
        uint32_t nwords;
    } *hdr;
    struct rom_ctx_s ctx;
    int r, gen;

    if (s == NULL) {
        hdr = (struct sbus_hdr *)sbus_pcie_rom_start;
    } else if ((hdr = (struct sbus_hdr *)serdes_lookup(s)) != 0) {
        pciesys_loginfo("$PCIE_SERDESFW selects %s\n", s);
    } else {
        pciesys_loginfo("$PCIE_SERDESFW bad value: %s (using default)\n", s);
        hdr = (struct sbus_hdr *)sbus_pcie_rom_start;
    }
    pciesys_loginfo("pcie sbus ROM @ %p", hdr);
    if (hdr->magic != SBUS_ROM_MAGIC) {
        pciesys_loginfo(", bad magic got 0x%x want 0x%x\n",
                        hdr->magic, SBUS_ROM_MAGIC);
        return -1;
    }
    pciesys_loginfo(", good magic.  %d words\n", hdr->nwords);

    ctx.buf = (uint32_t *)(hdr + 1);
    ctx.nwords = hdr->nwords;

    gen = pcieport_serdes_fw_gen();
    pal_reg_trace("================ cap_pcie_serdes_setup start\n");
    pciesys_sbus_lock();
    r = cap_pcie_serdes_setup(0, 0, gen == 1, &ctx);
    pciesys_sbus_unlock();

    /* verify crc */
    if (r >= 0) {
        const uint16_t lanemask = 0xffff;
        uint16_t lanepass = pcieport_serdes_crc_check(lanemask);
        if (lanepass != lanemask) {
            pciesys_logerror("serdes_init crc failed: "
                             "want 0x%04x got 0x%04x\n", lanemask, lanepass);
            r = -1;
        }
    }
    pal_reg_trace("================ cap_pcie_serdes_setup end %d\n", r);
    return r;
}

int
pcieportpd_serdes_reset(void)
{
    cap_pcie_serdes_reset(0, 0);
    return 0;
}

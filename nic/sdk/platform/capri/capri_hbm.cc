// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include <unistd.h>
#include <iostream>
#include "include/sdk/base.hpp"
#include "asic/rw/asicrw.hpp"
#include "platform/capri/capri_hbm_rw.hpp"
#include "platform/capri/capri_common.hpp"
#include "platform/capri/capri_state.hpp"
#include "third-party/asic/capri/model/utils/cap_blk_reg_model.h"
#include "third-party/asic/capri/model/cap_pic/cap_pics_csr.h"
#include "third-party/asic/capri/model/cap_wa/cap_wa_csr.h"
#include "third-party/asic/capri/model/cap_pcie/cap_pxb_csr.h"
#include "third-party/asic/capri/model/cap_top/cap_top_csr_defines.h"
#include "third-party/asic/capri/verif/apis/cap_nx_api.h"
#include "third-party/asic/capri/model/cap_top/cap_top_csr.h"
#include "third-party/asic/capri/model/cap_pic/cap_pics_csr.h"
#include "third-party/asic/capri/model/cap_ms/cap_ms_csr.h"
#include "third-party/asic/capri/model/cap_pcie/cap_pxb_csr.h"
#include "lib/pal/pal.hpp"

namespace sdk {
namespace platform {
namespace capri {

static uint32_t capri_freq = 0;

static inline sdk_ret_t
cap_nx_block_write(uint32_t chip, uint64_t addr, int size,
                   uint32_t *data_in , bool no_zero_time,
                   uint32_t flags)
{
    return sdk::asic::asic_reg_write(addr, data_in, 1,
                                        ASIC_WRITE_MODE_BLOCKING);
}

static inline uint32_t
cap_nx_block_read(uint32_t chip, uint64_t addr, int size,
                  bool no_zero_time, uint32_t flags)
{
    uint32_t data = 0x0;
    if (sdk::asic::asic_reg_read(addr, &data, 1, false /*read_thru*/) !=
                                 SDK_RET_OK) {
        SDK_TRACE_ERR("NX read failed. addr: %llx", addr);
    }
    return data;
}

unsigned int
cap_nx_read_pb_axi_cnt(int rd) { // 1=>rd , 0=> wr
    uint32_t rd_data1[2];
    uint32_t rd_data0[2];
    if (rd == 1) {
        rd_data0[0] = cap_nx_block_read(0, RBM_BRIDGE_(PB_M_3_150_AM_CNTR0), 1, false, 1);
        return (unsigned int)rd_data0[0];
    } else {
        rd_data1[0] = cap_nx_block_read(0, RBM_BRIDGE_(PB_M_3_150_AM_CNTR1), 1, false, 1);
        return (unsigned int)rd_data1[0];
    }
}

mem_addr_t
capri_get_mem_base (void)
{
    return g_capri_state_pd->mempartition()->base();
}

mem_addr_t
capri_get_mem_offset (const char *reg_name)
{
    return g_capri_state_pd->mempartition()->start_offset(reg_name);
}

mem_addr_t
capri_get_mem_addr (const char *reg_name)
{
    return g_capri_state_pd->mempartition()->start_addr(reg_name);
}

uint32_t
capri_get_mem_size_kb (const char *reg_name)
{
    return (g_capri_state_pd->mempartition()->size(reg_name) >> 10 );
}

mpartition_region_t *
capri_get_mem_region (char *reg_name)
{
    return g_capri_state_pd->mempartition()->region(reg_name);
}

mpartition_region_t *
capri_get_hbm_region_by_address (uint64_t addr)
{
    return g_capri_state_pd->mempartition()->region_by_address(addr);
}
// for HW platform this is now done during uboot
void
reset_hbm_regions (capri_cfg_t *capri_cfg)
{
    mpartition_region_t *reg;
    mem_addr_t va, pa;

    if (!capri_cfg)
        return;

    if (capri_cfg->platform != platform_type_t::PLATFORM_TYPE_HAPS &&
        capri_cfg->platform != platform_type_t::PLATFORM_TYPE_HW) {
        return;
    }

    for (int i = 0; i < g_capri_state_pd->mempartition()->num_regions(); i++) {
        reg = g_capri_state_pd->mempartition()->region(i);
        if (reg->reset) {
            // Reset only for haps
            SDK_TRACE_DEBUG("Resetting %s hbm region", reg->mem_reg_name);

            pa = g_capri_state_pd->mempartition()->addr(reg->start_offset);
            va = (mem_addr_t)sdk::lib::pal_mem_map(pa, reg->size);
            if (va) {
                memset((void *)va, 0, reg->size);
                sdk::lib::pal_mem_unmap((void *)va);
            } else {
                uint8_t zeros[1024] = {0};
                int64_t rem = reg->size;
                while (rem > 0) {
                    sdk::asic::asic_mem_write(pa, zeros, (uint64_t)rem > sizeof(zeros) ? sizeof(zeros) : rem);
                    pa += sizeof(zeros);
                    rem -= sizeof(zeros);
                }
            }
            SDK_TRACE_DEBUG("Resetting %s hbm region done", reg->mem_reg_name);
        }
    }
}

static sdk_ret_t
capri_hbm_llc_cache_init (capri_cfg_t *cfg)
{
    if (cfg == NULL || cfg->llc_cache == true) {
        SDK_TRACE_DEBUG("Enabling HBM LLC cache.");
        //cap_nx_cache_enable();
    } else {
        SDK_TRACE_DEBUG("Disabling HBM LLC cache.");
        //cap_nx_cache_disable();
    }

    return SDK_RET_OK;
}

static sdk_ret_t
capri_hbm_p4_cache_init (capri_cfg_t *cfg)
{
    uint32_t global_bypass = 0;
    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();

    if (cfg == NULL || cfg->p4_cache == false) {
        SDK_TRACE_DEBUG("Disabling HBM P4 cache based on HAL config.");
        global_bypass = 1;
    }

    // Enable P4 Ingress (inst_id = 1)
    cap_pics_csr_t & ig_pics_csr = cap0.ssi.pics;
    ig_pics_csr.picc.cfg_cache_global.bypass(global_bypass);
    ig_pics_csr.picc.cfg_cache_global.hash_mode(0);
    ig_pics_csr.picc.cfg_cache_global.write();

    // Invalidate P4IG cache
    ig_pics_csr.picc.dhs_cache_invalidate.entry.read();
    ig_pics_csr.picc.dhs_cache_invalidate.entry.inval_all(1);
    ig_pics_csr.picc.dhs_cache_invalidate.entry.write();

    // Invalidate MPU icaches
    for (uint8_t stage = 0; stage < 6; stage++) {
        cap0.sgi.mpu[stage].icache.read();
        cap0.sgi.mpu[stage].icache.invalidate(1);
        cap0.sgi.mpu[stage].icache.write();
    }

    // Enable P4 Egress (inst_id = 2)
    cap_pics_csr_t & eg_pics_csr = cap0.sse.pics;
    eg_pics_csr.picc.cfg_cache_global.bypass(global_bypass);
    eg_pics_csr.picc.cfg_cache_global.hash_mode(0);
    eg_pics_csr.picc.cfg_cache_global.write();

    // Invalidate P4EG cache
    eg_pics_csr.picc.dhs_cache_invalidate.entry.read();
    eg_pics_csr.picc.dhs_cache_invalidate.entry.inval_all(1);
    eg_pics_csr.picc.dhs_cache_invalidate.entry.write();

    // Invalidate MPU icaches
    for (uint8_t stage = 0; stage < 6; stage++) {
        cap0.sge.mpu[stage].icache.read();
        cap0.sge.mpu[stage].icache.invalidate(1);
        cap0.sge.mpu[stage].icache.write();
    }

    return SDK_RET_OK;
}

static sdk_ret_t
capri_hbm_p4plus_cache_init (capri_cfg_t *cfg)
{
    uint32_t global_bypass = 0;
    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();

    if (cfg == NULL || cfg->p4plus_cache == false) {
        SDK_TRACE_DEBUG("Disabling HBM P4Plus cache based on HAL config.");
        global_bypass = 1;
    }

    // Enable P4Plus RXDMA (inst_id = 0)
    cap_pics_csr_t & rxdma_pics_csr = cap0.rpc.pics;
    rxdma_pics_csr.picc.cfg_cache_global.bypass(global_bypass);
    rxdma_pics_csr.picc.cfg_cache_global.hash_mode(0);
    rxdma_pics_csr.picc.cfg_cache_global.write();

    // Invalidate RXDMA cache
    rxdma_pics_csr.picc.dhs_cache_invalidate.entry.read();
    rxdma_pics_csr.picc.dhs_cache_invalidate.entry.inval_all(1);
    rxdma_pics_csr.picc.dhs_cache_invalidate.entry.write();

    // Invalidate MPU icaches
    for (uint8_t stage = 0; stage < 8; stage++) {
        cap0.pcr.mpu[stage].icache.read();
        cap0.pcr.mpu[stage].icache.invalidate(1);
        cap0.pcr.mpu[stage].icache.write();
    }

    // Enable P4Plus TXDMA (inst_id = 3)
    cap_pics_csr_t & txdma_pics_csr = cap0.tpc.pics;
    txdma_pics_csr.picc.cfg_cache_global.bypass(global_bypass);
    txdma_pics_csr.picc.cfg_cache_global.hash_mode(0);
    txdma_pics_csr.picc.cfg_cache_global.write();

    // Invalidate TXDMA cache
    txdma_pics_csr.picc.dhs_cache_invalidate.entry.read();
    txdma_pics_csr.picc.dhs_cache_invalidate.entry.inval_all(1);
    txdma_pics_csr.picc.dhs_cache_invalidate.entry.write();

    // Invalidate MPU icaches
    for (uint8_t stage = 0; stage < 8; stage++) {
        cap0.pct.mpu[stage].icache.read();
        cap0.pct.mpu[stage].icache.invalidate(1);
        cap0.pct.mpu[stage].icache.write();
    }

    return SDK_RET_OK;
}

sdk_ret_t
capri_hbm_cache_init (capri_cfg_t *cfg)
{
    sdk_ret_t   ret;

    ret = capri_hbm_llc_cache_init(cfg);
    assert(ret == SDK_RET_OK);

    ret = capri_hbm_p4_cache_init(cfg);
    assert(ret == SDK_RET_OK);

    ret = capri_hbm_p4plus_cache_init(cfg);
    assert(ret == SDK_RET_OK);

    return SDK_RET_OK;
}

sdk_ret_t
capri_hbm_cache_program_region (mpartition_region_t *reg,
                                uint32_t inst_id,
                                uint32_t filter_idx,
                                bool slave,
                                bool master)
{
    cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, 0, inst_id);
    if (slave) {
        pics_csr.picc.filter_addr_lo_s.data[filter_idx].read();
        pics_csr.picc.filter_addr_lo_s.data[filter_idx].value(
            g_capri_state_pd->mempartition()->addr(reg->start_offset) >> 6);
        pics_csr.picc.filter_addr_lo_s.data[filter_idx].write();

        assert((reg->size >= 64) && ((reg->size & 0x3F) == 0));
        pics_csr.picc.filter_addr_hi_s.data[filter_idx].read();
        pics_csr.picc.filter_addr_hi_s.data[filter_idx].value(
            (g_capri_state_pd->mempartition()->addr(reg->start_offset) +
             (reg->size - 64)) >> 6);
        pics_csr.picc.filter_addr_hi_s.data[filter_idx].write();

        pics_csr.picc.filter_addr_ctl_s.value[filter_idx].read();
        // set Valid + CacheEnable + Invalidate&Fill (has ASIC bug so don't
        // enable this Invalidate&Fill) + Invalidate+Send
        pics_csr.picc.filter_addr_ctl_s.value[filter_idx].value(0xd);
        pics_csr.picc.filter_addr_ctl_s.value[filter_idx].write();
    }

    if (master) {
        pics_csr.picc.filter_addr_lo_m.data[filter_idx].read();
        pics_csr.picc.filter_addr_lo_m.data[filter_idx].value(
            g_capri_state_pd->mempartition()->addr(reg->start_offset) >> 6);
        pics_csr.picc.filter_addr_lo_m.data[filter_idx].write();

        assert((reg->size >= 64) && ((reg->size & 0x3F) == 0));
        pics_csr.picc.filter_addr_hi_m.data[filter_idx].read();
        pics_csr.picc.filter_addr_hi_m.data[filter_idx].value(
            (g_capri_state_pd->mempartition()->addr(reg->start_offset) +
             (reg->size - 64)) >> 6);
        pics_csr.picc.filter_addr_hi_m.data[filter_idx].write();

        pics_csr.picc.filter_addr_ctl_m.value[filter_idx].read();
        // set Valid + CacheEnable + Invalidate&Fill (has ASIC bug so don't
        // enable this Invalidate&Fill) + Invalidate+Send
        pics_csr.picc.filter_addr_ctl_m.value[filter_idx].value(0xd);
        pics_csr.picc.filter_addr_ctl_m.value[filter_idx].write();
    }

    return SDK_RET_OK;
}

sdk_ret_t
capri_hbm_cache_program_db (mpartition_region_t *reg,
                            uint32_t filter_idx)
{
    cap_wa_csr_t & wa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_wa_csr_t, 0, 0);

    wa_csr.filter_addr_lo.data[filter_idx].read();
    wa_csr.filter_addr_lo.data[filter_idx].value(
        g_capri_state_pd->mempartition()->addr(reg->start_offset) >> 6);
    wa_csr.filter_addr_lo.data[filter_idx].write();

    assert((reg->size >= 64) && ((reg->size & 0x3F) == 0));
    wa_csr.filter_addr_hi.data[filter_idx].read();
    wa_csr.filter_addr_hi.data[filter_idx].value(
        (g_capri_state_pd->mempartition()->addr(reg->start_offset) +
         (reg->size - 64)) >> 6);
    wa_csr.filter_addr_hi.data[filter_idx].write();

    wa_csr.filter_addr_ctl.value[filter_idx].read();
    // set Valid + CacheEnable + Invalidate&Fill (has ASIC bug so don't enable
    // this Invalidate&Fill) + Invalidate+Send
    wa_csr.filter_addr_ctl.value[filter_idx].value(0xd);
    wa_csr.filter_addr_ctl.value[filter_idx].write();

    return SDK_RET_OK;
}

sdk_ret_t
capri_hbm_cache_program_pcie (mpartition_region_t *reg,
                              uint32_t filter_idx)
{
    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, 0, 0);

    pxb_csr.filter_addr_lo.data[filter_idx].read();
    pxb_csr.filter_addr_lo.data[filter_idx].value(
        g_capri_state_pd->mempartition()->addr(reg->start_offset) >> 6);
    pxb_csr.filter_addr_lo.data[filter_idx].write();

    assert((reg->size >= 64) && ((reg->size & 0x3F) == 0));
    pxb_csr.filter_addr_hi.data[filter_idx].read();
    pxb_csr.filter_addr_hi.data[filter_idx].value(
        (g_capri_state_pd->mempartition()->addr(reg->start_offset) +
         (reg->size - 64)) >> 6);
    pxb_csr.filter_addr_hi.data[filter_idx].write();

    pxb_csr.filter_addr_ctl.value[filter_idx].read();
    // set Valid + CacheEnable + Invalidate&Fill (has ASIC bug so don't enable
    // this Invalidate&Fill) + Invalidate+Send
    pxb_csr.filter_addr_ctl.value[filter_idx].value(0xd);
    pxb_csr.filter_addr_ctl.value[filter_idx].write();

    return SDK_RET_OK;
}

sdk_ret_t
capri_hbm_cache_regions_init (void)
{
    mpartition_region_t *reg;
    mpartition_region_t p4ig_reg;
    mpartition_region_t p4eg_reg;
    mpartition_region_t p4plus_rxdma_reg;
    mpartition_region_t p4plus_txdma_reg;
    uint32_t            p4ig_filter_idx = 0;
    uint32_t            p4eg_filter_idx = 0;
    uint32_t            p4plus_txdma_filter_idx = 0;
    uint32_t            p4plus_rxdma_filter_idx = 0;
    uint32_t            p4plus_pcie_filter_idx = 0;
    uint32_t            p4plus_db_filter_idx = 0;

    memset(&p4ig_reg, 0, sizeof(p4ig_reg));
    memset(&p4eg_reg, 0, sizeof(p4eg_reg));
    memset(&p4plus_rxdma_reg, 0, sizeof(p4plus_rxdma_reg));
    memset(&p4plus_txdma_reg, 0, sizeof(p4plus_txdma_reg));
    for (int i = 0; i < g_capri_state_pd->mempartition()->num_regions(); i++) {
        reg = g_capri_state_pd->mempartition()->region(i);
        if (is_region_cache_pipe_none(reg) || is_region_an_alias(reg)) {
            continue;
        }

        if (is_region_cache_pipe_p4_ig(reg)) {
            if ((p4ig_reg.start_offset == 0) ||
                ((p4ig_reg.start_offset + p4ig_reg.size) != (reg->start_offset))) {
                SDK_TRACE_DEBUG("Programming %s to P4IG cache(region 1), "
                                "start=%lx size=%u index=%u", reg->mem_reg_name,
                                g_capri_state_pd->mempartition()->addr(reg->start_offset),
                                reg->size, p4ig_filter_idx);
                p4ig_reg.start_offset = reg->start_offset;
                p4ig_reg.size = reg->size;
                capri_hbm_cache_program_region(&p4ig_reg, 1, p4ig_filter_idx, 1, 0);
                p4ig_filter_idx++;
            } else {
                p4ig_reg.size += reg->size;
                SDK_TRACE_DEBUG("Programming %s to P4IG cache(region 1) (merge), "
                                "start=%lx size=%u index=%u", reg->mem_reg_name,
                                g_capri_state_pd->mempartition()->addr(p4ig_reg.start_offset),
                                p4ig_reg.size, p4ig_filter_idx - 1);
                capri_hbm_cache_program_region(&p4ig_reg, 1, p4ig_filter_idx - 1, 1, 0);
            }
        }

        if (is_region_cache_pipe_p4_eg(reg)) {
            if ((p4eg_reg.start_offset == 0) ||
                ((p4eg_reg.start_offset + p4eg_reg.size) != (reg->start_offset))) {
                SDK_TRACE_DEBUG("Programming %s to P4EG cache(region 2), "
                                "start=%lx size=%u index=%u", reg->mem_reg_name,
                                g_capri_state_pd->mempartition()->addr(reg->start_offset),
                                reg->size, p4eg_filter_idx);
                p4eg_reg.start_offset = reg->start_offset;
                p4eg_reg.size = reg->size;
                capri_hbm_cache_program_region(&p4eg_reg, 2, p4eg_filter_idx, 1, 0);
                p4eg_filter_idx++;
            } else {
                p4eg_reg.size += reg->size;
                SDK_TRACE_DEBUG("Programming %s to P4EG cache(region 2) (merge), "
                                "start=%lx size=%u index=%u", reg->mem_reg_name,
                                g_capri_state_pd->mempartition()->addr(p4eg_reg.start_offset),
                                p4eg_reg.size, p4eg_filter_idx - 1);
                capri_hbm_cache_program_region(&p4eg_reg, 2, p4eg_filter_idx - 1, 1, 0);
            }
        }

        if (is_region_cache_pipe_p4plus_txdma(reg)) {
            if ((p4plus_txdma_reg.start_offset == 0) ||
                ((p4plus_txdma_reg.start_offset + p4plus_txdma_reg.size) != (reg->start_offset))) {
                SDK_TRACE_DEBUG("Programming %s to P4PLUS TXDMA cache(region 3), "
                                "start=%lx size=%u index=%u", reg->mem_reg_name,
                                g_capri_state_pd->mempartition()->addr(reg->start_offset),
                                reg->size, p4plus_txdma_filter_idx);
                p4plus_txdma_reg.start_offset = reg->start_offset;
                p4plus_txdma_reg.size = reg->size;
                capri_hbm_cache_program_region(&p4plus_txdma_reg, 3, p4plus_txdma_filter_idx, 1, 1);
                p4plus_txdma_filter_idx++;
            } else {
                p4plus_txdma_reg.size += reg->size;
                SDK_TRACE_DEBUG("Programming %s to P4PLUS TXDMA cache(region 3) (merge), "
                                "start=%lx size=%u index=%u", reg->mem_reg_name,
                                g_capri_state_pd->mempartition()->addr(p4plus_txdma_reg.start_offset),
                                p4plus_txdma_reg.size, p4plus_txdma_filter_idx - 1);
                capri_hbm_cache_program_region(&p4plus_txdma_reg, 3, p4plus_txdma_filter_idx - 1, 1, 1);
            }
        }

        if (is_region_cache_pipe_p4plus_rxdma(reg)) {
            if ((p4plus_rxdma_reg.start_offset == 0) ||
                ((p4plus_rxdma_reg.start_offset + p4plus_rxdma_reg.size) != (reg->start_offset))) {
                SDK_TRACE_DEBUG("Programming %s to P4PLUS RXDMA cache(region 0), "
                                "start=%lx size=%u index=%u", reg->mem_reg_name,
                                g_capri_state_pd->mempartition()->addr(reg->start_offset),
                                reg->size, p4plus_rxdma_filter_idx);
                p4plus_rxdma_reg.start_offset = reg->start_offset;
                p4plus_rxdma_reg.size = reg->size;
                capri_hbm_cache_program_region(&p4plus_rxdma_reg, 0, p4plus_rxdma_filter_idx, 1, 1);
                p4plus_rxdma_filter_idx++;
            } else {
                p4plus_rxdma_reg.size += reg->size;
                SDK_TRACE_DEBUG("Programming %s to P4PLUS RXDMA cache(region 0) (merge), "
                                "start=%lx size=%u index=%u", reg->mem_reg_name,
                                g_capri_state_pd->mempartition()->addr(p4plus_rxdma_reg.start_offset),
                                p4plus_rxdma_reg.size, p4plus_rxdma_filter_idx - 1);
                capri_hbm_cache_program_region(&p4plus_rxdma_reg, 0, p4plus_rxdma_filter_idx - 1, 1, 1);
            }
        }

        if (is_region_cache_pipe_p4plus_pciedb(reg)) {
            SDK_TRACE_DEBUG("Programming %s to PCIE, "
                            "start=%lx size=%u index=%u", reg->mem_reg_name,
                            g_capri_state_pd->mempartition()->addr(reg->start_offset),
                            reg->size, p4plus_pcie_filter_idx);
            capri_hbm_cache_program_pcie(reg, p4plus_pcie_filter_idx);
            p4plus_pcie_filter_idx++;

            SDK_TRACE_DEBUG("Programming %s to Doorbell, "
                            "start=%lx size=%u index=%u", reg->mem_reg_name,
                            g_capri_state_pd->mempartition()->addr(reg->start_offset),
                            reg->size, p4plus_db_filter_idx);
            capri_hbm_cache_program_db(reg, p4plus_db_filter_idx);
            p4plus_db_filter_idx++;
        }
    }

    return SDK_RET_OK;
}

uint64_t
capri_hbm_timestamp_get (void)
{
    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;

    pbc_csr.hbm.sta_hbm_timestamp.read();

    return pbc_csr.hbm.sta_hbm_timestamp.all().convert_to<uint64_t>();
}

sdk_ret_t
capri_tpc_bw_mon_rd_get (uint64_t *maxv, uint64_t *avrg)
{
    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();
    cap_pics_csr_t &pics_csr = cap0.tpc.pics;

    pics_csr.sta_axi_bw_mon_rd_bandwidth.read();

    *maxv = pics_csr.sta_axi_bw_mon_rd_bandwidth.maxv().convert_to<uint64_t>();
    *avrg = pics_csr.sta_axi_bw_mon_rd_bandwidth.avrg().convert_to<uint64_t>();

    SDK_TRACE_DEBUG("TXDMA AVG_RD: %llu, MAX_RD: %llu", *avrg, *maxv);

    return SDK_RET_OK;
}

sdk_ret_t
capri_tpc_bw_mon_wr_get (uint64_t *maxv, uint64_t *avrg)
{
    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();
    cap_pics_csr_t &pics_csr = cap0.tpc.pics;

    pics_csr.sta_axi_bw_mon_wr_bandwidth.read();

    *maxv = pics_csr.sta_axi_bw_mon_wr_bandwidth.maxv().convert_to<uint64_t>();
    *avrg = pics_csr.sta_axi_bw_mon_wr_bandwidth.avrg().convert_to<uint64_t>();

    SDK_TRACE_DEBUG("TXDMA AVG_WR: %llu, MAX_WR: %llu", *avrg, *maxv);

    return SDK_RET_OK;
}

sdk_ret_t
capri_rpc_bw_mon_rd_get (uint64_t *maxv, uint64_t *avrg)
{
    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();
    cap_pics_csr_t &pics_csr = cap0.rpc.pics;

    pics_csr.sta_axi_bw_mon_rd_bandwidth.read();

    *maxv = pics_csr.sta_axi_bw_mon_rd_bandwidth.maxv().convert_to<uint64_t>();
    *avrg = pics_csr.sta_axi_bw_mon_rd_bandwidth.avrg().convert_to<uint64_t>();

    SDK_TRACE_DEBUG("RXDMA AVG_RD: %llu, MAX_RD: %llu", *avrg, *maxv);

    return SDK_RET_OK;
}

sdk_ret_t
capri_rpc_bw_mon_wr_get (uint64_t *maxv, uint64_t *avrg)
{
    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();
    cap_pics_csr_t &pics_csr = cap0.rpc.pics;

    pics_csr.sta_axi_bw_mon_wr_bandwidth.read();

    *maxv = pics_csr.sta_axi_bw_mon_wr_bandwidth.maxv().convert_to<uint64_t>();
    *avrg = pics_csr.sta_axi_bw_mon_wr_bandwidth.avrg().convert_to<uint64_t>();

    SDK_TRACE_DEBUG("RXDMA AVG_WR: %llu, MAX_WR: %llu", *avrg, *maxv);

    return SDK_RET_OK;
}

sdk_ret_t
capri_ms_bw_mon_rd_get (uint64_t *maxv, uint64_t *avrg)
{
    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();
    cap_ms_csr_t &ms_csr = cap0.ms.ms;

    ms_csr.sta_axi_bw_mon_rd_bandwidth.read();

    *maxv = ms_csr.sta_axi_bw_mon_rd_bandwidth.maxv().convert_to<uint64_t>();
    *avrg = ms_csr.sta_axi_bw_mon_rd_bandwidth.avrg().convert_to<uint64_t>();

    return SDK_RET_OK;
}

sdk_ret_t
capri_ms_bw_mon_wr_get (uint64_t *maxv, uint64_t *avrg)
{
    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();
    cap_ms_csr_t &ms_csr = cap0.ms.ms;

    ms_csr.sta_axi_bw_mon_wr_bandwidth.read();

    *maxv = ms_csr.sta_axi_bw_mon_wr_bandwidth.maxv().convert_to<uint64_t>();
    *avrg = ms_csr.sta_axi_bw_mon_wr_bandwidth.avrg().convert_to<uint64_t>();

    return SDK_RET_OK;
}

sdk_ret_t
capri_pxb_bw_mon_rd_get (uint64_t *maxv, uint64_t *avrg)
{
    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();
    cap_pxb_csr_t &pxb_csr = cap0.pxb.pxb;

    pxb_csr.sta_axi_bw_mon_rd_bandwidth.read();

    *maxv = pxb_csr.sta_axi_bw_mon_rd_bandwidth.maxv().convert_to<uint64_t>();
    *avrg = pxb_csr.sta_axi_bw_mon_rd_bandwidth.avrg().convert_to<uint64_t>();

    return SDK_RET_OK;
}

sdk_ret_t
capri_pxb_bw_mon_wr_get (uint64_t *maxv, uint64_t *avrg)
{
    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();
    cap_pxb_csr_t &pxb_csr = cap0.pxb.pxb;

    pxb_csr.sta_axi_bw_mon_wr_bandwidth.read();

    *maxv = pxb_csr.sta_axi_bw_mon_wr_bandwidth.maxv().convert_to<uint64_t>();
    *avrg = pxb_csr.sta_axi_bw_mon_wr_bandwidth.avrg().convert_to<uint64_t>();

    return SDK_RET_OK;
}

static inline void
populate_hbm_bw (uint64_t max_rd, uint64_t max_wr,
                 uint64_t avg_rd, uint64_t avg_wr,
                 asic_hbm_bw_t *hbm_bw,
                 uint32_t num_bits, uint32_t window_size)
{
    hbm_bw->max.read  = (max_rd * (num_bits * capri_freq)/1000.0f) / window_size;
    hbm_bw->max.write = (max_wr * (num_bits * capri_freq)/1000.0f) / window_size;
    hbm_bw->avg.read  = (avg_rd * (num_bits * capri_freq)/1000.0f) / window_size;
    hbm_bw->avg.write = (avg_wr * (num_bits * capri_freq)/1000.0f) / window_size;

    SDK_TRACE_DEBUG("AVG_RD: %llu, AVG_WR: %llu, "
                    "MAX_RD: %llu, MAX_WR: %llu",
                    hbm_bw->avg.read, hbm_bw->avg.write,
                    hbm_bw->max.read, hbm_bw->max.write);
}

uint32_t
capri_freq_get (void)
{
    uint64_t prev_ts     = 0;
    uint64_t cur_ts      = 0;
    int      delay       = 5000;

    prev_ts = capri_hbm_timestamp_get();
    usleep(delay * 1000);
    cur_ts  = capri_hbm_timestamp_get();

    return ((((cur_ts - prev_ts) * 1.0) / delay) * 1000) / 1000000.0;
}

static inline uint64_t
capri_pb_axi_read_cnt (void)
{
    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;

    pbc_csr.hbm.cnt_hbm_axi_rpl.read();
    pbc_csr.hbm.cnt_hbm_axi_pyld.read();
    pbc_csr.hbm.cnt_hbm_axi_ctrl.read();

    return pbc_csr.hbm.cnt_hbm_axi_rpl.all().convert_to<uint64_t>()
           + pbc_csr.hbm.cnt_hbm_axi_pyld.all().convert_to<uint64_t>()
           + pbc_csr.hbm.cnt_hbm_axi_ctrl.all().convert_to<uint64_t>();
}

static sdk_ret_t
capri_clear_hbm_bw (int val)
{
    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();

    cap_pics_csr_t &pics_csr = cap0.tpc.pics;
    pics_csr.cfg_axi_bw_mon.read();
    pics_csr.cfg_axi_bw_mon.alpha(val);
    pics_csr.cfg_axi_bw_mon.write();

    cap_pics_csr_t &rpc_pics_csr = cap0.rpc.pics;
    rpc_pics_csr.cfg_axi_bw_mon.read();
    rpc_pics_csr.cfg_axi_bw_mon.alpha(val);
    rpc_pics_csr.cfg_axi_bw_mon.write();

    cap_ms_csr_t &ms_csr = cap0.ms.ms;
    ms_csr.cfg_axi_bw_mon.read();
    ms_csr.cfg_axi_bw_mon.alpha(val);
    ms_csr.cfg_axi_bw_mon.write();

    cap_pxb_csr_t &pxb_csr = cap0.pxb.pxb;
    pxb_csr.cfg_axi_bw_mon.read();
    pxb_csr.cfg_axi_bw_mon.alpha(val);
    pxb_csr.cfg_axi_bw_mon.write();

    return SDK_RET_OK;
}

static sdk_ret_t
capri_set_hbm_bw_window (uint32_t val)
{
    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();

    cap_pics_csr_t &pics_csr = cap0.tpc.pics;
    pics_csr.cfg_axi_bw_mon.read();
    pics_csr.cfg_axi_bw_mon.cycle(val);
    pics_csr.cfg_axi_bw_mon.write();

    cap_pics_csr_t &rpc_pics_csr = cap0.rpc.pics;
    rpc_pics_csr.cfg_axi_bw_mon.read();
    rpc_pics_csr.cfg_axi_bw_mon.cycle(val);
    rpc_pics_csr.cfg_axi_bw_mon.write();

    cap_ms_csr_t &ms_csr = cap0.ms.ms;
    ms_csr.cfg_axi_bw_mon.read();
    ms_csr.cfg_axi_bw_mon.cycle(val);
    ms_csr.cfg_axi_bw_mon.write();

    cap_pxb_csr_t &pxb_csr = cap0.pxb.pxb;
    pxb_csr.cfg_axi_bw_mon.read();
    pxb_csr.cfg_axi_bw_mon.cycle(val);
    pxb_csr.cfg_axi_bw_mon.write();

    return SDK_RET_OK;
}

sdk_ret_t
capri_hbm_bw (uint32_t samples, uint32_t u_sleep, bool ms_pcie,
              asic_hbm_bw_t *hbm_bw_arr)
{
    uint64_t prev_ts     = 0;
    uint64_t cur_ts      = 0;
    uint64_t clk_diff    = 0;
    uint64_t max_rd      = 0;
    uint64_t avg_rd      = 0;
    uint64_t max_wr      = 0;
    uint64_t avg_wr      = 0;
    uint32_t num_bits    = 64 * 8;
    uint64_t rd_cnt      = 0;
    uint64_t wr_cnt      = 0;
    uint64_t prev_rd_cnt = 0;
    uint64_t prev_wr_cnt = 0;
    int      index       = 0;
    uint64_t cycle_per_nsec = 0;
    uint32_t window_size = 0xfff;
    asic_hbm_bw_t *hbm_bw = NULL;

    if (capri_freq == 0) {
        capri_freq = capri_freq_get();
        capri_set_hbm_bw_window(window_size);
        SDK_TRACE_DEBUG("HBM BW mon window size set to: 0x%x", window_size);
        SDK_TRACE_DEBUG("capri freq: %u", capri_freq);
    }

    capri_clear_hbm_bw(0);
    usleep(1000000);    // 1 sec
    capri_clear_hbm_bw(1);

    prev_ts = capri_hbm_timestamp_get();

    for (uint32_t i = 0; i < samples; ++i) {

        usleep(u_sleep);

        cur_ts = capri_hbm_timestamp_get();

        clk_diff = cur_ts - prev_ts;

        hbm_bw = &hbm_bw_arr[index++];
        hbm_bw->type = asic_block_t::ASIC_BLOCK_TXDMA;
        hbm_bw->clk_diff = clk_diff;
        capri_tpc_bw_mon_rd_get(&max_rd, &avg_rd);
        capri_tpc_bw_mon_wr_get(&max_wr, &avg_wr);
        SDK_TRACE_DEBUG("CLK_DIFF: %llu, TXDMA BW "
                        "AVG_RD: %llu, AVG_WR: %llu, "
                        "MAX_RD: %llu, MAX_WR: %llu",
                        clk_diff,
                        avg_rd, avg_wr,
                        max_rd, max_wr);
        populate_hbm_bw(max_rd, max_wr, avg_rd, avg_wr, hbm_bw, num_bits, window_size);

        hbm_bw = &hbm_bw_arr[index++];
        hbm_bw->type = asic_block_t::ASIC_BLOCK_RXDMA;
        hbm_bw->clk_diff = clk_diff;
        capri_rpc_bw_mon_rd_get(&max_rd, &avg_rd);
        capri_rpc_bw_mon_wr_get(&max_wr, &avg_wr);
        SDK_TRACE_DEBUG("CLK_DIFF: %llu, RXDMA BW "
                        "AVG_RD: %llu, AVG_WR: %llu, "
                        "MAX_RD: %llu, MAX_WR: %llu",
                        clk_diff,
                        avg_rd, avg_wr,
                        max_rd, max_wr);
        populate_hbm_bw(max_rd, max_wr, avg_rd, avg_wr, hbm_bw, num_bits, window_size);

        hbm_bw = &hbm_bw_arr[index++];
        hbm_bw->type = asic_block_t::ASIC_BLOCK_MS;
        hbm_bw->clk_diff = clk_diff;
        capri_ms_bw_mon_rd_get(&max_rd, &avg_rd);
        capri_ms_bw_mon_wr_get(&max_wr, &avg_wr);
        SDK_TRACE_DEBUG("CLK_DIFF: %llu, MS BW "
                        "AVG_RD: %llu, AVG_WR: %llu, "
                        "MAX_RD: %llu, MAX_WR: %llu",
                        clk_diff,
                        avg_rd, avg_wr,
                        max_rd, max_wr);
        populate_hbm_bw(max_rd, max_wr, avg_rd, avg_wr, hbm_bw, num_bits, window_size);

        hbm_bw = &hbm_bw_arr[index++];
        hbm_bw->type = asic_block_t::ASIC_BLOCK_PCIE;
        hbm_bw->clk_diff = clk_diff;
        capri_pxb_bw_mon_rd_get(&max_rd, &avg_rd);
        capri_pxb_bw_mon_wr_get(&max_wr, &avg_wr);
        SDK_TRACE_DEBUG("CLK_DIFF: %llu, PCIE BW "
                        "AVG_RD: %llu, AVG_WR: %llu, "
                        "MAX_RD: %llu, MAX_WR: %llu",
                        clk_diff,
                        avg_rd, avg_wr,
                        max_rd, max_wr);
        populate_hbm_bw(max_rd, max_wr, avg_rd, avg_wr, hbm_bw, num_bits, window_size);

        if (capri_freq != 0) {
            cycle_per_nsec = ((cur_ts - prev_ts) * 1000) / capri_freq;
        }

        hbm_bw = &hbm_bw_arr[index++];
        hbm_bw->type = asic_block_t::ASIC_BLOCK_PACKET_BUFFER;
        hbm_bw->clk_diff = clk_diff;
        rd_cnt = capri_pb_axi_read_cnt();
        wr_cnt = cap_nx_read_pb_axi_cnt(0);

        // avoid arithmetic exceptions
        if (cycle_per_nsec != 0) {
            avg_rd = ((rd_cnt - prev_rd_cnt) * num_bits) / cycle_per_nsec;
            avg_wr = ((wr_cnt - prev_wr_cnt) * num_bits) / cycle_per_nsec;
        }

        hbm_bw->avg.read  = avg_rd;
        hbm_bw->avg.write = avg_wr;
        SDK_TRACE_DEBUG("CLK_DIFF: %llu, PB BW "
                        "AVG_RD: %llu, AVG_WR: %llu",
                        clk_diff,
                        avg_rd, avg_wr);

        prev_rd_cnt = rd_cnt;
        prev_wr_cnt = wr_cnt;
        prev_ts     = cur_ts;
    }

    return SDK_RET_OK;
}

sdk_ret_t
capri_nx_set_llc_counters (uint32_t *data)
{
    for (int i = 0; i < 16; ++i) {
        switch (i) {
        case 0:
            cap_nx_block_write(0,
                    RBM_AGENT_(CCC0_LLC_EVENT_COUNTER), 1, data, true, 1);
            break;
        case 1:
            cap_nx_block_write(0,
                    RBM_AGENT_(CCC1_LLC_EVENT_COUNTER), 1, data, true, 1);
            break;
        case 2:
            cap_nx_block_write(0,
                    RBM_AGENT_(CCC2_LLC_EVENT_COUNTER), 1, data, true, 1);
            break;
        case 3:
            cap_nx_block_write(0,
                    RBM_AGENT_(CCC3_LLC_EVENT_COUNTER), 1, data, true, 1);
            break;
        case 4:
            cap_nx_block_write(0,
                    RBM_AGENT_(CCC4_LLC_EVENT_COUNTER), 1, data, true, 1);
            break;
        case 5:
            cap_nx_block_write(0,
                    RBM_AGENT_(CCC5_LLC_EVENT_COUNTER), 1, data, true, 1);
            break;
        case 6:
            cap_nx_block_write(0,
                    RBM_AGENT_(CCC6_LLC_EVENT_COUNTER), 1, data, true, 1);
            break;
        case 7:
            cap_nx_block_write(0,
                    RBM_AGENT_(CCC7_LLC_EVENT_COUNTER), 1, data, true, 1);
            break;
        case 8:
            cap_nx_block_write(0,
                    RBM_AGENT_(CCC8_LLC_EVENT_COUNTER), 1, data, true, 1);
            break;
        case 9:
            cap_nx_block_write(0,
                    RBM_AGENT_(CCC9_LLC_EVENT_COUNTER), 1, data, true, 1);
            break;
        case 10:
            cap_nx_block_write(0,
                    RBM_AGENT_(CCC10_LLC_EVENT_COUNTER), 1, data, true, 1);
            break;
        case 11:
            cap_nx_block_write(0,
                    RBM_AGENT_(CCC11_LLC_EVENT_COUNTER), 1, data, true, 1);
            break;
        case 12:
            cap_nx_block_write(0,
                    RBM_AGENT_(CCC12_LLC_EVENT_COUNTER), 1, data, true, 1);
            break;
        case 13:
            cap_nx_block_write(0,
                    RBM_AGENT_(CCC13_LLC_EVENT_COUNTER), 1, data, true, 1);
            break;
        case 14:
            cap_nx_block_write(0,
                    RBM_AGENT_(CCC14_LLC_EVENT_COUNTER), 1, data, true, 1);
            break;
        case 15:
            cap_nx_block_write(0,
                    RBM_AGENT_(CCC15_LLC_EVENT_COUNTER), 1, data, true, 1);
            break;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
capri_nx_get_llc_counters (uint32_t *mask, uint32_t *rd_data)
{
    for (int i = 0; i < 16; ++i) {
        switch (i) {
        case 0:
            rd_data[i] = cap_nx_block_read(0,
                        RBM_AGENT_(CCC0_LLC_EVENT_COUNTER), 1, false, 1);
            break;
        case 1:
            rd_data[i] = cap_nx_block_read(0,
                        RBM_AGENT_(CCC1_LLC_EVENT_COUNTER), 1, false, 1);
            break;
        case 2:
            rd_data[i] = cap_nx_block_read(0,
                        RBM_AGENT_(CCC2_LLC_EVENT_COUNTER), 1, false, 1);
            break;
        case 3:
            rd_data[i] = cap_nx_block_read(0,
                        RBM_AGENT_(CCC3_LLC_EVENT_COUNTER), 1, false, 1);
            break;
        case 4:
            rd_data[i] = cap_nx_block_read(0,
                        RBM_AGENT_(CCC4_LLC_EVENT_COUNTER), 1, false, 1);
            break;
        case 5:
            rd_data[i] = cap_nx_block_read(0,
                        RBM_AGENT_(CCC5_LLC_EVENT_COUNTER), 1, false, 1);
            break;
        case 6:
            rd_data[i] = cap_nx_block_read(0,
                        RBM_AGENT_(CCC6_LLC_EVENT_COUNTER), 1, false, 1);
            break;
        case 7:
            rd_data[i] = cap_nx_block_read(0,
                        RBM_AGENT_(CCC7_LLC_EVENT_COUNTER), 1, false, 1);
            break;
        case 8:
            rd_data[i] = cap_nx_block_read(0,
                        RBM_AGENT_(CCC8_LLC_EVENT_COUNTER), 1, false, 1);
            break;
        case 9:
            rd_data[i] = cap_nx_block_read(0,
                        RBM_AGENT_(CCC9_LLC_EVENT_COUNTER), 1, false, 1);
            break;
        case 10:
            rd_data[i] = cap_nx_block_read(0,
                        RBM_AGENT_(CCC10_LLC_EVENT_COUNTER), 1, false, 1);
            break;
        case 11:
            rd_data[i] = cap_nx_block_read(0,
                        RBM_AGENT_(CCC11_LLC_EVENT_COUNTER), 1, false, 1);
            break;
        case 12:
            rd_data[i] = cap_nx_block_read(0,
                        RBM_AGENT_(CCC12_LLC_EVENT_COUNTER), 1, false, 1);
            break;
        case 13:
            rd_data[i] = cap_nx_block_read(0,
                        RBM_AGENT_(CCC13_LLC_EVENT_COUNTER), 1, false, 1);
            break;
        case 14:
            rd_data[i] = cap_nx_block_read(0,
                        RBM_AGENT_(CCC14_LLC_EVENT_COUNTER), 1, false, 1);
            break;
        case 15:
            rd_data[i] = cap_nx_block_read(0,
                        RBM_AGENT_(CCC15_LLC_EVENT_COUNTER), 1, false, 1);
            break;
        }
    }

    // We only need to read the mask from the first counter. Currently mask is being set the same for all counters
    *mask = cap_nx_block_read(0,
                    RBM_AGENT_(CCC0_LLC_EVENT_COUNTER_MASK), 1, false, 1);

    return SDK_RET_OK;
}

sdk_ret_t
capri_nx_setup_llc_counters (uint32_t mask)
{
    /*
       e9[9] - Retry access
       e8[8] - Retry needed
       e7[7] - Eviction
       e6[6] - Cache maint op
       e5[5] - Partial write
       e4[4] - Cache miss
       e3[3] - Cache hit
       e2[2] - Scratchpad access
       e1[1] - Cache write
       e0[0] - Cache read
    */

    uint32_t *data = &mask;
    uint32_t reset = 0x0;

    if (mask == 0xffffffff) {
        capri_nx_set_llc_counters(&reset);
        return SDK_RET_OK;
    }

    cap_nx_block_write(0,
                RBM_AGENT_(CCC0_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0,
                RBM_AGENT_(CCC1_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0,
                RBM_AGENT_(CCC10_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0,
                RBM_AGENT_(CCC11_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0,
                RBM_AGENT_(CCC12_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0,
                RBM_AGENT_(CCC13_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0,
                RBM_AGENT_(CCC14_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0,
                RBM_AGENT_(CCC15_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0,
                RBM_AGENT_(CCC2_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0,
                RBM_AGENT_(CCC3_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0,
                RBM_AGENT_(CCC4_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0,
                RBM_AGENT_(CCC5_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0,
                RBM_AGENT_(CCC6_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0,
                RBM_AGENT_(CCC7_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0,
                RBM_AGENT_(CCC8_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0,
                RBM_AGENT_(CCC9_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);

    return SDK_RET_OK;
}

} // namespace capri
} // namespace platform
} // namespace sdk

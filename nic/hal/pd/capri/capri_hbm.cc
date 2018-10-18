#include "nic/include/base.hpp"
#include "nic/include/capri_common.h"
#include <unistd.h>
#include <iostream>
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/include/asic_pd.hpp"
#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include <arpa/inet.h>
#include "nic/include/hal.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/asic/capri/model/utils/cap_blk_reg_model.h"
#include "nic/asic/capri/model/cap_pic/cap_pics_csr.h"
#include "nic/asic/capri/model/cap_wa/cap_wa_csr.h"
#include "nic/asic/capri/model/cap_pcie/cap_pxb_csr.h"
#include "nic/asic/capri/model/cap_top/cap_top_csr_defines.h"
#include "nic/asic/capri/verif/apis/cap_nx_api.h"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/asic/capri/model/cap_pic/cap_pics_csr.h"
#include "nic/asic/capri/model/cap_ms/cap_ms_csr.h"
#include "nic/asic/capri/model/cap_pcie/cap_pxb_csr.h"

namespace pt = boost::property_tree;

using sdk::platform::capri;

capri g_capri;

static inline hal_ret_t
cap_nx_block_write(uint32_t chip, uint64_t addr, int size,
                   uint32_t *data_in , bool no_zero_time,
                   uint32_t flags)
{
    return hal::pd::asic_reg_write(addr, data_in, 1,
                                   hal::pd::ASIC_WRITE_MODE_BLOCKING);
}

static inline uint32_t
cap_nx_block_read(uint32_t chip, uint64_t addr, int size,
                  bool no_zero_time, uint32_t flags)
{
    uint32_t data = 0x0;
    if(hal::pd::asic_reg_read(addr, &data, 1, false /*read_thru*/) !=
                            HAL_RET_OK) {
        HAL_TRACE_ERR("NX read failed. addr: {}", addr);
    }
    return data;
}

hal_ret_t
capri_hbm_parse (capri_cfg_t *cfg)
{
    sdk_ret_t      sdk_ret;
    std::string             full_path;

    // makeup the full file path
    full_path =  cfg->cfg_path + "/" + cfg->pgm_name +
                     "/" + std::string("hbm_mem.json");
    sdk_ret = g_capri.capri_hbm_parse(full_path.c_str());
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

hbm_addr_t
get_hbm_base (void)
{
    return g_capri.get_hbm_base();
}

hbm_addr_t
get_hbm_offset (const char *reg_name)
{
    return g_capri.get_hbm_offset(reg_name);
}

hbm_addr_t
get_start_offset (const char *reg_name)
{
    return g_capri.get_start_offset(reg_name);
}

uint32_t
get_size_kb (const char *reg_name)
{
    return g_capri.get_size_kb(reg_name);
}

capri_hbm_region_t *
get_hbm_region (char *reg_name)
{
    return g_capri.get_hbm_region(reg_name);
}

capri_hbm_region_t *
get_hbm_region_by_address (uint64_t addr)
{
    return g_capri.get_hbm_region_by_address(addr);
}

void
reset_hbm_regions (void)
{
    hal::hal_cfg_t        *hal_cfg;
    capri_hbm_region_t    *reg;

    hal_cfg = (hal::hal_cfg_t *)hal::hal_get_current_thread()->data();
    if (hal_cfg &&
        ((hal_cfg->platform_mode == hal::HAL_PLATFORM_MODE_HAPS) ||
         (hal_cfg->platform_mode == hal::HAL_PLATFORM_MODE_HW))) {
        for (int i = 0; i < g_capri.num_hbm_regions(); i++) {
            reg = g_capri.hbm_region(i);
            if (reg->reset) {
                HAL_TRACE_DEBUG("Resetting {} hbm region", reg->mem_reg_name);
                hal::pd::asic_mem_write(HBM_OFFSET(reg->start_offset),
                                        NULL, reg->size_kb * 1024);
            }
        }
    }
}

int32_t
capri_hbm_read_mem (uint64_t addr, uint8_t *buf, uint32_t size)
{
    hal_ret_t rc = hal::pd::asic_mem_read(addr, buf, size);
    return (rc == HAL_RET_OK) ? 0 : -EIO;
}

int32_t
capri_hbm_write_mem (uint64_t addr, const uint8_t *buf, uint32_t size)
{
    hal_ret_t rc = hal::pd::asic_mem_write(addr, (uint8_t *)buf, size,
                                           hal::pd::ASIC_WRITE_MODE_BLOCKING);
    return (rc == HAL_RET_OK) ? 0 : -EIO;
}

static hal_ret_t
capri_hbm_llc_cache_init (capri_cfg_t *cfg)
{
    if (cfg == NULL || cfg->llc_cache == true) {
        HAL_TRACE_DEBUG("Enabling HBM LLC cache.");
        cap_nx_cache_enable();
    } else {
        HAL_TRACE_DEBUG("Disabling HBM LLC cache.");
        cap_nx_cache_disable();
    }

    return HAL_RET_OK;
}

static hal_ret_t
capri_hbm_p4_cache_init (capri_cfg_t *cfg)
{
    uint32_t global_bypass = 0;

    if (cfg == NULL || cfg->p4_cache == false) {
        HAL_TRACE_DEBUG("Disabling HBM P4 cache based on HAL config.");
        global_bypass = 1;
    }

    // Enable P4 Ingress (inst_id = 1)
    cap_pics_csr_t & ig_pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, 0, 1);
    ig_pics_csr.picc.cfg_cache_global.bypass(global_bypass);
    ig_pics_csr.picc.cfg_cache_global.hash_mode(0);
    ig_pics_csr.picc.cfg_cache_global.write();

    // Enable P4 Egress (inst_id = 2)
    cap_pics_csr_t & eg_pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, 0, 2);
    eg_pics_csr.picc.cfg_cache_global.bypass(global_bypass);
    eg_pics_csr.picc.cfg_cache_global.hash_mode(0);
    eg_pics_csr.picc.cfg_cache_global.write();

    return HAL_RET_OK;
}

static hal_ret_t
capri_hbm_p4plus_cache_init (capri_cfg_t *cfg)
{
    uint32_t global_bypass = 0;

    if (cfg == NULL || cfg->p4plus_cache == false) {
        HAL_TRACE_DEBUG("Disabling HBM P4Plus cache based on HAL config.");
        global_bypass = 1;
    }

    // Enable P4Plus RXDMA (inst_id = 0)
    cap_pics_csr_t & rxdma_pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, 0, 0);
    rxdma_pics_csr.picc.cfg_cache_global.bypass(global_bypass);
    rxdma_pics_csr.picc.cfg_cache_global.hash_mode(0);
    rxdma_pics_csr.picc.cfg_cache_global.write();

    // Enable P4Plus TXDMA (inst_id = 3)
    cap_pics_csr_t & txdma_pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, 0, 3);
    txdma_pics_csr.picc.cfg_cache_global.bypass(global_bypass);
    txdma_pics_csr.picc.cfg_cache_global.hash_mode(0);
    txdma_pics_csr.picc.cfg_cache_global.write();

    return HAL_RET_OK;
}

hal_ret_t
capri_hbm_cache_init (capri_cfg_t *cfg)
{
    hal_ret_t   ret;

    ret = capri_hbm_llc_cache_init(cfg);
    assert(ret == HAL_RET_OK);

    ret = capri_hbm_p4_cache_init(cfg);
    assert(ret == HAL_RET_OK);

    ret = capri_hbm_p4plus_cache_init(cfg);
    assert(ret == HAL_RET_OK);

    return HAL_RET_OK;
}

hal_ret_t
capri_hbm_cache_program_region (capri_hbm_region_t *reg,
                                uint32_t inst_id,
                                uint32_t filter_idx,
                                bool slave,
                                bool master)
{
    cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, 0, inst_id);
    if (slave) {
        pics_csr.picc.filter_addr_lo_s.data[filter_idx].read();
        pics_csr.picc.filter_addr_lo_s.data[filter_idx].value(HBM_OFFSET(reg->start_offset) >> 6); //28 MSB bits only
        pics_csr.picc.filter_addr_lo_s.data[filter_idx].write();

        pics_csr.picc.filter_addr_hi_s.data[filter_idx].read();
        pics_csr.picc.filter_addr_hi_s.data[filter_idx].value((HBM_OFFSET(reg->start_offset) +
                                                 (reg->size_kb * 1024)) >> 6);
        pics_csr.picc.filter_addr_hi_s.data[filter_idx].write();

        pics_csr.picc.filter_addr_ctl_s.value[filter_idx].read();
        // set Valid + CacheEnable + Invalidate&Fill (has ASIC bug so dont enable this Invalidate&Fill) + Invalidate+Send
        pics_csr.picc.filter_addr_ctl_s.value[filter_idx].value(0xd);
        pics_csr.picc.filter_addr_ctl_s.value[filter_idx].write();
    }

    if (master) {
        pics_csr.picc.filter_addr_lo_m.data[filter_idx].read();
        pics_csr.picc.filter_addr_lo_m.data[filter_idx].value(HBM_OFFSET(reg->start_offset) >> 6); //28 MSB bits only
        pics_csr.picc.filter_addr_lo_m.data[filter_idx].write();

        pics_csr.picc.filter_addr_hi_m.data[filter_idx].read();
        pics_csr.picc.filter_addr_hi_m.data[filter_idx].value((HBM_OFFSET(reg->start_offset) +
                                                 (reg->size_kb * 1024)) >> 6);
        pics_csr.picc.filter_addr_hi_m.data[filter_idx].write();

        pics_csr.picc.filter_addr_ctl_m.value[filter_idx].read();
        // set Valid + CacheEnable + Invalidate&Fill (has ASIC bug so dont enable this Invalidate&Fill) + Invalidate+Send
        pics_csr.picc.filter_addr_ctl_m.value[filter_idx].value(0xd);
        pics_csr.picc.filter_addr_ctl_m.value[filter_idx].write();
    }

    return HAL_RET_OK;
}

hal_ret_t
capri_hbm_cache_program_db (capri_hbm_region_t *reg,
                            uint32_t filter_idx)
{
    cap_wa_csr_t & wa_csr = CAP_BLK_REG_MODEL_ACCESS(cap_wa_csr_t, 0, 0);

    wa_csr.filter_addr_lo.data[filter_idx].read();
    wa_csr.filter_addr_lo.data[filter_idx].value(HBM_OFFSET(reg->start_offset) >> 6); //28 MSB bits only
    wa_csr.filter_addr_lo.data[filter_idx].write();

    wa_csr.filter_addr_hi.data[filter_idx].read();
    wa_csr.filter_addr_hi.data[filter_idx].value((HBM_OFFSET(reg->start_offset) +
                                        (reg->size_kb * 1024)) >> 6);
    wa_csr.filter_addr_hi.data[filter_idx].write();

    wa_csr.filter_addr_ctl.value[filter_idx].read();
    // set Valid + CacheEnable + Invalidate&Fill (has ASIC bug so dont enable this Invalidate&Fill) + Invalidate+Send
    wa_csr.filter_addr_ctl.value[filter_idx].value(0xd);
    wa_csr.filter_addr_ctl.value[filter_idx].write();

    return HAL_RET_OK;
}

hal_ret_t
capri_hbm_cache_program_pcie (capri_hbm_region_t *reg,
                            uint32_t filter_idx)
{
    cap_pxb_csr_t & pxb_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pxb_csr_t, 0, 0);

    pxb_csr.filter_addr_lo.data[filter_idx].read();
    pxb_csr.filter_addr_lo.data[filter_idx].value(HBM_OFFSET(reg->start_offset) >> 6); //28 MSB bits only
    pxb_csr.filter_addr_lo.data[filter_idx].write();

    pxb_csr.filter_addr_hi.data[filter_idx].read();
    pxb_csr.filter_addr_hi.data[filter_idx].value((HBM_OFFSET(reg->start_offset) +
                                        (reg->size_kb * 1024)) >> 6);
    pxb_csr.filter_addr_hi.data[filter_idx].write();

    pxb_csr.filter_addr_ctl.value[filter_idx].read();
    // set Valid + CacheEnable + Invalidate&Fill (has ASIC bug so dont enable this Invalidate&Fill) + Invalidate+Send
    pxb_csr.filter_addr_ctl.value[filter_idx].value(0xd);
    pxb_csr.filter_addr_ctl.value[filter_idx].write();

    return HAL_RET_OK;
}

hal_ret_t
capri_hbm_cache_regions_init (void)
{
    capri_hbm_region_t      *reg;
    uint32_t                p4ig_filter_idx = 0;
    uint32_t                p4eg_filter_idx = 0;
    uint32_t                p4plus_txdma_filter_idx = 0;
    uint32_t                p4plus_rxdma_filter_idx = 0;
    uint32_t                p4plus_pcie_filter_idx = 0;
    uint32_t                p4plus_db_filter_idx = 0;

    for (int i = 0; i < g_capri.num_hbm_regions(); i++) {
        reg = g_capri.hbm_region(i);
        if (reg->cache_pipe == sdk::platform::CAPRI_HBM_CACHE_PIPE_NONE) {
            continue;
        }

        if (reg->cache_pipe & sdk::platform::CAPRI_HBM_CACHE_PIPE_P4IG) {
            HAL_TRACE_DEBUG("Programming {} to P4IG cache(region 1), "
                            "start={} size={} index={}", reg->mem_reg_name,
                            HBM_OFFSET(reg->start_offset), reg->size_kb, p4ig_filter_idx);
            capri_hbm_cache_program_region(reg, 1, p4ig_filter_idx, 1, 0);
            p4ig_filter_idx++;
        }

        if (reg->cache_pipe & sdk::platform::CAPRI_HBM_CACHE_PIPE_P4EG) {
            HAL_TRACE_DEBUG("Programming {} to P4EG cache(region 2), "
                            "start={} size={} index={}", reg->mem_reg_name,
                            HBM_OFFSET(reg->start_offset), reg->size_kb, p4eg_filter_idx);
            capri_hbm_cache_program_region(reg, 2, p4eg_filter_idx, 1, 0);
            p4eg_filter_idx++;
        }

        if (reg->cache_pipe & sdk::platform::CAPRI_HBM_CACHE_PIPE_P4PLUS_TXDMA) {
            HAL_TRACE_DEBUG("Programming {} to P4PLUS TXDMA cache(region 3), "
                            "start={} size={} index={}", reg->mem_reg_name,
                            HBM_OFFSET(reg->start_offset), reg->size_kb, p4plus_txdma_filter_idx);
            capri_hbm_cache_program_region(reg, 3, p4plus_txdma_filter_idx, 1, 1);
            p4plus_txdma_filter_idx++;
        }

        if (reg->cache_pipe & sdk::platform::CAPRI_HBM_CACHE_PIPE_P4PLUS_RXDMA) {
            HAL_TRACE_DEBUG("Programming {} to P4PLUS RXDMA cache(region 0), "
                            "start={} size={} index={}", reg->mem_reg_name,
                            HBM_OFFSET(reg->start_offset), reg->size_kb, p4plus_rxdma_filter_idx);
            capri_hbm_cache_program_region(reg, 0, p4plus_rxdma_filter_idx, 1, 1);
            p4plus_rxdma_filter_idx++;
        }

        if (reg->cache_pipe & sdk::platform::CAPRI_HBM_CACHE_PIPE_P4PLUS_PCIE_DB) {
            HAL_TRACE_DEBUG("Programming {} to PCIE, "
                            "start={} size={} index={}", reg->mem_reg_name,
                            HBM_OFFSET(reg->start_offset), reg->size_kb, p4plus_pcie_filter_idx);
            capri_hbm_cache_program_pcie(reg, p4plus_pcie_filter_idx);
            p4plus_pcie_filter_idx++;

            HAL_TRACE_DEBUG("Programming {} to Doorbell, "
                            "start={} size={} index={}", reg->mem_reg_name,
                            HBM_OFFSET(reg->start_offset), reg->size_kb, p4plus_db_filter_idx);
            capri_hbm_cache_program_db(reg, p4plus_db_filter_idx);
            p4plus_db_filter_idx++;
        }
    }

    return HAL_RET_OK;
}

uint64_t
capri_hbm_timestamp_get (void)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;

    pbc_csr.hbm.sta_hbm_timestamp.read();

    return pbc_csr.hbm.sta_hbm_timestamp.all().convert_to<uint64_t>();
}

hal_ret_t
capri_tpc_bw_mon_rd_get (uint64_t *maxv, uint64_t *avrg)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pics_csr_t &pics_csr = cap0.tpc.pics;

    pics_csr.sta_axi_bw_mon_rd_bandwidth.read();

    *maxv = pics_csr.sta_axi_bw_mon_rd_bandwidth.maxv().convert_to<uint64_t>();
    *avrg = pics_csr.sta_axi_bw_mon_rd_bandwidth.avrg().convert_to<uint64_t>();

    return HAL_RET_OK;
}

hal_ret_t
capri_tpc_bw_mon_wr_get (uint64_t *maxv, uint64_t *avrg)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pics_csr_t &pics_csr = cap0.tpc.pics;

    pics_csr.sta_axi_bw_mon_wr_bandwidth.read();

    *maxv = pics_csr.sta_axi_bw_mon_wr_bandwidth.maxv().convert_to<uint64_t>();
    *avrg = pics_csr.sta_axi_bw_mon_wr_bandwidth.avrg().convert_to<uint64_t>();

    return HAL_RET_OK;
}

hal_ret_t
capri_rpc_bw_mon_rd_get (uint64_t *maxv, uint64_t *avrg)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pics_csr_t &pics_csr = cap0.rpc.pics;

    pics_csr.sta_axi_bw_mon_rd_bandwidth.read();

    *maxv = pics_csr.sta_axi_bw_mon_rd_bandwidth.maxv().convert_to<uint64_t>();
    *avrg = pics_csr.sta_axi_bw_mon_rd_bandwidth.avrg().convert_to<uint64_t>();

    return HAL_RET_OK;
}

hal_ret_t
capri_rpc_bw_mon_wr_get (uint64_t *maxv, uint64_t *avrg)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pics_csr_t &pics_csr = cap0.rpc.pics;

    pics_csr.sta_axi_bw_mon_wr_bandwidth.read();

    *maxv = pics_csr.sta_axi_bw_mon_wr_bandwidth.maxv().convert_to<uint64_t>();
    *avrg = pics_csr.sta_axi_bw_mon_wr_bandwidth.avrg().convert_to<uint64_t>();

    return HAL_RET_OK;
}

hal_ret_t
capri_ms_bw_mon_rd_get (uint64_t *maxv, uint64_t *avrg)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_ms_csr_t &ms_csr = cap0.ms.ms;

    ms_csr.sta_axi_bw_mon_rd_bandwidth.read();

    *maxv = ms_csr.sta_axi_bw_mon_rd_bandwidth.maxv().convert_to<uint64_t>();
    *avrg = ms_csr.sta_axi_bw_mon_rd_bandwidth.avrg().convert_to<uint64_t>();

    return HAL_RET_OK;
}

hal_ret_t
capri_ms_bw_mon_wr_get (uint64_t *maxv, uint64_t *avrg)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_ms_csr_t &ms_csr = cap0.ms.ms;

    ms_csr.sta_axi_bw_mon_wr_bandwidth.read();

    *maxv = ms_csr.sta_axi_bw_mon_wr_bandwidth.maxv().convert_to<uint64_t>();
    *avrg = ms_csr.sta_axi_bw_mon_wr_bandwidth.avrg().convert_to<uint64_t>();

    return HAL_RET_OK;
}

hal_ret_t
capri_pxb_bw_mon_rd_get (uint64_t *maxv, uint64_t *avrg)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pxb_csr_t &pxb_csr = cap0.pxb.pxb;

    pxb_csr.sta_axi_bw_mon_rd_bandwidth.read();

    *maxv = pxb_csr.sta_axi_bw_mon_rd_bandwidth.maxv().convert_to<uint64_t>();
    *avrg = pxb_csr.sta_axi_bw_mon_rd_bandwidth.avrg().convert_to<uint64_t>();

    return HAL_RET_OK;
}

hal_ret_t
capri_pxb_bw_mon_wr_get (uint64_t *maxv, uint64_t *avrg)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pxb_csr_t &pxb_csr = cap0.pxb.pxb;

    pxb_csr.sta_axi_bw_mon_wr_bandwidth.read();

    *maxv = pxb_csr.sta_axi_bw_mon_wr_bandwidth.maxv().convert_to<uint64_t>();
    *avrg = pxb_csr.sta_axi_bw_mon_wr_bandwidth.avrg().convert_to<uint64_t>();

    return HAL_RET_OK;
}

hal_ret_t
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
    asic_hbm_bw_t *hbm_bw = NULL;

    prev_ts = capri_hbm_timestamp_get();

    for (uint32_t i = 0; i < samples; ++i) {

        usleep(u_sleep);

        cur_ts = capri_hbm_timestamp_get();

        clk_diff = cur_ts - prev_ts;

        hbm_bw = &hbm_bw_arr[index++];
        hbm_bw->type = hal::pd::ASIC_BLOCK_TXD;
        hbm_bw->clk_diff = clk_diff;
        capri_tpc_bw_mon_rd_get(&max_rd, &avg_rd);
        capri_tpc_bw_mon_wr_get(&max_wr, &avg_wr);
        hbm_bw->max.read  = max_rd;
        hbm_bw->max.write = max_wr;
        hbm_bw->avg.read  = avg_rd;
        hbm_bw->avg.write = avg_wr;
        HAL_TRACE_DEBUG("CLK_DIFF: {}, TXD BW. MAX_RD: {}, MAX_WR: {}"
                        ", AVG_RD: {}, AVG_WR: {}",
                        clk_diff, max_rd, max_wr, avg_rd, avg_wr);

        hbm_bw = &hbm_bw_arr[index++];
        hbm_bw->type = hal::pd::ASIC_BLOCK_RXD;
        hbm_bw->clk_diff = clk_diff;
        capri_rpc_bw_mon_rd_get(&max_rd, &avg_rd);
        capri_rpc_bw_mon_wr_get(&max_wr, &avg_wr);
        hbm_bw->max.read  = max_rd;
        hbm_bw->max.write = max_wr;
        hbm_bw->avg.read  = avg_rd;
        hbm_bw->avg.write = avg_wr;
        HAL_TRACE_DEBUG("CLK_DIFF: {}, RXD BW. MAX_RD: {}, MAX_WR: {}"
                        ", AVG_RD: {}, AVG_WR: {}",
                        clk_diff, max_rd, max_wr, avg_rd, avg_wr);

        hbm_bw = &hbm_bw_arr[index++];
        hbm_bw->type = hal::pd::ASIC_BLOCK_MS;
        hbm_bw->clk_diff = clk_diff;
        capri_ms_bw_mon_rd_get(&max_rd, &avg_rd);
        capri_ms_bw_mon_wr_get(&max_wr, &avg_wr);
        hbm_bw->max.read  = max_rd;
        hbm_bw->max.write = max_wr;
        hbm_bw->avg.read  = avg_rd;
        hbm_bw->avg.write = avg_wr;
        HAL_TRACE_DEBUG("CLK_DIFF: {}, MS BW. MAX_RD: {}, MAX_WR: {}"
                        ", AVG_RD: {}, AVG_WR: {}",
                        clk_diff, max_rd, max_wr, avg_rd, avg_wr);

        hbm_bw = &hbm_bw_arr[index++];
        hbm_bw->type = hal::pd::ASIC_BLOCK_PCIE;
        hbm_bw->clk_diff = clk_diff;
        capri_pxb_bw_mon_rd_get(&max_rd, &avg_rd);
        capri_pxb_bw_mon_wr_get(&max_wr, &avg_wr);
        hbm_bw->max.read  = max_rd;
        hbm_bw->max.write = max_wr;
        hbm_bw->avg.read  = avg_rd;
        hbm_bw->avg.write = avg_wr;
        HAL_TRACE_DEBUG("CLK_DIFF: {}, PCIE BW. MAX_RD: {}, MAX_WR: {}"
                        ", AVG_RD: {}, AVG_WR: {}",
                        clk_diff, max_rd, max_wr, avg_rd, avg_wr);

        cycle_per_nsec = ((cur_ts - prev_ts) * 1000) / 830;

        hbm_bw = &hbm_bw_arr[index++];
        hbm_bw->type = hal::pd::ASIC_BLOCK_PB;
        hbm_bw->clk_diff = clk_diff;
        rd_cnt = cap_nx_read_pb_axi_cnt(1);
        wr_cnt = cap_nx_read_pb_axi_cnt(0);

        // avoid arithmetic exceptions
        if (cycle_per_nsec != 0) {
            avg_rd = ((rd_cnt - prev_rd_cnt) * num_bits * 1000) / cycle_per_nsec;
            avg_wr = ((wr_cnt - prev_wr_cnt) * num_bits * 1000) / cycle_per_nsec;
        }

        hbm_bw->avg.read  = avg_rd;
        hbm_bw->avg.write = avg_wr;
        HAL_TRACE_DEBUG("CLK_DIFF: {}, PB RD AVG: {}. PB WR AVG: {}",
                        clk_diff, avg_rd, avg_wr);

        prev_rd_cnt = rd_cnt;
        prev_wr_cnt = wr_cnt;
        prev_ts     = cur_ts;
    }

    return HAL_RET_OK;
}

hal_ret_t
capri_nx_get_llc_counters (uint32_t *rd_data)
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

    return HAL_RET_OK;
}

hal_ret_t
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

    cap_nx_block_write(0, RBM_AGENT_(CCC0_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0, RBM_AGENT_(CCC1_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0, RBM_AGENT_(CCC10_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0, RBM_AGENT_(CCC11_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0, RBM_AGENT_(CCC12_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0, RBM_AGENT_(CCC13_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0, RBM_AGENT_(CCC14_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0, RBM_AGENT_(CCC15_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0, RBM_AGENT_(CCC2_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0, RBM_AGENT_(CCC3_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0, RBM_AGENT_(CCC4_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0, RBM_AGENT_(CCC5_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0, RBM_AGENT_(CCC6_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0, RBM_AGENT_(CCC7_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0, RBM_AGENT_(CCC8_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);
    cap_nx_block_write(0, RBM_AGENT_(CCC9_LLC_EVENT_COUNTER_MASK), 1, data, true, 1);

    return HAL_RET_OK;
}

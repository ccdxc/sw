#include "nic/include/base.h"
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

#include "nic/asic/capri/model/utils/cap_blk_reg_model.h"
#include "nic/asic/capri/model/cap_pic/cap_pics_csr.h"

namespace pt = boost::property_tree;

static capri_hbm_region_t *hbm_regions_;
static int num_hbm_regions_;

#define HBM_OFFSET(x)       CAPRI_HBM_OFFSET(x)

hal_ret_t
capri_hbm_parse (capri_cfg_t *cfg)
{
    pt::ptree               json_pt;
    std::string             full_path;
    capri_hbm_region_t      *reg;

    // makeup the full file path
    full_path =  cfg->cfg_path + "/" + cfg->pgm_name +
                     "/" + std::string("hbm_mem.json");

    HAL_TRACE_DEBUG("HBM Memory Json: {}", full_path.c_str());

    // make sure cfg file exists
    if (access(full_path.c_str(), R_OK) < 0) {
        HAL_TRACE_ERR("{} not_present/no_read_permissions", full_path.c_str());
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }

    std::ifstream hbm_json(full_path.c_str());

    // Reading from file
    read_json(hbm_json, json_pt);

    boost::optional<pt::ptree&>reg_pt = json_pt.get_child_optional(JKEY_REGIONS);
    if (!reg_pt) {
        return HAL_RET_ERR;
    }

    num_hbm_regions_ = json_pt.get_child(JKEY_REGIONS).size();
    hbm_regions_ = (capri_hbm_region_t *)
        HAL_CALLOC(hal::HAL_MEM_ALLOC_PD, num_hbm_regions_ * sizeof(capri_hbm_region_t));
    if (!hbm_regions_) {
        return HAL_RET_OOM;
    }

    int idx = 0;
    uint64_t offset = 0;
    BOOST_FOREACH(pt::ptree::value_type &p4_tbl, json_pt.get_child(JKEY_REGIONS)) {

        reg = hbm_regions_ + idx;

        std::string reg_name = p4_tbl.second.get<std::string>(JKEY_REGION_NAME);

        std::string cache_pipe_name = p4_tbl.second.get<std::string>(JKEY_CACHE_PIPE, "null");
        if (cache_pipe_name == "p4ig") {
            reg->cache_pipe = CAPRI_HBM_CACHE_PIPE_P4IG;
        } else if (cache_pipe_name == "p4eg") {
            reg->cache_pipe = CAPRI_HBM_CACHE_PIPE_P4EG;
        } else if (cache_pipe_name == "p4plus-txdma") {
            reg->cache_pipe = CAPRI_HBM_CACHE_PIPE_P4PLUS_TXDMA;
        } else if (cache_pipe_name == "p4plus-rxdma") {
            reg->cache_pipe = CAPRI_HBM_CACHE_PIPE_P4PLUS_RXDMA;
        } else if (cache_pipe_name == "p4plus-both") {
            reg->cache_pipe = CAPRI_HBM_CACHE_PIPE_P4PLUS_BOTH;
        } else {
            reg->cache_pipe = CAPRI_HBM_CACHE_PIPE_NONE;
        }

        strcpy(reg->mem_reg_name, reg_name.c_str());
        reg->size_kb = p4_tbl.second.get<int>(JKEY_SIZE_KB);
        // reg->start_offset = p4_tbl.second.get<int>(JKEY_START_OFF);
        reg->start_offset = offset;

        HAL_TRACE_DEBUG("Region: {0:}, Size_KB: {1:}, Start_Off: 0x{2:x}", 
                reg->mem_reg_name, reg->size_kb, HBM_OFFSET(reg->start_offset));

        offset += reg->size_kb * 1024;
        idx++;
    }

    return HAL_RET_OK;
}

hbm_addr_t
get_hbm_base (void)
{
    return HBM_OFFSET(0);
}

hbm_addr_t
get_hbm_offset (const char *reg_name)
{
    capri_hbm_region_t *reg;

    for (int i = 0; i < num_hbm_regions_; i++) {
        reg = &hbm_regions_[i];
        if (!strcmp(reg->mem_reg_name, reg_name)) {
            return (reg->start_offset);
        }
    }

    return CAPRI_INVALID_OFFSET;
}

hbm_addr_t
get_start_offset (const char *reg_name)
{
    capri_hbm_region_t      *reg;

    for (int i = 0; i < num_hbm_regions_; i++) {
        reg = &hbm_regions_[i];
        if (!strcmp(reg->mem_reg_name, reg_name)) {
            return (HBM_OFFSET(reg->start_offset));
        }
    }

    return CAPRI_INVALID_OFFSET;
}

uint32_t
get_size_kb (const char *reg_name)
{
    capri_hbm_region_t      *reg;

    for (int i = 0; i < num_hbm_regions_; i++) {
        reg = &hbm_regions_[i];
        if (!strcmp(reg->mem_reg_name, reg_name)) {
            return reg->size_kb;
        }
    }

    return 0;
}

capri_hbm_region_t *
get_hbm_region (char *reg_name)
{
    capri_hbm_region_t      *reg;

    for (int i = 0; i < num_hbm_regions_; i++) {
        reg = &hbm_regions_[i];
        if (!strcmp(reg->mem_reg_name, reg_name)) {
            return reg;
        }
    }
    return NULL;
}

int32_t
capri_hbm_read_mem (uint64_t addr, uint8_t *buf, uint32_t size)
{
    hal_ret_t rc = hal::pd::asic_mem_read(addr, buf, size);
    return (rc == HAL_RET_OK) ? 0 : -EIO;
}

int32_t
capri_hbm_write_mem (uint64_t addr, uint8_t *buf, uint32_t size)
{
    hal_ret_t rc = hal::pd::asic_mem_write(addr, buf, size, true);
    return (rc == HAL_RET_OK) ? 0 : -EIO;
}

hal_ret_t
capri_hbm_cache_init (void)
{
    // Enable P4Plus RXDMA (inst_id = 0)
    cap_pics_csr_t & rxdma_pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, 0, 0);
    rxdma_pics_csr.picc.cfg_cache_global.bypass(0);
    rxdma_pics_csr.picc.cfg_cache_global.hash_mode(0);
    rxdma_pics_csr.picc.cfg_cache_global.write();

    // Enable P4 Ingress (inst_id = 1)
    cap_pics_csr_t & ig_pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, 0, 1);
    ig_pics_csr.picc.cfg_cache_global.bypass(0);
    ig_pics_csr.picc.cfg_cache_global.hash_mode(0);
    ig_pics_csr.picc.cfg_cache_global.write();

    // Enable P4 Egress (inst_id = 2)
    cap_pics_csr_t & eg_pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, 0, 2);
    eg_pics_csr.picc.cfg_cache_global.bypass(0);
    eg_pics_csr.picc.cfg_cache_global.hash_mode(0);
    eg_pics_csr.picc.cfg_cache_global.write();

    // Enable P4Plus TXDMA (inst_id = 3)
    cap_pics_csr_t & txdma_pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, 0, 3);
    txdma_pics_csr.picc.cfg_cache_global.bypass(0);
    txdma_pics_csr.picc.cfg_cache_global.hash_mode(0);
    txdma_pics_csr.picc.cfg_cache_global.write();

    return HAL_RET_OK;
}

hal_ret_t
capri_hbm_cache_program_region (capri_hbm_region_t *reg, 
                                uint32_t inst_id, 
                                uint32_t filter_idx)
{
    cap_pics_csr_t & pics_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pics_csr_t, 0, inst_id);
    pics_csr.picc.filter_addr_lo_s.data[filter_idx].read();
    pics_csr.picc.filter_addr_lo_s.data[filter_idx].value(HBM_OFFSET(reg->start_offset) >> 6); //28 MSB bits only
    pics_csr.picc.filter_addr_lo_s.data[filter_idx].write();

    pics_csr.picc.filter_addr_hi_s.data[filter_idx].read();
    pics_csr.picc.filter_addr_hi_s.data[filter_idx].value((HBM_OFFSET(reg->start_offset) + 
                                                 (reg->size_kb * 1024)) >> 6);
    pics_csr.picc.filter_addr_hi_s.data[filter_idx].write();

    pics_csr.picc.filter_addr_ctl_s.value[filter_idx].read();
    // set Valid + CacheEnable + Invalidate&Fill bits
    pics_csr.picc.filter_addr_ctl_s.value[filter_idx].value(0xe);
    pics_csr.picc.filter_addr_ctl_s.value[filter_idx].write();

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

    for (int i = 0; i < num_hbm_regions_; i++) {
        reg = &hbm_regions_[i];
        if (reg->cache_pipe == CAPRI_HBM_CACHE_PIPE_NONE) {
            continue;
        }

        if (reg->cache_pipe & CAPRI_HBM_CACHE_PIPE_P4IG) {
            HAL_TRACE_DEBUG("HBM Cache: Programming {} to P4IG cache(region 1), "
                            "start={} size={} index={}", reg->mem_reg_name,
                            HBM_OFFSET(reg->start_offset), reg->size_kb, p4ig_filter_idx);
            capri_hbm_cache_program_region(reg, 1, p4ig_filter_idx);
            p4ig_filter_idx++;
        }

        if (reg->cache_pipe & CAPRI_HBM_CACHE_PIPE_P4EG) {
            HAL_TRACE_DEBUG("HBM Cache: Programming {} to P4EG cache(region 2), "
                            "start={} size={} index={}", reg->mem_reg_name,
                            HBM_OFFSET(reg->start_offset), reg->size_kb, p4eg_filter_idx);
            capri_hbm_cache_program_region(reg, 2, p4eg_filter_idx);
            p4eg_filter_idx++;
        }

        if (reg->cache_pipe & CAPRI_HBM_CACHE_PIPE_P4PLUS_TXDMA) {
            HAL_TRACE_DEBUG("HBM Cache: Programming {} to P4PLUS TXDMA cache(region 3), "
                            "start={} size={} index={}", reg->mem_reg_name,
                            HBM_OFFSET(reg->start_offset), reg->size_kb, p4plus_txdma_filter_idx);
            capri_hbm_cache_program_region(reg, 3, p4plus_txdma_filter_idx);
            p4plus_txdma_filter_idx++;
        }

        if (reg->cache_pipe & CAPRI_HBM_CACHE_PIPE_P4PLUS_RXDMA) {
            HAL_TRACE_DEBUG("HBM Cache: Programming {} to P4PLUS RXDMA cache(region 0), "
                            "start={} size={} index={}", reg->mem_reg_name,
                            HBM_OFFSET(reg->start_offset), reg->size_kb, p4plus_rxdma_filter_idx);
            capri_hbm_cache_program_region(reg, 0, p4plus_rxdma_filter_idx);
            p4plus_rxdma_filter_idx++;
        }
    }

    return HAL_RET_OK;
}

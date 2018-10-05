#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "include/sdk/platform/capri/capri.hpp"
#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "include/sdk/mem.hpp"

namespace pt = boost::property_tree;

namespace sdk {
namespace platform {


sdk_ret_t
capri::capri_hbm_parse (const char *hbm_mem_json_file)
{
    pt::ptree               json_pt;
    std::string             full_path;
    capri_hbm_region_t      *reg;

    SDK_TRACE_DEBUG("HBM memory config file : %s", hbm_mem_json_file);
    // make sure cfg file exists
    if (access(hbm_mem_json_file, R_OK) < 0) {
        SDK_TRACE_ERR("%s not_present/no_read_permissions", hbm_mem_json_file);
        SDK_ASSERT_RETURN(0, SDK_RET_ERR);
    }

    std::ifstream hbm_json(hbm_mem_json_file);

    // Reading from file
    read_json(hbm_json, json_pt);

    boost::optional<pt::ptree&>reg_pt = json_pt.get_child_optional(JKEY_REGIONS);
    if (!reg_pt) {
        return SDK_RET_ERR;
    }

    num_hbm_regions_ = json_pt.get_child(JKEY_REGIONS).size();
    hbm_regions_ = (capri_hbm_region_t *)
        SDK_CALLOC(SDK_MEM_ALLOC_ID_PLATFORM_CAPRI, num_hbm_regions_ * sizeof(capri_hbm_region_t));
    if (!hbm_regions_) {
        return SDK_RET_OOM;
    }

    int idx = 0;
    uint64_t offset = 0;
    BOOST_FOREACH(pt::ptree::value_type &p4_tbl, json_pt.get_child(JKEY_REGIONS)) {
        reg = hbm_regions_ + idx;
        std::string reg_name = p4_tbl.second.get<std::string>(JKEY_REGION_NAME);
        std::string cache_pipe_name = p4_tbl.second.get<std::string>(JKEY_CACHE_PIPE, "null");
        reg->reset = p4_tbl.second.get<bool>(JKEY_RESET_REGION, false);
        if (cache_pipe_name == "p4ig") {
            reg->cache_pipe = CAPRI_HBM_CACHE_PIPE_P4IG;
        } else if (cache_pipe_name == "p4eg") {
            reg->cache_pipe = CAPRI_HBM_CACHE_PIPE_P4EG;
        } else if (cache_pipe_name == "p4plus-txdma") {
            reg->cache_pipe = CAPRI_HBM_CACHE_PIPE_P4PLUS_TXDMA;
        } else if (cache_pipe_name == "p4plus-rxdma") {
            reg->cache_pipe = CAPRI_HBM_CACHE_PIPE_P4PLUS_RXDMA;
        } else if (cache_pipe_name == "p4plus-all") {
            reg->cache_pipe = CAPRI_HBM_CACHE_PIPE_P4PLUS_ALL;
        } else {
            reg->cache_pipe = CAPRI_HBM_CACHE_PIPE_NONE;
        }
        strcpy(reg->mem_reg_name, reg_name.c_str());
        reg->size_kb = p4_tbl.second.get<int>(JKEY_SIZE_KB);
        reg->start_offset = offset;

        SDK_TRACE_DEBUG("region : %s, size : %dkb, reset : %d, "
                        "start : 0x%" PRIx64 ", end : 0x%" PRIx64 "",
                        reg->mem_reg_name, reg->size_kb, reg->reset,
                        HBM_OFFSET(reg->start_offset),
                        HBM_OFFSET(reg->start_offset + reg->size_kb * 1024));

        offset += reg->size_kb * 1024;
        idx++;
    }

    return SDK_RET_OK;
}

hbm_addr_t
capri::get_hbm_base (void)
{
    return HBM_OFFSET(0);
}

hbm_addr_t
capri::get_hbm_offset (const char *reg_name)
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
capri::get_start_offset (const char *reg_name)
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
capri::get_size_kb (const char *reg_name)
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
capri::get_hbm_region (char *reg_name)
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

capri_hbm_region_t *
capri::get_hbm_region_by_address (uint64_t addr)
{
    capri_hbm_region_t      *reg;

    for (int i = 0; i < num_hbm_regions_; i++) {
        reg = &hbm_regions_[i];

        /*
        SDK_TRACE_DEBUG("Region: {}, Size_KB: {}, Start:{:#x} End:{:#x}, addr: {:#x}",
                        reg->mem_reg_name, reg->size_kb,
                        HBM_OFFSET(reg->start_offset),
                        HBM_OFFSET(reg->start_offset + reg->size_kb * 1024), addr);
        */

        if ((addr >= HBM_OFFSET(reg->start_offset)) &&
            (addr < HBM_OFFSET(reg->start_offset + (reg->size_kb * 1024)))) {
            return reg;
        }
    }
    return NULL;
}

}   // namespace platform
}   // namespace sdk


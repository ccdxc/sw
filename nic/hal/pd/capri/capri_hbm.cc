#include <base.h>
#include <unistd.h>
#include <iostream>
#include "capri_hbm.hpp"
#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include <lib_model_client.h>
#include <arpa/inet.h>

namespace pt = boost::property_tree;

static capri_hbm_region_t *hbm_regions_;

capri_semaphore_t           *g_hbm_semaphore;
capri_descr_t               *g_hbm_descr;
capri_big_page_t            *g_hbm_big_page;
capri_small_page_t          *g_hbm_small_page;
uint64_t                    *g_hbm_nmdr;
uint64_t                    *g_hbm_nmpr_big;
uint64_t                    *g_hbm_nmpr_small;

#define HBM_OFFSET(x)       (0x80000000 + (x))

hal_ret_t
capri_hbm_parse() 
{
    char             		*cfg_path;
    char                    cfgfile[] = "hbm_mem.json";
    pt::ptree               json_pt;
    std::string             full_path;
    capri_hbm_region_t      *reg;

    // makeup the full file path
    cfg_path = std::getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        full_path =  std::string(cfg_path) + "/" + std::string(cfgfile);
    } else {
        HAL_TRACE_ERR("Please set HAL_CONFIG_PATH env. variable");
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }

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

    int num_regs = CARPI_HBM_MEM_NUM_MEM_REGS;
    hbm_regions_ = (capri_hbm_region_t *)HAL_CALLOC(HAL_MEM_ALLOC_PD, num_regs * sizeof(capri_hbm_region_t));
    if (!hbm_regions_) {
        return HAL_RET_OOM;
    }

    int idx = 0;
    uint64_t offset = 0;
    BOOST_FOREACH(pt::ptree::value_type &p4_tbl, json_pt.get_child(JKEY_REGIONS)) {

        reg = hbm_regions_ + idx;

        std::string reg_name = p4_tbl.second.get<std::string>(JKEY_REGION_NAME);


        strcpy(reg->mem_reg_name, reg_name.c_str());
        reg->size_kb = p4_tbl.second.get<int>(JKEY_SIZE_KB);
        // reg->start_offset = p4_tbl.second.get<int>(JKEY_START_OFF);
        reg->start_offset = offset;

        HAL_TRACE_DEBUG("Region: {}, Size_KB: {}, Start_Off: {}", 
                reg->mem_reg_name, reg->size_kb, reg->start_offset);

        offset += reg->size_kb * 1024;
        idx++;
    }

    return HAL_RET_OK;
}

uint32_t
get_start_offset(const char *reg_name)
{
    capri_hbm_region_t      *reg;

    for (int i = 0; i < CARPI_HBM_MEM_NUM_MEM_REGS; i++) {
        reg = &hbm_regions_[i];
        if (!strcmp(reg->mem_reg_name, reg_name)) {
            return (HBM_OFFSET(reg->start_offset));
        }
    }

    return 0;
}

uint32_t
get_size_kb(const char *reg_name)
{
    capri_hbm_region_t      *reg;

    for (int i = 0; i < CARPI_HBM_MEM_NUM_MEM_REGS; i++) {
        reg = &hbm_regions_[i];
        if (!strcmp(reg->mem_reg_name, reg_name)) {
            return reg->size_kb;
        }
    }

    return 0;
}

capri_hbm_region_t *
get_hbm_region(char *reg_name)
{
    capri_hbm_region_t      *reg;

    for (int i = 0; i < CARPI_HBM_MEM_NUM_MEM_REGS; i++) {
        reg = &hbm_regions_[i];
        if (!strcmp(reg->mem_reg_name, reg_name)) {
            return reg;
        }
    }
    return NULL;
}

static hal_ret_t
hbm_semaphore_init(void)
{
        capri_hbm_region_t  *reg;

        reg = get_hbm_region((char *)JP4_SEMAPHORE);
        if (!reg) {
                HAL_TRACE_ERR("Could not find {} region", JP4_SEMAPHORE);
                return HAL_RET_ERR;
        }
        assert(reg->size_kb * 1024 >=
                        2 * CAPRI_NUM_SEMAPHORES * sizeof(uint64_t));
        g_hbm_semaphore = (capri_semaphore_t *)(uint64_t)
                                        HBM_OFFSET(reg->start_offset);
        HAL_TRACE_DEBUG("g_hbm_semaphore 0x{0:x}, size {1}k",
                        (uint64_t)g_hbm_semaphore, reg->size_kb);

        return HAL_RET_OK;
}

hal_ret_t
capri_hbm_mem_init(void)
{
        hal_ret_t ret;

        ret = hbm_semaphore_init();
        assert (ret == HAL_RET_OK);

        return HAL_RET_OK;
}

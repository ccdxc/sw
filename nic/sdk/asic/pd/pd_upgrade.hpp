//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __SDK_ASIC_PD_UPGRADE_HPP__
#define __SDK_ASIC_PD_UPGRADE_HPP__

#include "include/sdk/base.hpp"
#include "asic/pd/pd.hpp"

namespace sdk {
namespace asic {
namespace pd {

typedef struct p4_upg_table_property_s {
    int tableid;
    int stage;
    int stage_tableid;
    uint64_t asm_err_offset;
    uint64_t asm_base;
    uint64_t mem_offset; // only for hbm tables
    uint32_t pc_offset;
    bool     pc_dyn;
} p4_upg_table_property_t;

uint32_t asicpd_upg_table_property_get(p4pd_pipeline_t pipeline,
                                       p4_upg_table_property_t *info,
                                       uint32_t ninfos);
sdk_ret_t asicpd_upg_table_property_set(p4pd_pipeline_t pipeline,
                                        p4_upg_table_property_t *cfg,
                                        uint32_t ncfgs);
sdk_ret_t asicpd_upg_rss_table_property_get(const char *handle, uint32_t tableid,
                                            p4_upg_table_property_t *rss);
void asicpd_upg_rss_table_property_set(p4_upg_table_property_t *rss);

}    // namespace pd
}    // namespace asic
}    // namespace sdk

using sdk::asic::pd::p4_upg_table_property_t;

#endif    // __SDK_ASIC_PD_UPGRADE_HPP__

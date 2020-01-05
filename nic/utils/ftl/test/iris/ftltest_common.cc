//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "../ftltest_common.hpp"

static void
fill_entry (uint32_t index, flow_hash_info_entry_t *entry)
{
    entry->clear();
    entry->set_flow_lkp_metadata_lkp_proto(17);
    entry->set_flow_lkp_metadata_lkp_sport(index);
    entry->set_flow_lkp_metadata_lkp_dport(index);

    uint8_t flow_lkp_metadata_lkp_src[16] = {0};
    flow_lkp_metadata_lkp_src[0] = 0x22;
    flow_lkp_metadata_lkp_src[12] = index >> 24 & 0xFF;
    flow_lkp_metadata_lkp_src[13] = index >> 16 & 0xFF;
    flow_lkp_metadata_lkp_src[14] = index >> 8 & 0xFF;
    flow_lkp_metadata_lkp_src[15] = index & 0xFF;
    entry->set_flow_lkp_metadata_lkp_src(flow_lkp_metadata_lkp_src);

    uint8_t flow_lkp_metadata_lkp_dst[16] = {0};
    flow_lkp_metadata_lkp_dst[0] = 0x33;
    flow_lkp_metadata_lkp_dst[12] = index >> 24 & 0xFF;
    flow_lkp_metadata_lkp_dst[13] = index >> 16 & 0xFF;
    flow_lkp_metadata_lkp_dst[14] = index >> 8 & 0xFF;
    flow_lkp_metadata_lkp_dst[15] = index & 0xFF;
    entry->set_flow_lkp_metadata_lkp_dst(flow_lkp_metadata_lkp_dst);

    entry->set_flow_index(index+1);
    return;
}

sdk_table_api_params_t *
gen_entry (uint32_t index, bool with_hash, uint32_t hash_32b) {
    static flow_hash_info_entry_t entry[POOL_SIZE];
    static sdk_table_api_params_t params[POOL_SIZE];
    static uint32_t pidx = 0;

    fill_entry(index+1, &entry[pidx]);
    params[pidx].entry = &entry[pidx];
    params[pidx].entry_size = flow_hash_info_entry_t::entry_size();
    params[pidx].hash_valid = with_hash;
    if (with_hash) {
        params[pidx].hash_32b = hash_32b ? hash_32b : index;
    } else {
        params[pidx].hash_32b = 0;
    }

    auto ret = &params[pidx];
    pidx = (pidx + 1) % POOL_SIZE;
    ret->handle.clear();
    return ret;
}

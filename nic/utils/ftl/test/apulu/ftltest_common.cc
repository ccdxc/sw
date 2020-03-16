//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/utils/ftl/test/ftltest_common.hpp"

static void
fill_entry (uint32_t index, flow_hash_entry_t *entry)
{
    entry->clear();
    entry->set_key_metadata_proto(17);
    entry->set_key_metadata_sport(index);
    entry->set_key_metadata_dport(index);

    uint8_t key_metadata_src[16] = {0};
    key_metadata_src[0] = 0x22;
    key_metadata_src[12] = index >> 24 & 0xFF;
    key_metadata_src[13] = index >> 16 & 0xFF;
    key_metadata_src[14] = index >> 8 & 0xFF;
    key_metadata_src[15] = index & 0xFF;
    entry->set_key_metadata_src(key_metadata_src);

    uint8_t key_metadata_dst[16] = {0};
    key_metadata_dst[0] = 0x33;
    key_metadata_dst[12] = index >> 24 & 0xFF;
    key_metadata_dst[13] = index >> 16 & 0xFF;
    key_metadata_dst[14] = index >> 8 & 0xFF;
    key_metadata_dst[15] = index & 0xFF;
    entry->set_key_metadata_dst(key_metadata_dst);

    entry->set_session_index(index+1);
    return;
}

sdk_table_api_params_t *
gen_entry (uint32_t index, bool with_hash, uint32_t hash_32b) {
    static thread_local flow_hash_entry_t entry[POOL_SIZE];
    static thread_local sdk_table_api_params_t params[POOL_SIZE];
    static thread_local uint32_t pidx = 0;

    fill_entry(index+1, &entry[pidx]);
    params[pidx].entry = &entry[pidx];
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

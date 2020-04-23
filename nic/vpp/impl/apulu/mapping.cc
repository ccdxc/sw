//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//

#include <arpa/inet.h>
#include <nic/sdk/include/sdk/ip.hpp>
#include <nic/sdk/include/sdk/table.hpp>
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include <gen/p4gen/apulu/include/p4pd.h>
#include <nic/apollo/p4/include/apulu_defines.h>
#include "nic/sdk/include/sdk/table.hpp"
#include "mapping.h"

using namespace sdk;
using namespace table;

#define PDS_IMPL_FILL_IP_MAPPING_SWKEY(key, vpc_hw_id, ip)                   \
{                                                                            \
    memset((key), 0, sizeof(*(key)));                                        \
    (key)->p4e_i2e_mapping_lkp_id = vpc_hw_id;                               \
    if ((ip)->af == IP_AF_IPV6) {                                            \
        (key)->p4e_i2e_mapping_lkp_type = KEY_TYPE_IPV6;                     \
        sdk::lib::memrev((key)->p4e_i2e_mapping_lkp_addr,                    \
                         (ip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);           \
    } else {                                                                 \
        (key)->p4e_i2e_mapping_lkp_type = KEY_TYPE_IPV4;                     \
        memcpy((key)->p4e_i2e_mapping_lkp_addr,                              \
               &(ip)->addr.v4_addr, IP4_ADDR8_LEN);                          \
    }                                                                        \
}

#define PDS_IMPL_FILL_TABLE_API_PARAMS(api_params, key_, mask_,              \
                                       data, action, hdl)                    \
{                                                                            \
    memset((api_params), 0, sizeof(*(api_params)));                          \
    (api_params)->key = (key_);                                              \
    (api_params)->mask = (mask_);                                            \
    (api_params)->appdata = (data);                                          \
    (api_params)->action_id = (action);                                      \
    (api_params)->handle = (hdl);                                            \
    (api_params)->force_hwread = TRUE;                                       \
}

#define PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(key, vpc_hw_id, ip)             \
{                                                                            \
    memset((key), 0, sizeof(*(key)));                                        \
    (key)->key_metadata_local_mapping_lkp_id = vpc_hw_id;                    \
    if ((ip)->af == IP_AF_IPV6) {                                            \
        (key)->key_metadata_local_mapping_lkp_type = KEY_TYPE_IPV6;          \
        sdk::lib::memrev((key)->key_metadata_local_mapping_lkp_addr,         \
                         (ip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);           \
    } else {                                                                 \
        (key)->key_metadata_local_mapping_lkp_type = KEY_TYPE_IPV4;          \
        memcpy((key)->key_metadata_local_mapping_lkp_addr,                   \
               &(ip)->addr.v4_addr, IP4_ADDR8_LEN);                          \
    }                                                                        \
}

extern "C" {

mem_hash *mapping_tbl;
mem_hash *local_mapping_tbl;

void
pds_mapping_table_init (void)
{
    static bool init_done = FALSE;
    sdk_table_factory_params_t    tparams;

    if (init_done == TRUE) {
        return;
    }

    // instantiate P4 tables for bookkeeping
    bzero(&tparams, sizeof(tparams));
    tparams.max_recircs = 8;
    tparams.entry_trace_en = true;
    tparams.key2str = NULL;
    tparams.appdata2str = NULL;

    // MAPPING table bookkeeping
    tparams.table_id = P4TBL_ID_MAPPING;
    tparams.num_hints = P4_MAPPING_NUM_HINTS_PER_ENTRY;
    mapping_tbl = mem_hash::factory(&tparams);
    SDK_ASSERT(mapping_tbl != NULL);
    init_done = TRUE;
}

void
pds_local_mapping_table_init (void)
{
    static bool init_done = FALSE;
    sdk_table_factory_params_t tparams;

    if (init_done == TRUE) {
        return;
    }

    // instantiate P4 tables for bookkeeping
    bzero(&tparams, sizeof(tparams));
    tparams.max_recircs = 8;
    tparams.entry_trace_en = true;
    tparams.key2str = NULL;
    tparams.appdata2str = NULL;

    // MAPPING table bookkeeping
    tparams.table_id = P4TBL_ID_LOCAL_MAPPING;
    tparams.num_hints = P4_LOCAL_MAPPING_NUM_HINTS_PER_ENTRY;
    local_mapping_tbl = mem_hash::factory(&tparams);
    SDK_ASSERT(local_mapping_tbl != NULL);
    init_done = TRUE;
}

static inline void
pds_dst_addr_get (uint32_t dst_addr, ip_addr_t *dst)
{
    if (dst) {
        dst->af = 0;
        dst->addr.v4_addr = dst_addr;
    }
}

static inline int
pds_mapping_dmac_get (mac_addr_t mac_addr, uint32_t dst_addr,
                      uint16_t vpc_id, uint16_t bd_id)
{
    sdk_table_api_params_t tparams;
    mapping_swkey_t mapping_key;
    mapping_appdata_t mapping_data;
    ip_addr_t dst;
    sdk_ret_t ret;
    uint32_t vr_ip = 0;
    uint8_t *vr_mac;

    pds_impl_db_vr_ip_mac_get(bd_id, &vr_ip, &vr_mac);
    if (vr_ip == dst_addr) {
        memcpy(mac_addr, vr_mac, ETH_ADDR_LEN);
        return 0;
    }
    pds_dst_addr_get(dst_addr, &dst);
    PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key, vpc_id, &dst);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL,
                                   &mapping_data, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_tbl->get(&tparams);
    if (ret != SDK_RET_OK) {
        return -1;
    }
    sdk::lib::memrev(mac_addr, mapping_data.dmaci, ETH_ADDR_LEN);

    return 0;
}

// Based on the config, we get either the dst mac from remote mapping table
// or subnet's vr mac from bd table
int
pds_dst_mac_get (uint16_t vpc_id, uint16_t bd_id, mac_addr_t mac_addr,
                 uint32_t dst_addr)
{
    int ret;

    ret = pds_mapping_dmac_get(mac_addr, dst_addr, vpc_id, bd_id);
    return ret;
}

int 
pds_local_mapping_vnic_id_get (uint16_t vpc_id, uint32_t addr, uint16_t *vnic_id)
{
    sdk_table_api_params_t tparams;
    local_mapping_swkey_t local_mapping_key;
    local_mapping_appdata_t local_mapping_data;
    ip_addr_t dst;
    sdk_ret_t ret;

    pds_dst_addr_get(addr, &dst);
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key, vpc_id, &dst);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL,
                                   &local_mapping_data, 0,
                                   sdk::table::handle_t::null());
    ret = local_mapping_tbl->get(&tparams);
    if (ret != SDK_RET_OK) {
        return -1;
    }
    
    *vnic_id = local_mapping_data.vnic_id;
    return 0;
}
}

//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//

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
}

extern "C" {

mem_hash *mapping_tbl;

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
                      uint16_t vpc_id)
{
    sdk_table_api_params_t tparams;
    mapping_swkey_t mapping_key;
    mapping_appdata_t mapping_data;
    ip_addr_t dst;
    sdk_ret_t ret;

    pds_dst_addr_get(dst_addr, &dst);
    PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key, vpc_id, &dst);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL,
                                   &mapping_data, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_tbl->get(&tparams);
    if (ret != SDK_RET_OK) {
        return SDK_RET_ERR;
    }
    sdk::lib::memrev(mac_addr, mapping_data.dmaci, ETH_ADDR_LEN);

    return SDK_RET_OK;
}

#define data action_u.bd_bd_info
static inline int
pds_bd_vr_mac_get (mac_addr_t mac_addr, uint16_t bd_id)
{
    p4pd_error_t p4pd_ret;
    bd_actiondata_t bd_data;

    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_BD, bd_id, NULL, NULL,
                                      &bd_data);
    memcpy(mac_addr, bd_data.data.vrmac, 6);

    return p4pd_ret;
}

// Based on the config, we get either the dst mac from remote mapping table
// or subnet's vr mac from bd table
int
pds_dst_mac_get (void *hdr, mac_addr_t mac_addr, bool remote,
                 uint32_t dst_addr)
{
    uint16_t vpc_id;
    uint16_t bd_id;

    if (remote) {
        // vpc_id = ((p4_rx_cpu_hdr_t *)hdr)->vpc_id;
        vpc_id = 1;
        pds_mapping_dmac_get(mac_addr, dst_addr, vpc_id);
    } else {
        bd_id = ((p4_rx_cpu_hdr_t *)hdr)->ingress_bd_id;
        pds_bd_vr_mac_get(mac_addr, bd_id);
    }
    return SDK_RET_OK;
}

}

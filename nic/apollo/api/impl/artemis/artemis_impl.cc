//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// artemis pipeline implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
// TODO: clean this up
#include "nic/sdk/platform/capri/capri_tbl_rw.hpp"
#include "nic/sdk/platform/capri/capri_sw_phv.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/impl/artemis/artemis_impl.hpp"
#include "nic/apollo/api/impl/artemis/pds_impl_state.hpp"
#include "nic/apollo/api/impl/artemis/impl_utils.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/p4/include/artemis_defines.h"
#include "nic/apollo/p4/include/artemis_table_sizes.h"
#include "gen/platform/mem_regions.hpp"
#include "gen/p4gen/artemis/include/p4pd.h"
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"
#include "gen/p4gen/p4plus_rxdma/include/ingress_phv.h"

extern sdk_ret_t init_service_lif(uint32_t lif_id, const char *cfg_path);

#define MEM_REGION_RXDMA_PROGRAM_NAME        "rxdma_program"
#define MEM_REGION_TXDMA_PROGRAM_NAME        "txdma_program"
#define MEM_REGION_LIF_STATS_BASE            "lif_stats_base"

#define MEM_REGION_INVALID_FLOW_BASE         "invalid_flow_base"
#define MEM_REGION_FLOW_BASE                 "flow"
#define MEM_REGION_FLOW_OHASH_BASE           "flow_ohash"
#define MEM_REGION_IPV4_FLOW_BASE            "ipv4_flow"
#define MEM_REGION_IPV4_FLOW_OHASH_BASE      "ipv4_flow_ohash"

#define MEM_REGION_FLOW_NAME                 "flow"
#define MEM_REGION_FLOW_OHASH_NAME           "flow_ohash"
#define MEM_REGION_IPV4_FLOW_NAME            "ipv4_flow"
#define MEM_REGION_IPV4_FLOW_OHASH_NAME      "ipv4_flow_ohash"
#define MEM_REGION_SESSION_STATS_NAME        "session_stats"

#define MEM_REGION_SESSION2FLOW_BASE         "session2flow"
#define MEM_REGION_SESSION2FLOW_NAME         "session2flow"

using ftlite::internal::ipv6_entry_t;
using ftlite::internal::ipv4_entry_t;

#define RXDMA_SYMBOLS_MAX                    2
#define TXDMA_SYMBOLS_MAX                    6

namespace api {
namespace impl {

#define ARTEMIS_PHV_SIZE (4096 / 8)

uint8_t data[ARTEMIS_PHV_SIZE];

void
artemis_impl::sort_mpu_programs_(std::vector<std::string>& programs) {
    sort_mpu_programs_compare sort_compare;

    for (uint32_t tableid = p4pd_tableid_min_get();
         tableid < p4pd_tableid_max_get(); tableid++) {
        p4pd_table_properties_t tbl_ctx;
        if (p4pd_table_properties_get(tableid, &tbl_ctx) != P4PD_FAIL) {
            sort_compare.add_table(std::string(tbl_ctx.tablename), tbl_ctx);
        }
    }
    sort(programs.begin(), programs.end(), sort_compare);
}

uint32_t
artemis_impl::rxdma_symbols_init_(void **p4plus_symbols,
                                  platform_type_t platform_type)
{
    uint32_t    i = 0;

    *p4plus_symbols =
        (sdk::p4::p4_param_info_t *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_RXDMA_SYMBOLS,
                   RXDMA_SYMBOLS_MAX * sizeof(sdk::p4::p4_param_info_t));
    sdk::p4::p4_param_info_t *symbols =
        (sdk::p4::p4_param_info_t *)(*p4plus_symbols);

    symbols[i].name = MEM_REGION_LIF_STATS_BASE;
    symbols[i].val = api::g_pds_state.mempartition()->start_addr(MEM_REGION_LIF_STATS_NAME);
    SDK_ASSERT(symbols[i].val != INVALID_MEM_ADDRESS);
    i++;
    symbols[i].name = MEM_REGION_SESSION2FLOW_BASE;
    symbols[i].val = api::g_pds_state.mempartition()->start_addr(MEM_REGION_SESSION2FLOW_NAME);
    SDK_ASSERT(symbols[i].val != INVALID_MEM_ADDRESS);
    i++;
    SDK_ASSERT(i <= RXDMA_SYMBOLS_MAX);

    return i;
}

uint32_t
artemis_impl::txdma_symbols_init_(void **p4plus_symbols,
                                  platform_type_t platform_type)
{
    uint32_t    i = 0;

    *p4plus_symbols =
        (sdk::p4::p4_param_info_t *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_TXDMA_SYMBOLS,
                   TXDMA_SYMBOLS_MAX * sizeof(sdk::p4::p4_param_info_t));
    sdk::p4::p4_param_info_t *symbols =
        (sdk::p4::p4_param_info_t *)(*p4plus_symbols);

    symbols[i].name = MEM_REGION_LIF_STATS_BASE;
    symbols[i].val = api::g_pds_state.mempartition()->start_addr(MEM_REGION_LIF_STATS_NAME);
    SDK_ASSERT(symbols[i].val != INVALID_MEM_ADDRESS);
    i++;

    symbols[i].name = MEM_REGION_INVALID_FLOW_BASE;
    symbols[i].val = 0xc0000000;
    i++;

    symbols[i].name = MEM_REGION_FLOW_BASE;
    symbols[i].val = api::g_pds_state.mempartition()->start_addr(MEM_REGION_FLOW_NAME);
    SDK_ASSERT(symbols[i].val != INVALID_MEM_ADDRESS);
    i++;

    symbols[i].name = MEM_REGION_FLOW_OHASH_BASE;
    symbols[i].val = api::g_pds_state.mempartition()->start_addr(MEM_REGION_FLOW_OHASH_NAME);
    SDK_ASSERT(symbols[i].val != INVALID_MEM_ADDRESS);
    i++;

    symbols[i].name = MEM_REGION_IPV4_FLOW_BASE;
    symbols[i].val = api::g_pds_state.mempartition()->start_addr(MEM_REGION_IPV4_FLOW_BASE);
    SDK_ASSERT(symbols[i].val != INVALID_MEM_ADDRESS);
    i++;

    symbols[i].name = MEM_REGION_IPV4_FLOW_OHASH_BASE;
    symbols[i].val = api::g_pds_state.mempartition()->start_addr(MEM_REGION_IPV4_FLOW_OHASH_NAME);
    SDK_ASSERT(symbols[i].val != INVALID_MEM_ADDRESS);
    i++;

    SDK_ASSERT(i <= TXDMA_SYMBOLS_MAX);

    return i;
}

sdk_ret_t
artemis_impl::init_(pipeline_cfg_t *pipeline_cfg) {
    pipeline_cfg_ = *pipeline_cfg;
    return SDK_RET_OK;
}

artemis_impl *
artemis_impl::factory(pipeline_cfg_t *pipeline_cfg) {
    artemis_impl    *impl;

    impl = (artemis_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_PIPELINE_IMPL,
                                      sizeof(artemis_impl));
    new (impl) artemis_impl();
    if (impl->init_(pipeline_cfg) != SDK_RET_OK) {
        impl->~artemis_impl();
        SDK_FREE(SDK_MEM_ALLOC_PDS_PIPELINE_IMPL, impl);
        return NULL;
    }
    return impl;
}

void
artemis_impl::destroy(artemis_impl *impl) {
    int i;

    // remove key native table entries
    for (i = 0; i < MAX_KEY_NATIVE_TBL_ENTRIES; i++) {
        artemis_impl_db()->key_native_tbl()->remove(
            artemis_impl_db()->key_native_tbl_idx_[i]);
    }
    // remove key tunneled table entries
    for (i = 0; i < MAX_KEY_TUNNELED_TBL_ENTRIES; i++) {
        artemis_impl_db()->key_tunneled_tbl()->remove(
            artemis_impl_db()->key_tunneled_tbl_idx_[i]);
    }
    // remove key tunneled2 table entries
    for (i = 0; i < MAX_KEY_TUNNELED_TBL_ENTRIES; i++) {
        artemis_impl_db()->key_tunneled2_tbl()->remove(
            artemis_impl_db()->key_tunneled2_tbl_idx_[i]);
    }
    // remove drop stats table entries
    for (i = P4E_DROP_REASON_MIN; i <= P4E_DROP_REASON_MAX; i++) {
        artemis_impl_db()->egress_drop_stats_tbl()->remove(i);
    }
    for (i = P4I_DROP_REASON_MIN; i <= P4I_DROP_REASON_MAX; i++) {
        artemis_impl_db()->ingress_drop_stats_tbl()->remove(i);
    }
    api::impl::pds_impl_state::destroy(&api::impl::g_pds_impl_state);
    p4pd_cleanup();
}

void
artemis_impl::program_config_init(pds_init_params_t *init_params,
                                 asic_cfg_t *asic_cfg) {
    asic_cfg->num_pgm_cfgs = 3;
    memset(asic_cfg->pgm_cfg, 0, sizeof(asic_cfg->pgm_cfg));
    asic_cfg->pgm_cfg[0].path = std::string("p4_bin");
    asic_cfg->pgm_cfg[1].path = std::string("rxdma_bin");
    asic_cfg->pgm_cfg[2].path = std::string("txdma_bin");
}

void
artemis_impl::asm_config_init(pds_init_params_t *init_params,
                              asic_cfg_t *asic_cfg) {
    asic_cfg->num_asm_cfgs = 3;
    memset(asic_cfg->asm_cfg, 0, sizeof(asic_cfg->asm_cfg));
    asic_cfg->asm_cfg[0].name = init_params->pipeline + "_p4";
    asic_cfg->asm_cfg[0].path = std::string("p4_asm");
    asic_cfg->asm_cfg[0].base_addr = std::string(MEM_REGION_P4_PROGRAM_NAME);
    asic_cfg->asm_cfg[0].sort_func = sort_mpu_programs_;
    asic_cfg->asm_cfg[1].name = init_params->pipeline + "_rxdma";
    asic_cfg->asm_cfg[1].path = std::string("rxdma_asm");
    asic_cfg->asm_cfg[1].base_addr = std::string(MEM_REGION_RXDMA_PROGRAM_NAME);
    asic_cfg->asm_cfg[1].symbols_func = rxdma_symbols_init_;
    asic_cfg->asm_cfg[2].name = init_params->pipeline + "_txdma";
    asic_cfg->asm_cfg[2].path = std::string("txdma_asm");
    asic_cfg->asm_cfg[2].base_addr = std::string(MEM_REGION_TXDMA_PROGRAM_NAME);
    asic_cfg->asm_cfg[2].symbols_func = txdma_symbols_init_;
}

void
artemis_impl::ring_config_init(asic_cfg_t *asic_cfg) {
    asic_cfg->num_rings = 0;
}

sdk_ret_t
artemis_impl::key_native_init_(void) {
    sdk_ret_t                  ret;
    uint32_t                   idx = 0;
    key_native_swkey_t         key;
    key_native_swkey_mask_t    mask;
    key_native_actiondata_t    data;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // entry for native IPv4 packets
    key.ipv4_1_valid = 1;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 0;
    key.ipv4_2_valid = 0;
    key.ipv6_2_valid = 0;
    key.ethernet_3_valid = 0;
    key.ipv4_3_valid = 0;
    key.ipv6_3_valid = 0;
    data.action_id = KEY_NATIVE_NATIVE_IPV4_PACKET_ID;
    mask.ipv4_1_valid_mask = 0xFF;
    mask.ipv6_1_valid_mask = 0xFF;
    mask.ethernet_2_valid_mask = 0xFF;
    mask.ipv4_2_valid_mask = 0xFF;
    mask.ipv6_2_valid_mask = 0xFF;
    mask.ethernet_3_valid_mask = 0xFF;
    mask.ipv4_3_valid_mask = 0xFF;
    mask.ipv6_3_valid_mask = 0xFF;
    ret = artemis_impl_db()->key_native_tbl()->insert(&key, &mask, &data,
              &artemis_impl_db()->key_native_tbl_idx_[idx++]);
    SDK_ASSERT(ret == SDK_RET_OK);

    // entry for native IPv6 packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    key.ipv4_1_valid = 0;
    key.ipv6_1_valid = 1;
    key.ethernet_2_valid = 0;
    key.ipv4_2_valid = 0;
    key.ipv6_2_valid = 0;
    key.ethernet_3_valid = 0;
    key.ipv4_3_valid = 0;
    key.ipv6_3_valid = 0;
    data.action_id = KEY_NATIVE_NATIVE_IPV6_PACKET_ID;
    mask.ipv4_1_valid_mask = 0xFF;
    mask.ipv6_1_valid_mask = 0xFF;
    mask.ethernet_2_valid_mask = 0xFF;
    mask.ipv4_2_valid_mask = 0xFF;
    mask.ipv6_2_valid_mask = 0xFF;
    mask.ethernet_3_valid_mask = 0xFF;
    mask.ipv4_3_valid_mask = 0xFF;
    mask.ipv6_3_valid_mask = 0xFF;
    ret = artemis_impl_db()->key_native_tbl()->insert(&key, &mask, &data,
              &artemis_impl_db()->key_native_tbl_idx_[idx++]);
    SDK_ASSERT(ret == SDK_RET_OK);

    // entry for native non-IP packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    key.ipv4_1_valid = 0;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 0;
    key.ipv4_2_valid = 0;
    key.ipv6_2_valid = 0;
    key.ethernet_3_valid = 0;
    key.ipv4_3_valid = 0;
    key.ipv6_3_valid = 0;
    data.action_id = KEY_NATIVE_NATIVE_NONIP_PACKET_ID;
    mask.ipv4_1_valid_mask = 0xFF;
    mask.ipv6_1_valid_mask = 0xFF;
    mask.ethernet_2_valid_mask = 0xFF;
    mask.ipv4_2_valid_mask = 0xFF;
    mask.ipv6_2_valid_mask = 0xFF;
    mask.ethernet_3_valid_mask = 0xFF;
    mask.ipv4_3_valid_mask = 0xFF;
    mask.ipv6_3_valid_mask = 0xFF;
    ret = artemis_impl_db()->key_native_tbl()->insert(&key, &mask, &data,
              &artemis_impl_db()->key_native_tbl_idx_[idx++]);
    SDK_ASSERT(ret == SDK_RET_OK);

    // entry for tunneled/double tunneled packets
    key.ipv4_1_valid = 0;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 1;
    key.ipv4_2_valid = 0;
    key.ipv6_2_valid = 0;
    key.ethernet_3_valid = 0;
    key.ipv4_3_valid = 0;
    key.ipv6_3_valid = 0;
    data.action_id = KEY_NATIVE_SET_TEP1_DST_ID;
    mask.ipv4_1_valid_mask = 0;
    mask.ipv6_1_valid_mask = 0;
    mask.ethernet_2_valid_mask = 0xFF;
    mask.ipv4_2_valid_mask = 0;
    mask.ipv6_2_valid_mask = 0;
    mask.ethernet_3_valid_mask = 0;
    mask.ipv4_3_valid_mask = 0;
    mask.ipv6_3_valid_mask = 0;
    ret = artemis_impl_db()->key_native_tbl()->insert(&key, &mask, &data,
              &artemis_impl_db()->key_native_tbl_idx_[idx++]);
    SDK_ASSERT(ret == SDK_RET_OK);

    // check overflow
    SDK_ASSERT(idx <= MAX_KEY_NATIVE_TBL_ENTRIES);
    return ret;
}

sdk_ret_t
artemis_impl::key_tunneled_init_(void) {
    sdk_ret_t                    ret;
    uint32_t                     idx = 0;
    key_tunneled_swkey_t         key;
    key_tunneled_swkey_mask_t    mask;
    key_tunneled_actiondata_t    data;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0xFF, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // entry for tunneled (inner) IPv4 packets (outer can be IPv4 or IPv6)
    key.ipv4_1_valid = 0;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 1;
    key.ipv4_2_valid = 1;
    key.ipv6_2_valid = 0;
    key.ethernet_3_valid = 0;
    key.ipv4_3_valid = 0;
    key.ipv6_3_valid = 0;
    data.action_id = KEY_TUNNELED_TUNNELED_IPV4_PACKET_ID;
    mask.ipv4_1_valid_mask = 0;
    mask.ipv6_1_valid_mask = 0;
    mask.ethernet_2_valid_mask = 0xFF;
    mask.ipv4_2_valid_mask = 0xFF;
    mask.ipv6_2_valid_mask = 0xFF;
    mask.ethernet_3_valid_mask = 0xFF;
    mask.ipv4_3_valid_mask = 0xFF;
    mask.ipv6_3_valid_mask = 0xFF;
    ret = artemis_impl_db()->key_tunneled_tbl()->insert(
              &key, &mask, &data,
              &artemis_impl_db()->key_tunneled_tbl_idx_[idx++]);
    SDK_ASSERT(ret == SDK_RET_OK);

    // entry for tunneled (inner) IPv6 packets (outer can be IPv4 or IPv6)
    key.ipv4_1_valid = 0;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 1;
    key.ipv4_2_valid = 0;
    key.ipv6_2_valid = 1;
    key.ethernet_3_valid = 0;
    key.ipv4_3_valid = 0;
    key.ipv6_3_valid = 0;
    data.action_id = KEY_TUNNELED_TUNNELED_IPV6_PACKET_ID;
    mask.ipv4_1_valid_mask = 0;
    mask.ipv6_1_valid_mask = 0;
    mask.ethernet_2_valid_mask = 0xFF;
    mask.ipv4_2_valid_mask = 0xFF;
    mask.ipv6_2_valid_mask = 0xFF;
    mask.ethernet_3_valid_mask = 0xFF;
    mask.ipv4_3_valid_mask = 0xFF;
    mask.ipv6_3_valid_mask = 0xFF;
    ret = artemis_impl_db()->key_tunneled_tbl()->insert(
              &key, &mask, &data,
              &artemis_impl_db()->key_tunneled_tbl_idx_[idx++]);
    SDK_ASSERT(ret == SDK_RET_OK);

    // entry for tunneled (inner) non-IP packets (outer can be IPv4 or IPv6)
    key.ipv4_1_valid = 0;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 1;
    key.ipv4_2_valid = 0;
    key.ipv6_2_valid = 0;
    key.ethernet_3_valid = 0;
    key.ipv4_3_valid = 0;
    key.ipv6_3_valid = 0;
    data.action_id = KEY_TUNNELED_TUNNELED_NONIP_PACKET_ID;
    mask.ipv4_1_valid_mask = 0;
    mask.ipv6_1_valid_mask = 0;
    mask.ethernet_2_valid_mask = 0xFF;
    mask.ipv4_2_valid_mask = 0xFF;
    mask.ipv6_2_valid_mask = 0xFF;
    mask.ethernet_3_valid_mask = 0xFF;
    mask.ipv4_3_valid_mask = 0xFF;
    mask.ipv6_3_valid_mask = 0xFF;
    ret = artemis_impl_db()->key_tunneled_tbl()->insert(
              &key, &mask, &data,
              &artemis_impl_db()->key_tunneled_tbl_idx_[idx++]);
    SDK_ASSERT(ret == SDK_RET_OK);

    // entry for double tunneled packets (outer headers can be IPv4 or IPv6)
    key.ipv4_1_valid = 0;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 0;
    key.ipv4_2_valid = 0;
    key.ipv6_2_valid = 0;
    key.ethernet_3_valid = 1;
    key.ipv4_3_valid = 0;
    key.ipv6_3_valid = 0;
    data.action_id = KEY_TUNNELED_SET_TEP2_DST_ID;
    mask.ipv4_1_valid_mask = 0;
    mask.ipv6_1_valid_mask = 0;
    mask.ethernet_2_valid_mask = 0;
    mask.ipv4_2_valid_mask = 0;
    mask.ipv6_2_valid_mask = 0;
    mask.ethernet_3_valid_mask = 0xFF;
    mask.ipv4_3_valid_mask = 0;
    mask.ipv6_3_valid_mask = 0;
    ret = artemis_impl_db()->key_tunneled_tbl()->insert(
              &key, &mask, &data,
              &artemis_impl_db()->key_tunneled_tbl_idx_[idx++]);
    SDK_ASSERT(ret == SDK_RET_OK);

    // check max
    SDK_ASSERT(idx <= MAX_KEY_TUNNELED_TBL_ENTRIES);
    return ret;
}

sdk_ret_t
artemis_impl::key_tunneled2_init_(void) {
    sdk_ret_t                      ret;
    uint32_t                       idx = 0;
    key_tunneled2_swkey_t          key;
    key_tunneled2_swkey_mask_t     mask;
    key_tunneled2_actiondata_t     data;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0xFF, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // entry for double tunneled (inner-most) IPv4 packets (outer headers can
    // be IPv4 or IPv6)
    key.ipv4_1_valid = 0;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 0;
    key.ipv4_2_valid = 0;
    key.ipv6_2_valid = 0;
    key.ethernet_3_valid = 1;
    key.ipv4_3_valid = 1;
    key.ipv6_3_valid = 0;
    data.action_id = KEY_TUNNELED2_TUNNELED2_IPV4_PACKET_ID;
    mask.ipv4_1_valid_mask = 0;
    mask.ipv6_1_valid_mask = 0;
    mask.ethernet_2_valid_mask = 0;
    mask.ipv4_2_valid_mask = 0;
    mask.ipv6_2_valid_mask = 0;
    mask.ethernet_3_valid_mask = 0xFF;
    mask.ipv4_3_valid_mask = 0xFF;
    mask.ipv6_3_valid_mask = 0xFF;
    ret = artemis_impl_db()->key_tunneled2_tbl()->insert(
              &key, &mask, &data,
              &artemis_impl_db()->key_tunneled2_tbl_idx_[idx++]);

    // entry for double tunneled (inner-most) IPv6 packets (outer headers can
    // be IPv4 or IPv6)
    key.ipv4_1_valid = 0;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 0;
    key.ipv4_2_valid = 0;
    key.ipv6_2_valid = 0;
    key.ethernet_3_valid = 1;
    key.ipv4_3_valid = 0;
    key.ipv6_3_valid = 1;
    data.action_id = KEY_TUNNELED2_TUNNELED2_IPV6_PACKET_ID;
    mask.ipv4_1_valid_mask = 0;
    mask.ipv6_1_valid_mask = 0;
    mask.ethernet_2_valid_mask = 0;
    mask.ipv4_2_valid_mask = 0;
    mask.ipv6_2_valid_mask = 0;
    mask.ethernet_3_valid_mask = 0xFF;
    mask.ipv4_3_valid_mask = 0xFF;
    mask.ipv6_3_valid_mask = 0xFF;
    ret = artemis_impl_db()->key_tunneled2_tbl()->insert(
              &key, &mask, &data,
              &artemis_impl_db()->key_tunneled2_tbl_idx_[idx++]);

    // entry for tunneled (inner) non-IP packets (outer can be IPv4 or IPv6)
    key.ipv4_1_valid = 0;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 0;
    key.ipv4_2_valid = 0;
    key.ipv6_2_valid = 0;
    key.ethernet_3_valid = 1;
    key.ipv4_3_valid = 0;
    key.ipv6_3_valid = 0;
    data.action_id = KEY_TUNNELED2_TUNNELED2_NONIP_PACKET_ID;
    mask.ipv4_1_valid_mask = 0;
    mask.ipv6_1_valid_mask = 0;
    mask.ethernet_2_valid_mask = 0;
    mask.ipv4_2_valid_mask = 0;
    mask.ipv6_2_valid_mask = 0;
    mask.ethernet_3_valid_mask = 0xFF;
    mask.ipv4_3_valid_mask = 0xFF;
    mask.ipv6_3_valid_mask = 0xFF;
    ret = artemis_impl_db()->key_tunneled2_tbl()->insert(
              &key, &mask, &data,
              &artemis_impl_db()->key_tunneled2_tbl_idx_[idx++]);

    // check max
    SDK_ASSERT(idx <= MAX_KEY_TUNNELED_TBL_ENTRIES);
    return ret;
}

sdk_ret_t
artemis_impl::inter_pipe_init_(void) {
    p4pd_error_t p4pd_ret;
    inter_pipe_ingress_actiondata_t data = { 0 };

    data.action_id = INTER_PIPE_INGRESS_INGRESS_TO_EGRESS_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_INTER_PIPE_INGRESS,
                                       PIPE_EGRESS,
                                       NULL, NULL, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    data.action_id = INTER_PIPE_INGRESS_INGRESS_TO_CPS_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_INTER_PIPE_INGRESS,
                                       PIPE_CPS,
                                       NULL, NULL, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    data.action_id = INTER_PIPE_INGRESS_INGRESS_TO_ARM_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_INTER_PIPE_INGRESS,
                                       PIPE_ARM,
                                       NULL, NULL, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    data.action_id = INTER_PIPE_INGRESS_INGRESS_TO_CLASSIC_NIC_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_INTER_PIPE_INGRESS,
                                       PIPE_CLASSIC_NIC,
                                       NULL, NULL, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    data.action_id = INTER_PIPE_INGRESS_INGRESS_TO_UPLINK_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_INTER_PIPE_INGRESS,
                                       PIPE_UPLINK,
                                       NULL, NULL, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    data.action_id = INTER_PIPE_INGRESS_INGRESS_TO_INGRESS_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_INTER_PIPE_INGRESS,
                                       PIPE_INGRESS,
                                       NULL, NULL, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    return SDK_RET_OK;
}

sdk_ret_t
artemis_impl::egress_drop_stats_init_(void) {
    sdk_ret_t                      ret;
    p4e_drop_stats_swkey_t         key = { 0 };
    p4e_drop_stats_swkey_mask_t    key_mask = { 0 };
    p4e_drop_stats_actiondata_t    data = { 0 };

    for (uint32_t i = P4E_DROP_REASON_MIN; i <= P4E_DROP_REASON_MAX; i++) {
        key.control_metadata_p4e_drop_reason = ((uint32_t)1 << i);
        key_mask.control_metadata_p4e_drop_reason_mask =
            key.control_metadata_p4e_drop_reason;
        data.action_id = P4E_DROP_STATS_P4E_DROP_STATS_ID;
        ret =
            artemis_impl_db()->egress_drop_stats_tbl()->insert_withid(&key,
                                                                     &key_mask,
                                                                     &data, i);
        SDK_ASSERT(ret == SDK_RET_OK);
    }
    return ret;
}

sdk_ret_t
artemis_impl::ingress_drop_stats_init_(void) {
    sdk_ret_t                      ret;
    p4i_drop_stats_swkey_t         key = { 0 };
    p4i_drop_stats_swkey_mask_t    key_mask = { 0 };
    p4i_drop_stats_actiondata_t    data = { 0 };

    for (uint32_t i = P4I_DROP_REASON_MIN; i <= P4I_DROP_REASON_MAX; i++) {
        key.control_metadata_p4i_drop_reason = ((uint32_t)1 << i);
        key_mask.control_metadata_p4i_drop_reason_mask =
            key.control_metadata_p4i_drop_reason;
        data.action_id = P4I_DROP_STATS_P4I_DROP_STATS_ID;
        ret =
            artemis_impl_db()->ingress_drop_stats_tbl()->insert_withid(&key,
                                                                     &key_mask,
                                                                     &data, i);
        SDK_ASSERT(ret == SDK_RET_OK);
    }
    return ret;
}

sdk_ret_t
artemis_impl::stats_init_(void) {
    ingress_drop_stats_init_();
    egress_drop_stats_init_();
    return SDK_RET_OK;
}

#define nacl_redirect_action    action_u.nacl_nacl_redirect
sdk_ret_t
artemis_impl::nacl_init_(void) {
    uint32_t idx;
    sdk_ret_t ret;
    nacl_swkey_t key = { 0 };
    nacl_swkey_mask_t mask = { 0 };
    nacl_actiondata_t data =  { 0 };

    if (g_pds_state.platform_type() == platform_type_t::PLATFORM_TYPE_SIM) {
        // flow miss packet coming back from txdma -> CPU (but in SIM mode
        // we redirect it uplink for validation purposes)
        key.cps_blob_valid = 1;
        mask.cps_blob_valid_mask = 0xFF;
        data.action_id = NACL_NACL_REDIRECT_ID;
        data.nacl_redirect_action.pipe_id = PIPE_ARM;
        data.action_u.nacl_nacl_redirect.oport = TM_PORT_UPLINK_1;
        data.action_u.nacl_nacl_redirect.lif = 0;
        data.action_u.nacl_nacl_redirect.qtype = 0;
        data.action_u.nacl_nacl_redirect.qid = 0;
        data.action_u.nacl_nacl_redirect.vlan_strip = 0;
        ret = artemis_impl_db()->nacl_tbl()->insert(&key, &mask, &data, &idx);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to program NACL entry for uplink redirect, "
                          "err %u", ret);
            return ret;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
artemis_impl::checksum_init_(void) {
    uint64_t idx;
    p4pd_error_t p4pd_ret;
    checksum_swkey_t key;
    checksum_actiondata_t data;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.ipv4_1_valid = 1;
    data.action_id = CHECKSUM_UPDATE_IPV4_CHECKSUM_ID;
    idx = p4pd_index_to_hwindex_map(P4TBL_ID_CHECKSUM, &key);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_CHECKSUM,
                                       idx, NULL, NULL, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.ipv4_1_valid = 1;
    key.udp_1_valid = 1;
    data.action_id = CHECKSUM_UPDATE_IPV4_UDP_CHECKSUM_ID;
    idx = p4pd_index_to_hwindex_map(P4TBL_ID_CHECKSUM, &key);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_CHECKSUM,
                                       idx, NULL, NULL, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.ipv4_1_valid = 1;
    key.tcp_valid = 1;
    data.action_id = CHECKSUM_UPDATE_IPV4_TCP_CHECKSUM_ID;
    idx = p4pd_index_to_hwindex_map(P4TBL_ID_CHECKSUM, &key);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_CHECKSUM,
                                       idx, NULL, NULL, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.ipv4_1_valid = 1;
    key.icmp_valid = 1;
    data.action_id = CHECKSUM_UPDATE_IPV4_ICMP_CHECKSUM_ID;
    idx = p4pd_index_to_hwindex_map(P4TBL_ID_CHECKSUM, &key);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_CHECKSUM,
                                       idx, NULL, NULL, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.ipv6_1_valid = 1;
    key.udp_1_valid = 1;
    data.action_id = CHECKSUM_UPDATE_IPV6_UDP_CHECKSUM_ID;
    idx = p4pd_index_to_hwindex_map(P4TBL_ID_CHECKSUM, &key);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_CHECKSUM,
                                       idx, NULL, NULL, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.ipv6_1_valid = 1;
    key.tcp_valid = 1;
    data.action_id = CHECKSUM_UPDATE_IPV6_TCP_CHECKSUM_ID;
    idx = p4pd_index_to_hwindex_map(P4TBL_ID_CHECKSUM, &key);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_CHECKSUM,
                                       idx, NULL, NULL, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.ipv6_1_valid = 1;
    key.icmp_valid = 1;
    data.action_id = CHECKSUM_UPDATE_IPV6_ICMP_CHECKSUM_ID;
    idx = p4pd_index_to_hwindex_map(P4TBL_ID_CHECKSUM, &key);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_CHECKSUM,
                                       idx, NULL, NULL, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
artemis_impl::table_init_(void) {
    sdk_ret_t     ret;
    mem_addr_t    addr;

    ret = key_native_init_();
    if (ret != SDK_RET_OK) {
        return ret;
    }
    ret = key_tunneled_init_();
    if (ret != SDK_RET_OK) {
        return ret;
    }
    ret = key_tunneled2_init_();
    if (ret != SDK_RET_OK) {
        return ret;
    }
    ret = inter_pipe_init_();
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // program session stats table base address as table constant
    // of session table
    addr = api::g_pds_state.mempartition()->start_addr("session_stats");
    SDK_ASSERT(addr != INVALID_MEM_ADDRESS);
    // subtract 2G (saves ASM instructions)
    addr -= ((uint64_t)1 << 31);
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_SESSION, addr);

    // program IPv4 tag tree's root address as table constant of v4 flow tables
    addr = api::g_pds_state.mempartition()->start_addr("tag_v4");
    SDK_ASSERT(addr != INVALID_MEM_ADDRESS);
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_IPV4_FLOW, addr);
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_IPV4_FLOW_OHASH,
                                                 addr);

    // program IPv6 tag tree's root address as table constant of v6 flow tables
    addr = api::g_pds_state.mempartition()->start_addr("tag_v6");
    SDK_ASSERT(addr != INVALID_MEM_ADDRESS);
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_FLOW, addr);
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_FLOW_OHASH, addr);

    // program meter stats region base address as table constant
    addr = api::g_pds_state.mempartition()->start_addr("meter_stats");
    SDK_ASSERT(addr != INVALID_MEM_ADDRESS);
    // subtract 2G (saves ASM instructions)
    addr -= ((uint64_t)1 << 31);
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_METER_STATS, addr);

    // initialize checksum table
    ret = checksum_init_();
    SDK_ASSERT(ret == SDK_RET_OK);

    // initialize stats tables
    ret = stats_init_();
    SDK_ASSERT(ret == SDK_RET_OK);

    // install all default NACL entries
    ret = nacl_init_();
    SDK_ASSERT(ret == SDK_RET_OK);

    return SDK_RET_OK;
}

sdk_ret_t
artemis_impl::p4plus_table_init_(void) {
    p4plus_prog_t prog;
    p4pd_table_properties_t tbl_ctx_apphdr;
    p4pd_table_properties_t tbl_ctx_txdma_act;
    p4pd_table_properties_t tbl_ctx_apphdr_off;

    p4pd_global_table_properties_get(P4_P4PLUS_RXDMA_TBL_ID_COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE,
                                     &tbl_ctx_apphdr);
    memset(&prog, 0, sizeof(prog));
    prog.stageid = tbl_ctx_apphdr.stage;
    prog.stage_tableid = tbl_ctx_apphdr.stage_tableid;
    prog.stage_tableid_off = tbl_ctx_apphdr_off.stage_tableid;
    prog.control = "artemis_rxdma";
    prog.prog_name = "rxdma_stage0.bin";
    prog.pipe = P4_PIPELINE_RXDMA;
    sdk::platform::capri::capri_p4plus_table_init(&prog, api::g_pds_state.platform_type());

    p4pd_global_table_properties_get(P4_P4PLUS_TXDMA_TBL_ID_TX_TABLE_S0_T0,
                                     &tbl_ctx_txdma_act);
    memset(&prog, 0, sizeof(prog));
    prog.stageid = tbl_ctx_txdma_act.stage;
    prog.stage_tableid = tbl_ctx_txdma_act.stage_tableid;
    prog.control = "artemis_txdma";
    prog.prog_name = "txdma_stage0.bin";
    prog.pipe = P4_PIPELINE_TXDMA;
    sdk::platform::capri::capri_p4plus_table_init(&prog, api::g_pds_state.platform_type());

    return SDK_RET_OK;
}

sdk_ret_t
artemis_impl::pipeline_init(void) {
    sdk_ret_t  ret;
    p4pd_cfg_t p4pd_cfg;
    std::string cfg_path = api::g_pds_state.cfg_path();

    p4pd_cfg.cfg_path = cfg_path.c_str();
    ret = pipeline_p4_hbm_init(&p4pd_cfg);
    SDK_ASSERT(ret == SDK_RET_OK);

    // skip the remaining if it is a soft initialization
    if (sdk::asic::asic_is_soft_init()) {
        return SDK_RET_OK;
    }

    ret = sdk::asic::pd::asicpd_p4plus_table_mpu_base_init(&p4pd_cfg);
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = sdk::asic::pd::asicpd_toeplitz_init("artemis_rxdma",
                             P4_P4PLUS_RXDMA_TBL_ID_ETH_RX_RSS_INDIR);
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = p4plus_table_init_();
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = sdk::asic::pd::asicpd_table_mpu_base_init(&p4pd_cfg);
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = sdk::asic::pd::asicpd_program_table_mpu_pc();
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = sdk::asic::pd::asicpd_deparser_init();
    SDK_ASSERT(ret == SDK_RET_OK);
    g_pds_impl_state.init(&api::g_pds_state);
    api::g_pds_state.lif_db()->impl_state_set(g_pds_impl_state.lif_impl_db());

    ret = init_service_lif(ARTEMIS_SERVICE_LIF, p4pd_cfg.cfg_path);
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = table_init_();
    SDK_ASSERT(ret == SDK_RET_OK);

    ret = sdk::asic::pd::asicpd_sw_phv_init();
    SDK_ASSERT(ret == SDK_RET_OK);

    bzero(data, ARTEMIS_PHV_SIZE);

    p4plus_rxdma_ingress_phv_t *phv = (p4plus_rxdma_ingress_phv_t *) data;

    phv->p4_to_rxdma_aging_enable = 1;
    phv->p4_to_rxdma_cps_path_en = 1;

#ifdef SESSION_AGING
    ret = asicpd_sw_phv_inject(ASICPD_SWPHV_TYPE_RXDMA, 0, 0, 1, data);
    SDK_ASSERT(ret == SDK_RET_OK);
#endif

    return SDK_RET_OK;
}

sdk_ret_t
artemis_impl::write_to_rxdma_table(mem_addr_t addr, uint32_t tableid,
                                   uint8_t action_id, void *actiondata) {
    uint32_t     len;
    uint8_t      packed_bytes[CACHE_LINE_SIZE];
    uint8_t      *packed_entry = packed_bytes;

    if (p4pd_rxdma_get_max_action_id(tableid) > 1) {
        struct line_s {
            uint8_t action_pc;
            uint8_t packed_entry[CACHE_LINE_SIZE-sizeof(action_pc)];
        };

        auto line = (struct line_s *) packed_bytes;
        line->action_pc = sdk::asic::pd::asicpd_get_action_pc(tableid,
                                                              action_id);
        packed_entry = line->packed_entry;
    }

    p4pd_p4plus_rxdma_raw_table_hwentry_query(tableid, action_id, &len);
    p4pd_p4plus_rxdma_entry_pack(tableid, action_id,
                                  actiondata, packed_entry);
    return asic_mem_write(addr, packed_bytes, 1 + (len >> 3),
                          ASIC_WRITE_MODE_WRITE_THRU);
}

sdk_ret_t
artemis_impl::write_to_txdma_table(mem_addr_t addr, uint32_t tableid,
                                   uint8_t action_id, void *actiondata) {
    uint32_t     len;
    uint8_t      packed_bytes[CACHE_LINE_SIZE];
    uint8_t      *packed_entry = packed_bytes;

    if (p4pd_txdma_get_max_action_id(tableid) > 1) {
        struct line_s {
            uint8_t action_pc;
            uint8_t packed_entry[CACHE_LINE_SIZE-sizeof(action_pc)];
        };

        auto line = (struct line_s *) packed_bytes;
        line->action_pc = sdk::asic::pd::asicpd_get_action_pc(tableid,
                                                              action_id);
        packed_entry = line->packed_entry;
    }

    p4pd_p4plus_txdma_raw_table_hwentry_query(tableid, action_id, &len);
    p4pd_p4plus_txdma_entry_pack(tableid, action_id,
                                  actiondata, packed_entry);
    return asic_mem_write(addr, packed_bytes, 1 + (len >> 3),
                          ASIC_WRITE_MODE_WRITE_THRU);
}

sdk_ret_t
artemis_impl::transaction_begin(void) {
    vpc_impl_db()->table_transaction_begin();
    vnic_impl_db()->table_transaction_begin();
    mapping_impl_db()->table_transaction_begin();
    route_table_impl_db()->table_transaction_begin();
    security_policy_impl_db()->table_transaction_begin();
    meter_impl_db()->table_transaction_begin();
    nexthop_impl_db()->table_transaction_begin();
    svc_mapping_impl_db()->table_transaction_begin();
    tag_impl_db()->table_transaction_begin();
    tep_impl_db()->table_transaction_begin();
    vpc_peer_impl_db()->table_transaction_begin();
    return SDK_RET_OK;
}

sdk_ret_t
artemis_impl::transaction_end(void) {
    vpc_impl_db()->table_transaction_end();
    vnic_impl_db()->table_transaction_end();
    mapping_impl_db()->table_transaction_end();
    route_table_impl_db()->table_transaction_end();
    security_policy_impl_db()->table_transaction_end();
    meter_impl_db()->table_transaction_end();
    nexthop_impl_db()->table_transaction_end();
    svc_mapping_impl_db()->table_transaction_end();
    tag_impl_db()->table_transaction_end();
    tep_impl_db()->table_transaction_end();
    vpc_peer_impl_db()->table_transaction_end();
    return SDK_RET_OK;
}

sdk_ret_t
artemis_impl::table_stats(debug::table_stats_get_cb_t cb, void *ctxt) {
    mapping_impl_db()->table_stats(cb, ctxt);
    return SDK_RET_OK;
}

/**
 * @brief      Meter Stats Get
 * @param[in]   cb      Callback
 *              lowidx  Low Index for stats to be read
 *              highidx High Index for stats to be read
 *              ctxt    Opaque context to be passed to callback
 * @return      SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
artemis_impl::meter_stats(debug::meter_stats_get_cb_t cb, uint32_t lowidx,
                          uint32_t highidx, void *ctxt) {
    sdk_ret_t ret;
    pds_meter_debug_stats_t stats = {0};
    uint64_t tx_offset = 0, rx_offset = 0;
    uint64_t start_addr = 0;

    if (highidx > (METER_STATS_TABLE_SIZE >> 1)) {
        PDS_TRACE_ERR("Read meter stats failed, invalid index {} specified",
                highidx);
        return SDK_RET_ERR;
    }

    start_addr = api::g_pds_state.mempartition()->start_addr("meter_stats");
    for (uint32_t idx = lowidx; idx <= highidx; idx ++) {
        tx_offset = idx * 8; // Each statistics is 8B
        rx_offset = tx_offset + (METER_STATS_TABLE_SIZE << 2); // ((SIZE/2) * 8)

        stats.idx = idx;

        ret = sdk::asic::asic_mem_read(start_addr + tx_offset,
                (uint8_t *)&stats.tx_bytes, 8);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Read meter TX stats for index {} failed with err %u",
                    idx, ret);
            return ret;
        }

        ret = sdk::asic::asic_mem_read(start_addr + rx_offset,
                (uint8_t *)&stats.rx_bytes, 8);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Read meter RX stats for index {} failed with err %u",
                    idx, ret);
            return ret;
        }
        cb (&stats, ctxt);
    }

    return SDK_RET_OK;
}

sdk_ret_t
artemis_impl::session_stats(debug::session_stats_get_cb_t cb, uint32_t lowidx,
                            uint32_t highidx, void *ctxt) {
    sdk_ret_t ret;
    uint64_t offset = 0;
    uint64_t start_addr = 0;
    pds_session_debug_stats_t session_stats_entry;

    memset(&session_stats_entry, 0, sizeof(pds_session_debug_stats_t));

    start_addr = api::g_pds_state.mempartition()->start_addr(
                                                  MEM_REGION_SESSION_STATS_NAME);

    for (uint32_t idx = lowidx; idx <= highidx; idx ++) {
        offset = idx * sizeof(pds_session_debug_stats_t);

        ret = sdk::asic::asic_mem_read(start_addr + offset,
                                       (uint8_t *)&session_stats_entry,
                                       sizeof(pds_session_debug_stats_t));
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to read session stats for index %u err %u", idx, ret);
            return ret;
        }

        cb(idx, &session_stats_entry, ctxt);
    }

    return SDK_RET_OK;
}

sdk_ret_t
artemis_impl::session(debug::session_get_cb_t cb, void *ctxt) {
    return SDK_RET_OK;
}

sdk_ret_t
artemis_impl::flow(debug::flow_get_cb_t cb, void *ctxt) {
    uint32_t idx = 0;
    p4pd_error_t p4pd_ret;
    p4pd_table_properties_t prop;
    ipv6_entry_t ipv6_entry;
    ipv4_entry_t ipv4_entry;

    // Read IPv4 sessions
    p4pd_global_table_properties_get(P4TBL_ID_IPV4_FLOW, &prop);
    for (idx = 0; idx < prop.tabledepth; idx ++) {
        memcpy(&ipv4_entry, ((ipv4_entry_t *)(prop.base_mem_va)) + idx,
               sizeof(ipv4_entry_t));
        ipv4_entry.swizzle();
        if (ipv4_entry.entry_valid) {
            cb(&ipv4_entry, NULL, ctxt);
        }
    }
    p4pd_global_table_properties_get(P4TBL_ID_IPV4_FLOW_OHASH, &prop);
    for (idx = 0; idx < prop.tabledepth; idx ++) {
        memcpy(&ipv4_entry, ((ipv4_entry_t *)(prop.base_mem_va)) + idx,
               sizeof(ipv4_entry_t));
        ipv4_entry.swizzle();
        if (ipv4_entry.entry_valid) {
            cb(&ipv4_entry, NULL, ctxt);
        }
    }
    cb(NULL, NULL, ctxt);

    // Read IPv6 sessions
    p4pd_global_table_properties_get(P4TBL_ID_FLOW, &prop);
    for (idx = 0; idx < prop.tabledepth; idx ++) {
        memcpy(&ipv6_entry, ((ipv6_entry_t *)(prop.base_mem_va)) + idx,
               sizeof(ipv6_entry_t));
        ipv6_entry.swizzle();
        if (ipv6_entry.entry_valid) {
            cb(NULL, &ipv6_entry, ctxt);
        }
    }
    p4pd_global_table_properties_get(P4TBL_ID_FLOW_OHASH, &prop);
    for (idx = 0; idx < prop.tabledepth; idx ++) {
        memcpy(&ipv6_entry, ((ipv6_entry_t *)(prop.base_mem_va)) + idx,
               sizeof(ipv6_entry_t));
        ipv6_entry.swizzle();
        if (ipv6_entry.entry_valid) {
            cb(NULL, &ipv6_entry, ctxt);
        }
    }
    cb(NULL, NULL, ctxt);

    return SDK_RET_OK;
}

sdk_ret_t
artemis_impl::session_clear(uint32_t idx) {
    return SDK_RET_OK;
}

}    // namespace impl
}    // namespace api


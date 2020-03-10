//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// apulu pipeline implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/utils/utils.hpp"

// TODO: clean this up
#include "nic/sdk/platform/capri/capri_tbl_rw.hpp"
#include "nic/sdk/platform/capri/capri_common.hpp"

#include "nic/sdk/platform/ring/ring.hpp"
#include "nic/sdk/platform/pal/include/pal_mem.h"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/upgrade_state.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/impl_utils.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/p4/include/apulu_defines.h"
#include "gen/platform/mem_regions.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"

extern sdk_ret_t init_service_lif(uint32_t lif_id, const char *cfg_path);
extern sdk_ret_t service_lif_upg_verify(uint32_t lif_id, const char *cfg_path);

#define MEM_REGION_RXDMA_PROGRAM_NAME "rxdma_program"
#define MEM_REGION_TXDMA_PROGRAM_NAME "txdma_program"
#define MEM_REGION_LIF_STATS_BASE     "lif_stats_base"
#define MEM_REGION_SESSION_STATS_NAME "session_stats"

#define RXDMA_SYMBOLS_MAX             1
#define TXDMA_SYMBOLS_MAX             1

using ftlite::internal::ipv6_entry_t;
using ftlite::internal::ipv4_entry_t;

mac_addr_t g_zero_mac;

namespace api {
namespace impl {

/// \defgroup PDS_PIPELINE_IMPL - pipeline wrapper implementation
/// \ingroup PDS_PIPELINE
/// @{

void
apulu_impl::sort_mpu_programs_(std::vector<std::string>& programs) {
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
apulu_impl::rxdma_symbols_init_(void **p4plus_symbols,
                                 platform_type_t platform_type)
{
    uint32_t    i = 0;

    *p4plus_symbols =
        (sdk::p4::p4_param_info_t *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_RXDMA_SYMBOLS,
                   RXDMA_SYMBOLS_MAX * sizeof(sdk::p4::p4_param_info_t));
    sdk::p4::p4_param_info_t *symbols =
        (sdk::p4::p4_param_info_t *)(*p4plus_symbols);

    symbols[i].name = MEM_REGION_LIF_STATS_BASE;
    symbols[i].val =
        api::g_pds_state.mempartition()->start_addr(MEM_REGION_LIF_STATS_NAME);
    SDK_ASSERT(symbols[i].val != INVALID_MEM_ADDRESS);
    i++;
    SDK_ASSERT(i <= RXDMA_SYMBOLS_MAX);

    return i;
}

uint32_t
apulu_impl::txdma_symbols_init_(void **p4plus_symbols,
                                 platform_type_t platform_type)
{
    uint32_t    i = 0;

    *p4plus_symbols =
        (sdk::p4::p4_param_info_t *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_TXDMA_SYMBOLS,
                   TXDMA_SYMBOLS_MAX * sizeof(sdk::p4::p4_param_info_t));
    sdk::p4::p4_param_info_t *symbols =
        (sdk::p4::p4_param_info_t *)(*p4plus_symbols);

    symbols[i].name = MEM_REGION_LIF_STATS_BASE;
    symbols[i].val =
        api::g_pds_state.mempartition()->start_addr(MEM_REGION_LIF_STATS_NAME);
    SDK_ASSERT(symbols[i].val != INVALID_MEM_ADDRESS);
    i++;
    SDK_ASSERT(i <= TXDMA_SYMBOLS_MAX);

    return i;
}

void
apulu_impl::table_engine_cfg_backup_(p4pd_pipeline_t pipe) {
    p4_tbl_eng_cfg_t *cfg;
    uint32_t num_cfgs, max_cfgs;

    num_cfgs = api::g_upg_state->tbl_eng_cfg(pipe, &cfg, &max_cfgs);
    num_cfgs = sdk::asic::pd::asicpd_tbl_eng_cfg_get(pipe, &cfg[num_cfgs],
                                                     max_cfgs - num_cfgs);
    g_upg_state->incr_tbl_eng_cfg_count(pipe, num_cfgs);
}

sdk_ret_t
apulu_impl::init_(pipeline_cfg_t *pipeline_cfg) {
    pipeline_cfg_ = *pipeline_cfg;
    return SDK_RET_OK;
}

apulu_impl *
apulu_impl::factory(pipeline_cfg_t *pipeline_cfg) {
    apulu_impl    *impl;

    impl = (apulu_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_PIPELINE_IMPL,
                                    sizeof(apulu_impl));
    new (impl) apulu_impl();
    if (impl->init_(pipeline_cfg) != SDK_RET_OK) {
        impl->~apulu_impl();
        SDK_FREE(SDK_MEM_ALLOC_PDS_PIPELINE_IMPL, impl);
        return NULL;
    }
    return impl;
}

// TODO: usage of handles here is incorrect (may be delete by index ?)
void
apulu_impl::destroy(apulu_impl *impl) {
    int i;
    sdk_table_api_params_t       tparams = { 0 };

    // remove drop stats table entries
    for (i = P4E_DROP_REASON_MIN; i <= P4E_DROP_REASON_MAX; i++) {
        tparams.handle = apulu_impl_db()->egr_drop_stats_tbl_hdls_[i];
        apulu_impl_db()->egress_drop_stats_tbl()->remove(&tparams);
    }
    for (i = P4I_DROP_REASON_MIN; i <= P4I_DROP_REASON_MAX; i++) {
        tparams.handle = apulu_impl_db()->ing_drop_stats_tbl_hdls_[i];
        apulu_impl_db()->ingress_drop_stats_tbl()->remove(&tparams);
    }
    api::impl::pds_impl_state::destroy(&api::impl::g_pds_impl_state);
    p4pd_cleanup();
}

void
apulu_impl::program_config_init(pds_init_params_t *init_params,
                                asic_cfg_t *asic_cfg) {
    asic_cfg->num_pgm_cfgs = 3;
    memset(asic_cfg->pgm_cfg, 0, sizeof(asic_cfg->pgm_cfg));
    asic_cfg->pgm_cfg[0].path = std::string("p4_bin");
    asic_cfg->pgm_cfg[1].path = std::string("rxdma_bin");
    asic_cfg->pgm_cfg[2].path = std::string("txdma_bin");
}

void
apulu_impl::asm_config_init(pds_init_params_t *init_params,
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
apulu_impl::ring_config_init(asic_cfg_t *asic_cfg) {
    asic_cfg->num_rings = 0;
}

sdk_ret_t
apulu_impl::inter_pipe_init_(void) {
    return SDK_RET_OK;
}

sdk_ret_t
apulu_impl::egress_drop_stats_init_(void) {
    sdk_ret_t                    ret;
    sdk_table_api_params_t       tparams;
    p4e_drop_stats_swkey_t       key = { 0 };
    p4e_drop_stats_actiondata_t  data = { 0 };
    p4e_drop_stats_swkey_mask_t  key_mask = { 0 };

    for (uint32_t i = P4E_DROP_REASON_MIN; i <= P4E_DROP_REASON_MAX; i++) {
        key.control_metadata_p4e_drop_reason = ((uint32_t)1 << i);
        key_mask.control_metadata_p4e_drop_reason_mask =
            key.control_metadata_p4e_drop_reason;
        data.action_id = P4E_DROP_STATS_P4E_DROP_STATS_ID;
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &key, &key_mask, &data,
                                       P4E_DROP_STATS_P4E_DROP_STATS_ID,
                                       sdk::table::handle_t::null());
        ret = apulu_impl_db()->egress_drop_stats_tbl()->insert(&tparams);
        apulu_impl_db()->egr_drop_stats_tbl_hdls_[i] = tparams.handle;
        SDK_ASSERT(ret == SDK_RET_OK);
    }
    return ret;
}

sdk_ret_t
apulu_impl::ingress_drop_stats_init_(void) {
    sdk_ret_t                    ret;
    sdk_table_api_params_t       tparams;
    p4i_drop_stats_swkey_t       key = { 0 };
    p4i_drop_stats_actiondata_t  data = { 0 };
    p4i_drop_stats_swkey_mask_t  key_mask = { 0 };

    for (uint32_t i = P4I_DROP_REASON_MIN; i <= P4I_DROP_REASON_MAX; i++) {
        key.control_metadata_p4i_drop_reason = ((uint32_t)1 << i);
        key_mask.control_metadata_p4i_drop_reason_mask =
            key.control_metadata_p4i_drop_reason;
        data.action_id = P4I_DROP_STATS_P4I_DROP_STATS_ID;
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &key, &key_mask, &data,
                                       P4I_DROP_STATS_P4I_DROP_STATS_ID,
                                       sdk::table::handle_t::null());
        ret = apulu_impl_db()->ingress_drop_stats_tbl()->insert(&tparams);
        // TODO: storing handles is not going to work !!
        apulu_impl_db()->ing_drop_stats_tbl_hdls_[i] = tparams.handle;
        SDK_ASSERT(ret == SDK_RET_OK);
    }
    return ret;
}

sdk_ret_t
apulu_impl::stats_init_(void) {
    ingress_drop_stats_init_();
    egress_drop_stats_init_();
    return SDK_RET_OK;
}

sdk_ret_t
apulu_impl::nacl_init_(void) {
    sdk_ret_t ret;
    nacl_swkey_t key;
    p4pd_error_t p4pd_ret;
    nacl_swkey_mask_t mask;
    nacl_actiondata_t data;
    uint32_t idx = PDS_IMPL_NACL_BLOCK_GLOBAL_MIN;

    // drop all IPv6 traffic
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    key.key_metadata_ktype = KEY_TYPE_IPV6;
    mask.key_metadata_ktype_mask = ~0;
    data.action_id = NACL_NACL_DROP_ID;
    p4pd_ret = p4pd_entry_install(P4TBL_ID_NACL, idx++, &key, &mask, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program NACL entry for ipv6 drop");
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

#if 0
    // TODO: we need this for EP aging probes !!!
    // drop all ARP responses seen coming on host lifs
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    key.control_metadata_rx_packet = 0;
    key.key_metadata_ktype = KEY_TYPE_MAC;
    //key.control_metadata_lif_type = P4_LIF_TYPE_HOST;
    key.control_metadata_tunneled_packet = 0;
    key.key_metadata_dport = ETH_TYPE_ARP;
    key.key_metadata_sport = 2;    // ARP response
    mask.control_metadata_rx_packet_mask = ~0;
    mask.key_metadata_ktype_mask = ~0;
    mask.control_metadata_lif_type = ~0;
    mask.control_metadata_tunneled_packet_mask = ~0;
    mask.key_metadata_dport_mask = ~0;
    mask.key_metadata_sport_mask = ~0;
    data.action_id = NACL_NACL_DROP_ID;
    p4pd_ret = p4pd_entry_install(P4TBL_ID_NACL, idx++, &key, &mask, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program drop entry for ARP responses "
                      "on host lifs");
        ret = sdk::SDK_RET_HW_PROGRAM_ERR;
        goto error;
    }
#endif

    // drop all DHCP responses from host lifs to prevent DHCP server spoofing
    // by workloads
    // TODO:
    // 1. address the case where DHCP server is running as workload
    // 2. why not ask user to configure this as infra policy ?
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    key.control_metadata_rx_packet = 0;
    key.key_metadata_ktype = KEY_TYPE_IPV4;
    key.control_metadata_lif_type = P4_LIF_TYPE_HOST;
    key.control_metadata_tunneled_packet = 0;
    key.key_metadata_sport = 67;
    key.key_metadata_proto = 17;    // UDP
    mask.control_metadata_rx_packet_mask = ~0;
    mask.key_metadata_ktype_mask = ~0;
    mask.control_metadata_lif_type_mask = ~0;
    mask.control_metadata_tunneled_packet_mask = ~0;
    mask.key_metadata_sport_mask = ~0;
    mask.key_metadata_proto_mask = ~0;
    data.action_id = NACL_NACL_DROP_ID;
    p4pd_ret = p4pd_entry_install(P4TBL_ID_NACL, idx++, &key, &mask, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program drop entry for DHCP responses on "
                      "host lifs");
        ret = sdk::SDK_RET_HW_PROGRAM_ERR;
        goto error;
    }

    // install a NACL to use nexthop information from the ARM header for packets
    // that are re-injected by vpp or learn thread
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    key.arm_to_p4i_nexthop_valid = 1;
    mask.arm_to_p4i_nexthop_valid_mask = ~0;
    data.action_id = NACL_NACL_REDIRECT_ID;
    p4pd_ret = p4pd_entry_install(P4TBL_ID_NACL, idx++, &key, &mask, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program redirect entry for re-injected pkts "
                      "from s/w datapath");
        ret = sdk::SDK_RET_HW_PROGRAM_ERR;
        goto error;
    }
    // make sure we stayed with in the global entry range in the TCAM table
    SDK_ASSERT(idx <= PDS_IMPL_NACL_BLOCK_LEARN_MIN);
    return SDK_RET_OK;

error:

    return ret;
}

sdk_ret_t
apulu_impl::checksum_init_(void) {
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
apulu_impl::table_init_(void) {
    sdk_ret_t     ret;
    mem_addr_t    addr;

    ret = inter_pipe_init_();
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // initialize checksum table
    ret = checksum_init_();
    SDK_ASSERT(ret == SDK_RET_OK);

    // initialize stats tables
    ret = stats_init_();
    SDK_ASSERT(ret == SDK_RET_OK);

    // install all default NACL entries
    ret = nacl_init_();
    SDK_ASSERT(ret == SDK_RET_OK);

    // program session stats table base address as table constant
    // of session table
    addr = api::g_pds_state.mempartition()->start_addr(
                                        MEM_REGION_SESSION_STATS_NAME);
    SDK_ASSERT(addr != INVALID_MEM_ADDRESS);
    // subtract 2G (saves ASM instructions)
    addr -= ((uint64_t)1 << 31);
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_SESSION, addr);

    return SDK_RET_OK;
}

sdk_ret_t
apulu_impl::p4plus_table_init_(void) {
    p4plus_prog_t prog;
    p4pd_table_properties_t tbl_ctx_apphdr;
    p4pd_table_properties_t tbl_ctx_apphdr_off;
    p4pd_table_properties_t tbl_ctx_txdma_act;
    p4pd_table_properties_t tbl_ctx_txdma_act_ext;

    p4pd_global_table_properties_get(P4_P4PLUS_RXDMA_TBL_ID_COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE,
                                     &tbl_ctx_apphdr);
    memset(&prog, 0, sizeof(prog));
    prog.stageid = tbl_ctx_apphdr.stage;
    prog.stage_tableid = tbl_ctx_apphdr.stage_tableid;
    prog.stage_tableid_off = tbl_ctx_apphdr_off.stage_tableid;
    prog.control = "apulu_rxdma";
    prog.prog_name = "rxdma_stage0.bin";
    prog.pipe = P4_PIPELINE_RXDMA;
    // configure only in hardinit
    if (api::g_pds_state.cold_boot()) {
        sdk::platform::capri::capri_p4plus_table_init(&prog,
                                                      api::g_pds_state.platform_type());
    } else {
        // for upgrade init, save the information and will be applied after p4 quiesce
        table_engine_cfg_backup_(P4_PIPELINE_RXDMA);
    }

    p4pd_global_table_properties_get(P4_P4PLUS_TXDMA_TBL_ID_TX_TABLE_S0_T0,
                                     &tbl_ctx_txdma_act);
    memset(&prog, 0, sizeof(prog));
    prog.stageid = tbl_ctx_txdma_act.stage;
    prog.stage_tableid = tbl_ctx_txdma_act.stage_tableid;
    prog.control = "apulu_txdma";
    prog.prog_name = "txdma_stage0.bin";
    prog.pipe = P4_PIPELINE_TXDMA;
    // configure only in hardinit
    if (api::g_pds_state.cold_boot()) {
        sdk::platform::capri::capri_p4plus_table_init(&prog,
                                                      api::g_pds_state.platform_type());
    } else {
        // for upgrade init, save the information and will be applied after p4 quiesce
        table_engine_cfg_backup_(P4_PIPELINE_TXDMA);
    }

    return SDK_RET_OK;
}

sdk_ret_t
apulu_impl::pipeline_init(void) {
    sdk_ret_t  ret;
    p4pd_cfg_t p4pd_cfg;
    std::string cfg_path = api::g_pds_state.cfg_path();

    p4pd_cfg.cfg_path = cfg_path.c_str();
    ret = pipeline_p4_hbm_init(&p4pd_cfg, api::g_pds_state.cold_boot());
    SDK_ASSERT(ret == SDK_RET_OK);

    // skip the remaining if it is a soft initialization
    if (sdk::asic::is_soft_init()) {
        return SDK_RET_OK;
    }

    ret = sdk::asic::pd::asicpd_p4plus_table_mpu_base_init(&p4pd_cfg);
    SDK_ASSERT(ret == SDK_RET_OK);
    // rss config is not modified across upgrades
    // TODO : confirm this aproach is correct
    if (api::g_pds_state.cold_boot()) {
        ret = sdk::asic::pd::asicpd_toeplitz_init("apulu_rxdma",
                             P4_P4PLUS_RXDMA_TBL_ID_ETH_RX_RSS_INDIR);
    }
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = p4plus_table_init_();
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = sdk::asic::pd::asicpd_table_mpu_base_init(&p4pd_cfg);
    SDK_ASSERT(ret == SDK_RET_OK);
    // on upgrade boot, this will be done during switchover
    if (api::g_pds_state.cold_boot()) {
        ret = sdk::asic::pd::asicpd_program_table_mpu_pc();
        SDK_ASSERT(ret == SDK_RET_OK);
        ret = sdk::asic::pd::asicpd_deparser_init();
        SDK_ASSERT(ret == SDK_RET_OK);
    }

    g_pds_impl_state.init(&api::g_pds_state);
    api::g_pds_state.lif_db()->impl_state_set(g_pds_impl_state.lif_impl_db());

    // it can happen that service lif is modified during upgrade. in that case
    // just initialize the new service lif
    //
    // during A to B upgrade and the service lif id of A(g_upg_state->service_lif_id)
    // matches with B(APULU_SERVICE_LIF), verify the configs are perfectly matching
    if (api::g_pds_state.cold_boot() ||
        (api::g_upg_state->service_lif_id() != APULU_SERVICE_LIF)) {
        ret = init_service_lif(APULU_SERVICE_LIF, p4pd_cfg.cfg_path);
    } else {
        // on upgrade boot, verify the config
        ret = service_lif_upg_verify(APULU_SERVICE_LIF, p4pd_cfg.cfg_path);
    }
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = table_init_();
    SDK_ASSERT(ret == SDK_RET_OK);

    return SDK_RET_OK;
}


// this re-writes the common registers in the pipeline during A to B upgrade.
// should be called in quiesced state and it should be the final
// stage of the upgrade steps
// if this returns OK, pipeline is switched to B from A.
// if there is an error, rollback should rewrites these to A. so old
// configuration should be saved by A impl(see upg_backup)
// keep this code very minimum to reduce the traffic hit duration.
// avoid TRACES.
sdk_ret_t
apulu_impl::upg_switchover(void) {
    sdk_ret_t ret;
    p4pd_pipeline_t pipe[] = { P4_PIPELINE_INGRESS, P4_PIPELINE_EGRESS,
                               P4_PIPELINE_RXDMA, P4_PIPELINE_TXDMA };
    p4_tbl_eng_cfg_t *cfg;
    uint32_t num_cfgs, max_cfgs;
    std::list<qstate_cfg_t> q;

    // return error if the pipeline is not quiesced
    if (!sdk::asic::is_quiesced()) {
        return SDK_RET_ERR;
    }
    // program generated nic/pgm_bin (generated configs)
    sdk::platform::capri::capri_pgm_init();

    // update the table engine configs
    for (uint32_t i = 0; i < sizeof(pipe)/sizeof(uint32_t); i++) {
        num_cfgs = api::g_upg_state->tbl_eng_cfg(pipe[i], &cfg, &max_cfgs);
        ret = sdk::asic::pd::asicpd_tbl_eng_cfg_modify(pipe[i], cfg, num_cfgs);
        if (ret != SDK_RET_OK) {
            return ret;
        }
    }
    ret = sdk::asic::pd::asicpd_deparser_init();
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // TODO: DMA sram/tcam shadow copy to memory

    // update pc offsets for qstate
    q = api::g_upg_state->qstate_cfg();
    for (std::list<qstate_cfg_t>::iterator it=q.begin();
         it != q.end(); ++it) {
        uint8_t pgm_off = it->pgm_off;
        if (sdk::lib::pal_mem_write(it->addr, &pgm_off, 1) != 0) {
            return SDK_RET_ERR;
        }
        PAL_barrier();
        p4plus_invalidate_cache(it->addr, it->size, P4PLUS_CACHE_INVALIDATE_BOTH);
    }

    // update rss config
    api::g_upg_state->tbl_eng_rss_cfg(&cfg);
    sdk::asic::pd::asicpd_rss_tbl_eng_cfg_modify(cfg);
    return SDK_RET_OK;
}

// backup all the existing configs which will be modified during switchover.
// during A to B upgrade, this will be invoked by A
// if there is a switchover failue from A to B, during rollbacking, this backed up config
// will be applied as B might have overwritten some of these configs
// pipeline switchover and pipeline backup should be in sync
// sequence : A(backup) -> upgrade -> A2B(switchover) ->
//          : B(switchover_failure) -> rollback -> B2A(switchover) -> success/failure
// B2A(switchover_failure) cannot be recovered
sdk_ret_t
apulu_impl::upg_backup(void) {
    p4pd_pipeline_t pipe[] = { P4_PIPELINE_INGRESS, P4_PIPELINE_EGRESS,
                               P4_PIPELINE_RXDMA, P4_PIPELINE_TXDMA };
    p4_tbl_eng_cfg_t *cfg;
    p4plus_prog_t prog;
    uint32_t rss_tblid = P4_P4PLUS_RXDMA_TBL_ID_ETH_RX_RSS_INDIR;

    // backup table engine config
    for (uint32_t i = 0; i < sizeof(pipe)/sizeof(uint32_t); i++) {
        table_engine_cfg_backup_(pipe[i]);
    }
    // backup rss table engine config
    api::g_upg_state->tbl_eng_rss_cfg(&cfg);
    sdk::asic::pd::asicpd_rss_tbl_eng_cfg_get("apulu_rxdma", rss_tblid, cfg);

    // TODO : backup pc offsets for qstate
    return SDK_RET_OK;
}

sdk_ret_t
apulu_impl::write_to_rxdma_table(mem_addr_t addr, uint32_t tableid,
                                  uint8_t action_id, void *actiondata) {
    uint32_t     len;
    uint8_t      packed_bytes[CACHE_LINE_SIZE];
    uint8_t      *packed_entry = packed_bytes;

    if (p4pd_rxdma_get_max_action_id(tableid) > 1) {
        struct line_s {
            uint8_t action_pc;
            uint8_t packed_entry[CACHE_LINE_SIZE - sizeof(action_pc)];
        };
        auto line = (struct line_s *)packed_bytes;
        line->action_pc = sdk::asic::pd::asicpd_get_action_pc(tableid,
                                                              action_id);
        packed_entry = line->packed_entry;
    }
    p4pd_p4plus_rxdma_raw_table_hwentry_query(tableid, action_id, &len);
    p4pd_p4plus_rxdma_entry_pack(tableid, action_id, actiondata, packed_entry);
    return asic_mem_write(addr, packed_bytes, 1 + (len >> 3),
                          ASIC_WRITE_MODE_WRITE_THRU);
}

sdk_ret_t
apulu_impl::write_to_txdma_table(mem_addr_t addr, uint32_t tableid,
                                  uint8_t action_id, void *actiondata) {
    uint32_t     len;
    uint8_t      packed_bytes[CACHE_LINE_SIZE];
    uint8_t      *packed_entry = packed_bytes;

    if (p4pd_txdma_get_max_action_id(tableid) > 1) {
        struct line_s {
            uint8_t action_pc;
            uint8_t packed_entry[CACHE_LINE_SIZE - sizeof(action_pc)];
        };
        auto line = (struct line_s *) packed_bytes;
        line->action_pc = sdk::asic::pd::asicpd_get_action_pc(tableid,
                                                              action_id);
        packed_entry = line->packed_entry;
    }
    p4pd_p4plus_txdma_raw_table_hwentry_query(tableid, action_id, &len);
    p4pd_p4plus_txdma_entry_pack(tableid, action_id, actiondata, packed_entry);
    return asic_mem_write(addr, packed_bytes, 1 + (len >> 3),
                          ASIC_WRITE_MODE_WRITE_THRU);
}

sdk_ret_t
apulu_impl::transaction_begin(void) {
    vpc_impl_db()->table_transaction_begin();
    tep_impl_db()->table_transaction_begin();
    vnic_impl_db()->table_transaction_begin();
    mapping_impl_db()->table_transaction_begin();
    route_table_impl_db()->table_transaction_begin();
    security_policy_impl_db()->table_transaction_begin();
    svc_mapping_impl_db()->table_transaction_begin();
    return SDK_RET_OK;
}

sdk_ret_t
apulu_impl::transaction_end(void) {
    vpc_impl_db()->table_transaction_end();
    tep_impl_db()->table_transaction_end();
    vnic_impl_db()->table_transaction_end();
    mapping_impl_db()->table_transaction_end();
    route_table_impl_db()->table_transaction_end();
    security_policy_impl_db()->table_transaction_end();
    svc_mapping_impl_db()->table_transaction_end();
    return SDK_RET_OK;
}

sdk_ret_t
apulu_impl::table_stats(debug::table_stats_get_cb_t cb, void *ctxt) {
    mapping_impl_db()->table_stats(cb, ctxt);
    svc_mapping_impl_db()->table_stats(cb, ctxt);
    apulu_impl_db()->table_stats(cb, ctxt);
    return SDK_RET_OK;
}

sdk_ret_t
apulu_impl::session_stats(debug::session_stats_get_cb_t cb, uint32_t lowidx,
                           uint32_t highidx, void *ctxt) {
    sdk_ret_t ret;
    uint64_t offset = 0;
    uint64_t start_addr = 0;
    pds_session_debug_stats_t session_stats_entry;

    memset(&session_stats_entry, 0, sizeof(pds_session_debug_stats_t));
    start_addr =
        api::g_pds_state.mempartition()->start_addr(MEM_REGION_SESSION_STATS_NAME);

    for (uint32_t idx = lowidx; idx <= highidx; idx ++) {
        // TODO: usage of sizeof(pds_session_debug_stats_t) is incorrect here,
        //       we need to use a pipeline specific data structure here ...
        //       ideally, the o/p of p4pd_global_table_properties_get() will
        //       have entry size
        offset = idx * sizeof(pds_session_debug_stats_t);
        ret = sdk::asic::asic_mem_read(start_addr + offset,
                                       (uint8_t *)&session_stats_entry,
                                       sizeof(pds_session_debug_stats_t));
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to read session stats for index %u err %u",
                          idx, ret);
            return ret;
        }
        cb(idx, &session_stats_entry, ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
apulu_impl::session(debug::session_get_cb_t cb, void *ctxt) {
    return SDK_RET_OK;
}

sdk_ret_t
apulu_impl::flow(debug::flow_get_cb_t cb, void *ctxt) {
    uint32_t idx = 0;
    p4pd_error_t p4pd_ret;
    p4pd_table_properties_t prop;
    ipv4_entry_t ipv4_entry;
    ipv6_entry_t ipv6_entry;

    // read IPv4 sessions
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

    // read IPv6 sessions
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
apulu_impl::impl_state_slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
    return g_pds_impl_state.slab_walk(walk_cb, ctxt);
}

sdk_ret_t
apulu_impl::session_clear(uint32_t idx) {
    return SDK_RET_OK;
}

sdk_ret_t
apulu_impl::handle_cmd(cmd_ctxt_t *ctxt) {
    switch (ctxt->cmd) {
    case CLI_CMD_MAPPING_DUMP:
        g_pds_impl_state.mapping_impl_db()->mapping_dump(ctxt->fd,
                    (ctxt->args.valid == true) ? &ctxt->args : NULL);
        break;
    default:
        return SDK_RET_INVALID_ARG;
    }

    return SDK_RET_OK;
}

#define lif_action              action_u.lif_lif_info
sdk_ret_t
program_lif_table (uint16_t lif_hw_id, uint8_t lif_type, uint16_t vpc_hw_id,
                   uint16_t bd_hw_id, uint16_t vnic_hw_id, mac_addr_t vr_mac,
                   bool learn_en)
{
    sdk_ret_t ret;
    p4pd_error_t p4pd_ret;
    lif_actiondata_t lif_data = { 0 };

    PDS_TRACE_DEBUG("Programming LIF table at idx %u, vpc hw id %u, "
                    "bd hw id %u, vnic hw id %u", lif_hw_id, vpc_hw_id,
                    bd_hw_id, vnic_hw_id, macaddr2str(vr_mac));

    // program the LIF table
    lif_data.action_id = LIF_LIF_INFO_ID;
    lif_data.lif_action.direction = P4_LIF_DIR_HOST;
    lif_data.lif_action.lif_type = lif_type;
    lif_data.lif_action.vnic_id = vnic_hw_id;
    lif_data.lif_action.bd_id = bd_hw_id;
    lif_data.lif_action.vpc_id = vpc_hw_id;
    sdk::lib::memrev(lif_data.lif_action.vrmac, vr_mac, ETH_ADDR_LEN);
    lif_data.lif_action.learn_enabled = learn_en ? TRUE : FALSE;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_LIF, lif_hw_id,
                                       NULL, NULL, &lif_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program LIF table for lif %u", lif_hw_id);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_LIF2, lif_hw_id,
                                       NULL, NULL, &lif_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program LIF2 table for lif %u", lif_hw_id);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api

//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// apollo pipeline implementation
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
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/impl/apollo/apollo_impl.hpp"
#include "nic/apollo/api/impl/apollo/pds_impl_state.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/p4/include/defines.h"
#include "gen/p4gen/apollo/include/p4pd.h"
#include "gen/p4gen/apollo_rxdma/include/apollo_rxdma_p4pd.h"
#include "gen/p4gen/apollo_txdma/include/apollo_txdma_p4pd.h"

extern int p4pd_txdma_get_max_action_id(uint32_t tableid);
extern sdk_ret_t init_service_lif(const char *cfg_path);

#define MEM_REGION_RXDMA_PROGRAM_NAME "rxdma_program"
#define MEM_REGION_TXDMA_PROGRAM_NAME "txdma_program"
#define MEM_REGION_LIF_STATS_BASE     "lif_stats_base"
#define RXDMA_SYMBOLS_MAX             1
#define TXDMA_SYMBOLS_MAX             1
#define MEM_REGION_SESSION_STATS_NAME "session_stats"

namespace api {
namespace impl {

/// \defgroup PDS_PIPELINE_IMPL - pipeline wrapper implementation
/// \ingroup PDS_PIPELINE
/// @{

/// \brief helper class to sort p4/p4+ programs to maximize performance
class sort_mpu_programs_compare {
public:
    bool operator() (std::string p1, std::string p2) {
        std::map <std::string, p4pd_table_properties_t>::iterator it1, it2;

        it1 = tbl_map_.find(p1);
        it2 = tbl_map_.find(p2);
        if ((it1 == tbl_map_.end()) || (it2 == tbl_map_.end())) {
            return (p1 < p2);
        }
        p4pd_table_properties_t tbl_ctx1 = it1->second;
        p4pd_table_properties_t tbl_ctx2 = it2->second;
        if (tbl_ctx1.gress != tbl_ctx2.gress) {
            return (tbl_ctx1.gress < tbl_ctx2.gress);
        }
        if (tbl_ctx1.stage != tbl_ctx2.stage) {
            return (tbl_ctx1.stage < tbl_ctx2.stage);
        }
        return (tbl_ctx1.stage_tableid < tbl_ctx2.stage_tableid);
    }

    void add_table(std::string tbl_name, p4pd_table_properties_t tbl_ctx) {
        std::pair <std::string, p4pd_table_properties_t> key_value;
        key_value = std::make_pair(tbl_name.append(".bin"), tbl_ctx);
        tbl_map_.insert(key_value);
    }

private:
    std::map <std::string, p4pd_table_properties_t> tbl_map_;
};

void
apollo_impl::sort_mpu_programs_(std::vector<std::string>& programs) {
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
apollo_impl::rxdma_symbols_init_(void **p4plus_symbols,
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
    SDK_ASSERT(i <= RXDMA_SYMBOLS_MAX);

    return i;
}

uint32_t
apollo_impl::txdma_symbols_init_(void **p4plus_symbols,
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
    SDK_ASSERT(i <= TXDMA_SYMBOLS_MAX);

    return i;
}

sdk_ret_t
apollo_impl::init_(pipeline_cfg_t *pipeline_cfg) {
    pipeline_cfg_ = *pipeline_cfg;
    return SDK_RET_OK;
}

apollo_impl *
apollo_impl::factory(pipeline_cfg_t *pipeline_cfg) {
    apollo_impl    *impl;

    impl = (apollo_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_PIPELINE_IMPL,
                                    sizeof(apollo_impl));
    new (impl) apollo_impl();
    if (impl->init_(pipeline_cfg) != SDK_RET_OK) {
        impl->~apollo_impl();
        SDK_FREE(SDK_MEM_ALLOC_PDS_PIPELINE_IMPL, impl);
        return NULL;
    }
    return impl;
}

void
apollo_impl::destroy(apollo_impl *impl) {
    int i;

    // Remove key native table entries
    for (i = 0; i < MAX_KEY_NATIVE_TBL_ENTRIES; i++) {
        apollo_impl_db()->key_native_tbl()->remove(
            apollo_impl_db()->key_native_tbl_idx_[i]);
    }
    // Remove key tunneled table entries
    for (i = 0; i < MAX_KEY_TUNNELED_TBL_ENTRIES; i++) {
        apollo_impl_db()->key_tunneled_tbl()->remove(
            apollo_impl_db()->key_tunneled_tbl_idx_[i]);
    }
    // Remove drop stats table entries
    for (i = P4E_DROP_REASON_MIN; i <= P4E_DROP_REASON_MAX; i++) {
        apollo_impl_db()->egress_drop_stats_tbl()->remove(i);
    }
    for (i = P4I_DROP_REASON_MIN; i <= P4I_DROP_REASON_MAX; i++) {
        apollo_impl_db()->ingress_drop_stats_tbl()->remove(i);
    }
    api::impl::pds_impl_state::destroy(&api::impl::g_pds_impl_state);
    p4pd_cleanup();
}

void
apollo_impl::program_config_init(pds_init_params_t *init_params,
                                 asic_cfg_t *asic_cfg) {
    asic_cfg->num_pgm_cfgs = 3;
    memset(asic_cfg->pgm_cfg, 0, sizeof(asic_cfg->pgm_cfg));
    asic_cfg->pgm_cfg[0].path = std::string("p4_bin");
    asic_cfg->pgm_cfg[1].path = std::string("rxdma_bin");
    asic_cfg->pgm_cfg[2].path = std::string("txdma_bin");
}

void
apollo_impl::asm_config_init(pds_init_params_t *init_params,
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

sdk_ret_t
apollo_impl::key_native_init_(void) {
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
    data.action_id = KEY_NATIVE_NATIVE_IPV4_PACKET_ID;
    mask.ipv4_1_valid_mask = 0xFF;
    mask.ipv6_1_valid_mask = 0xFF;
    mask.ethernet_2_valid_mask = 0xFF;
    mask.ipv4_2_valid_mask = 0xFF;
    mask.ipv6_2_valid_mask = 0xFF;
    ret = apollo_impl_db()->key_native_tbl()->insert(
        &key, &mask, &data, &apollo_impl_db()->key_native_tbl_idx_[idx++]);

    // entry for native IPv6 packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    key.ipv4_1_valid = 0;
    key.ipv6_1_valid = 1;
    key.ethernet_2_valid = 0;
    key.ipv4_2_valid = 0;
    key.ipv6_2_valid = 0;
    data.action_id = KEY_NATIVE_NATIVE_IPV6_PACKET_ID;
    mask.ipv4_1_valid_mask = 0xFF;
    mask.ipv6_1_valid_mask = 0xFF;
    mask.ethernet_2_valid_mask = 0xFF;
    mask.ipv4_2_valid_mask = 0xFF;
    mask.ipv6_2_valid_mask = 0xFF;
    ret = apollo_impl_db()->key_native_tbl()->insert(
        &key, &mask, &data, &apollo_impl_db()->key_native_tbl_idx_[idx++]);

    // entry for native non-IP packets
    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));
    key.ipv4_1_valid = 0;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 0;
    key.ipv4_2_valid = 0;
    key.ipv6_2_valid = 0;
    data.action_id = KEY_NATIVE_NATIVE_NONIP_PACKET_ID;
    mask.ipv4_1_valid_mask = 0xFF;
    mask.ipv6_1_valid_mask = 0xFF;
    mask.ethernet_2_valid_mask = 0xFF;
    mask.ipv4_2_valid_mask = 0xFF;
    mask.ipv6_2_valid_mask = 0xFF;
    ret = apollo_impl_db()->key_native_tbl()->insert(
        &key, &mask, &data, &apollo_impl_db()->key_native_tbl_idx_[idx++]);

    // check overflow
    SDK_ASSERT(idx <= MAX_KEY_NATIVE_TBL_ENTRIES);
    return ret;
}

sdk_ret_t
apollo_impl::key_tunneled_init_(void) {
    sdk_ret_t                    ret;
    uint32_t                     idx = 0;
    key_tunneled_swkey_t         key;
    key_tunneled_swkey_mask_t    mask;
    key_tunneled_actiondata_t    data;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0xFF, sizeof(mask));
    memset(&data, 0, sizeof(data));

    // entry for tunneled (inner) IPv4 packets
    key.ipv4_1_valid = 1;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 0;
    key.ipv4_2_valid = 1;
    key.ipv6_2_valid = 0;
    data.action_id = KEY_TUNNELED_TUNNELED_IPV4_PACKET_ID;
    mask.ipv4_1_valid_mask = 0xFF;
    mask.ipv6_1_valid_mask = 0xFF;
    mask.ethernet_2_valid_mask = 0x0;
    mask.ipv4_2_valid_mask = 0xFF;
    mask.ipv6_2_valid_mask = 0xFF;
    ret = apollo_impl_db()->key_tunneled_tbl()->insert(
        &key, &mask, &data, &apollo_impl_db()->key_tunneled_tbl_idx_[idx++]);
    // entry for tunneled (inner) IPv6 packets
    key.ipv4_1_valid = 1;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 0;
    key.ipv4_2_valid = 0;
    key.ipv6_2_valid = 1;
    data.action_id = KEY_TUNNELED_TUNNELED_IPV6_PACKET_ID;
    mask.ipv4_1_valid_mask = 0xFF;
    mask.ipv6_1_valid_mask = 0xFF;
    mask.ethernet_2_valid_mask = 0x0;
    mask.ipv4_2_valid_mask = 0xFF;
    mask.ipv6_2_valid_mask = 0xFF;
    ret = apollo_impl_db()->key_tunneled_tbl()->insert(
        &key, &mask, &data, &apollo_impl_db()->key_tunneled_tbl_idx_[idx++]);

    // entry for tunneled (inner) non-IP packets
    key.ipv4_1_valid = 1;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 1;
    key.ipv4_2_valid = 0;
    key.ipv6_2_valid = 0;
    data.action_id = KEY_TUNNELED_TUNNELED_NONIP_PACKET_ID;
    mask.ipv4_1_valid_mask = 0xFF;
    mask.ipv6_1_valid_mask = 0xFF;
    mask.ethernet_2_valid_mask = 0xFF;
    mask.ipv4_2_valid_mask = 0xFF;
    mask.ipv6_2_valid_mask = 0xFF;
    ret = apollo_impl_db()->key_tunneled_tbl()->insert(
        &key, &mask, &data, &apollo_impl_db()->key_tunneled_tbl_idx_[idx++]);

    // check max
    SDK_ASSERT(idx <= MAX_KEY_TUNNELED_TBL_ENTRIES);
    return ret;
}

sdk_ret_t
apollo_impl::ingress_to_rxdma_init_(void) {
    p4pd_error_t p4pd_ret;
    ingress_to_rxdma_actiondata_t data = { 0 };

    data.action_id = INGRESS_TO_RXDMA_CLASSIC_NIC_APP_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_INGRESS_TO_RXDMA,
                                       P4PLUS_APPTYPE_CLASSIC_NIC,
                                       NULL, NULL, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    data.action_id = INGRESS_TO_RXDMA_REDIRECT_TO_ARM_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_INGRESS_TO_RXDMA,
                                       P4PLUS_APPTYPE_CPU,
                                       NULL, NULL, &data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    return SDK_RET_OK;
}

sdk_ret_t
apollo_impl::egress_drop_stats_init_(void) {
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
            apollo_impl_db()->egress_drop_stats_tbl()->insert_withid(&key,
                                                                     &key_mask,
                                                                     &data, i);
        SDK_ASSERT(ret == SDK_RET_OK);
    }
    return ret;
}

sdk_ret_t
apollo_impl::ingress_drop_stats_init_(void) {
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
            apollo_impl_db()->ingress_drop_stats_tbl()->insert_withid(&key,
                                                                     &key_mask,
                                                                     &data, i);
        SDK_ASSERT(ret == SDK_RET_OK);
    }
    return ret;
}

sdk_ret_t
apollo_impl::stats_init_(void) {
    ingress_drop_stats_init_();
    egress_drop_stats_init_();
    return SDK_RET_OK;
}

sdk_ret_t
apollo_impl::table_init_(void) {
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
    ret = ingress_to_rxdma_init_();
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // program session stats table base address as table constant
    // of session table
    addr = api::g_pds_state.mempartition()->start_addr(
                                        MEM_REGION_SESSION_STATS_NAME);
    SDK_ASSERT(addr != INVALID_MEM_ADDRESS);
    // reset bit 31 (saves one asm instruction)
    addr &= ~((uint64_t)1 << 31);
    sdk::asic::pd::asicpd_program_table_constant(P4TBL_ID_SESSION, addr);

    return SDK_RET_OK;
}

sdk_ret_t
apollo_impl::p4plus_table_init_(void) {
    p4pd_table_properties_t tbl_ctx_apphdr;
    p4pd_table_properties_t tbl_ctx_apphdr_off;
    p4pd_table_properties_t tbl_ctx_txdma_act;
    p4pd_table_properties_t tbl_ctx_txdma_act_ext;
    p4plus_prog_t prog;


    p4pd_global_table_properties_get(P4_APOLLO_RXDMA_TBL_ID_COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE,
                                     &tbl_ctx_apphdr);
    memset(&prog, 0, sizeof(prog));
    prog.stageid = tbl_ctx_apphdr.stage;
    prog.stage_tableid = tbl_ctx_apphdr.stage_tableid;
    prog.stage_tableid_off = tbl_ctx_apphdr_off.stage_tableid;
    prog.control = "apollo_rxdma";
    prog.prog_name = "rxdma_stage0.bin";
    prog.pipe = P4_PIPELINE_RXDMA;
    sdk::platform::capri::capri_p4plus_table_init(&prog, api::g_pds_state.platform_type());

    p4pd_global_table_properties_get(P4_APOLLO_TXDMA_TBL_ID_TX_TABLE_S0_T0,
                                     &tbl_ctx_txdma_act);
    memset(&prog, 0, sizeof(prog));
    prog.stageid = tbl_ctx_txdma_act.stage;
    prog.stage_tableid = tbl_ctx_txdma_act.stage_tableid;
    prog.control = "apollo_txdma";
    prog.prog_name = "txdma_stage0.bin";
    prog.pipe = P4_PIPELINE_TXDMA;
    sdk::platform::capri::capri_p4plus_table_init(&prog, api::g_pds_state.platform_type());

    return SDK_RET_OK;
}

sdk_ret_t
apollo_impl::pipeline_init(void) {
    p4pd_error_t    p4pd_ret;
    sdk_ret_t       ret;
    p4pd_cfg_t p4pd_cfg = {
        .table_map_cfg_file  = "apollo/capri_p4_table_map.json",
        .p4pd_pgm_name       = "apollo_p4",
        .p4pd_rxdma_pgm_name = "apollo_rxdma",
        .p4pd_txdma_pgm_name = "apollo_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };
    p4pd_cfg_t p4pd_rxdma_cfg = {
        .table_map_cfg_file  = "apollo/capri_rxdma_table_map.json",
        .p4pd_pgm_name       = "apollo_p4",
        .p4pd_rxdma_pgm_name = "apollo_rxdma",
        .p4pd_txdma_pgm_name = "apollo_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };
    p4pd_cfg_t p4pd_txdma_cfg = {
        .table_map_cfg_file  = "apollo/capri_txdma_table_map.json",
        .p4pd_pgm_name       = "apollo_p4",
        .p4pd_rxdma_pgm_name = "apollo_rxdma",
        .p4pd_txdma_pgm_name = "apollo_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };

    p4pd_ret = p4pd_init(&p4pd_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);
    p4pd_ret = p4pluspd_rxdma_init(&p4pd_rxdma_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);
    p4pd_ret = p4pluspd_txdma_init(&p4pd_txdma_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    ret = sdk::asic::pd::asicpd_program_hbm_table_base_addr();
    SDK_ASSERT(ret == SDK_RET_OK);

    // Skip the remaining if it is a slave initialization
    if (sdk::asic::is_slave_init()) {
        return SDK_RET_OK;
    }

    ret = sdk::asic::pd::asicpd_p4plus_table_mpu_base_init(&p4pd_cfg);
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = sdk::asic::pd::asicpd_toeplitz_init("apollo_rxdma",
                             P4_APOLLO_RXDMA_TBL_ID_ETH_RX_RSS_INDIR);
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

    ret = init_service_lif(p4pd_cfg.cfg_path);
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = table_init_();
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = stats_init_();
    SDK_ASSERT(ret == SDK_RET_OK);

    return SDK_RET_OK;
}

void
apollo_impl::dump_egress_drop_stats_(FILE *fp) {
}

void
apollo_impl::dump_ingress_drop_stats_(FILE *fp) {
    sdk_ret_t                      ret;
    uint64_t                       pkts;
    tcam                           *table;
    p4i_drop_stats_swkey_t         key = { 0 };
    p4i_drop_stats_swkey_mask_t    key_mask = { 0 };
    p4i_drop_stats_actiondata_t    data = { 0 };

    table = apollo_impl_db()->ingress_drop_stats_tbl();
    for (uint32_t i = P4I_DROP_REASON_MIN; i <= P4I_DROP_REASON_MAX; i++) {
        ret = table->retrieve_from_hw(i, &key, &key_mask, &data);
        if (ret == SDK_RET_OK) {
            memcpy(&pkts,
                   data.action_u.p4i_drop_stats_p4i_drop_stats.drop_stats_pkts,
                   sizeof(data.action_u.p4i_drop_stats_p4i_drop_stats.drop_stats_pkts));
            fprintf(fp,
                    "    drop reason : 0x%x, drop mask : 0x%x, pkts : %lu\n",
                    key.control_metadata_p4i_drop_reason,
                    key_mask.control_metadata_p4i_drop_reason_mask, pkts);
        }
    }
    fprintf(fp, "\n");
}

void
apollo_impl::debug_dump(FILE *fp) {
    fprintf(fp, "Ingress drop statistics\n");
    dump_ingress_drop_stats_(fp);
    fprintf(fp, "Egress drop statistics\n");
    dump_egress_drop_stats_(fp);
}

sdk_ret_t
apollo_impl::write_to_rxdma_table(mem_addr_t addr, uint32_t tableid,
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
        line->action_pc = sdk::asic::pd::asicpd_get_action_pc(tableid, action_id);
        packed_entry = line->packed_entry;
    }

    p4pd_apollo_rxdma_raw_table_hwentry_query(tableid, action_id, &len);
    p4pd_apollo_rxdma_entry_pack(tableid, action_id, actiondata, packed_entry);
    return asic_mem_write(addr, packed_bytes, 1 + (len >> 3),
                          ASIC_WRITE_MODE_WRITE_THRU);
}

sdk_ret_t
apollo_impl::write_to_txdma_table(mem_addr_t addr, uint32_t tableid,
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
        line->action_pc = sdk::asic::pd::asicpd_get_action_pc(tableid, action_id);
        packed_entry = line->packed_entry;
    }

    p4pd_apollo_txdma_raw_table_hwentry_query(tableid, action_id, &len);
    p4pd_apollo_txdma_entry_pack(tableid, action_id, actiondata, packed_entry);
    return asic_mem_write(addr, packed_bytes, 1 + (len >> 3),
                          ASIC_WRITE_MODE_WRITE_THRU);
}

sdk_ret_t
apollo_impl::table_transaction_begin(void) {
    tep_impl_db()->table_transaction_begin();
    vnic_impl_db()->table_transaction_begin();
    mapping_impl_db()->table_transaction_begin();
    route_table_impl_db()->table_transaction_begin();
    security_policy_impl_db()->table_transaction_begin();
    return SDK_RET_OK;
}

sdk_ret_t
apollo_impl::table_transaction_end(void) {
    tep_impl_db()->table_transaction_end();
    vnic_impl_db()->table_transaction_end();
    mapping_impl_db()->table_transaction_end();
    route_table_impl_db()->table_transaction_end();
    security_policy_impl_db()->table_transaction_end();
    return SDK_RET_OK;
}

sdk_ret_t
apollo_impl::table_stats(debug::table_stats_get_cb_t cb, void *ctxt) {
    mapping_impl_db()->table_stats(cb, ctxt);
    return SDK_RET_OK;
}

sdk_ret_t
apollo_impl::session_stats(debug::session_stats_get_cb_t cb, uint32_t lowidx,
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

/// \@}

}    // namespace impl
}    // namespace api

/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    apollo_impl.cc
 *
 * @brief   CAPRI pipeline implementation
 */

#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/api/impl/apollo_impl.hpp"
#include "nic/apollo/api/impl/oci_impl_state.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"
#include "nic/apollo/p4/include/defines.h"

extern sdk_ret_t init_service_lif(void);

#define JP4_PRGM        "p4_program"
#define JRXDMA_PRGM     "rxdma_program"
#define JTXDMA_PRGM     "txdma_program"

namespace impl {

/**
 * @defgroup OCI_PIPELINE_IMPL - pipeline wrapper implementation
 * @ingroup OCI_PIPELINE
 * @{
 */

/**
 * @brief    helper class to sort p4/p4+ programs to maximize performance
 */
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

/**
 * @brief    apollo specific mpu program sort function
 * @param[in] program information
 */
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

/**
 * @brief    initialize an instance of apollo impl class
 * @param[in] pipeline_cfg    pipeline information
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
apollo_impl::init_(pipeline_cfg_t *pipeline_cfg) {
    pipeline_cfg_ = *pipeline_cfg;
    return SDK_RET_OK;
}

/**
 * @brief    factory method to pipeline impl instance
 * @param[in] pipeline_cfg    pipeline information
 * @return    new instance of apollo pipeline impl or NULL, in case of error
 */
apollo_impl *
apollo_impl::factory(pipeline_cfg_t *pipeline_cfg) {
    apollo_impl    *impl;

    impl = (apollo_impl *)SDK_CALLOC(SDK_MEM_ALLOC_OCI_PIPELINE_IMPL,
                                    sizeof(apollo_impl));
    new (impl) apollo_impl();
    if (impl->init_(pipeline_cfg) != SDK_RET_OK) {
        impl->~apollo_impl();
        SDK_FREE(SDK_MEM_ALLOC_OCI_PIPELINE_IMPL, impl);
        return NULL;
    }
    return impl;
}

/**
 * @brief    initialize program configuration
 * @param[in] init_params    initialization time parameters passed by app
 * @param[in] asic_cfg       asic configuration to be populated
 */
void
apollo_impl::program_config_init(oci_init_params_t *init_params,
                                 asic_cfg_t *asic_cfg) {
    asic_cfg->num_pgm_cfgs = 3;
    memset(asic_cfg->pgm_cfg, 0, sizeof(asic_cfg->pgm_cfg));
    asic_cfg->pgm_cfg[0].path = std::string("p4_bin");
    asic_cfg->pgm_cfg[1].path = std::string("rxdma_bin");
    asic_cfg->pgm_cfg[2].path = std::string("txdma_bin");
}

/**
 * @brief    initialize asm configuration
 * @param[in] init_params    initialization time parameters passed by app
 * @param[in] asic_cfg       asic configuration to be populated
 */
void
apollo_impl::asm_config_init(oci_init_params_t *init_params,
                             asic_cfg_t *asic_cfg) {
    asic_cfg->num_asm_cfgs = 3;
    memset(asic_cfg->asm_cfg, 0, sizeof(asic_cfg->asm_cfg));
    asic_cfg->asm_cfg[0].name = init_params->pipeline + "_p4";
    asic_cfg->asm_cfg[0].path = std::string("p4_asm");
    asic_cfg->asm_cfg[0].base_addr = std::string(JP4_PRGM);

    asic_cfg->asm_cfg[0].sort_func = sort_mpu_programs_;
    asic_cfg->asm_cfg[1].name = init_params->pipeline + "_rxdma";
    asic_cfg->asm_cfg[1].path = std::string("rxdma_asm");
    asic_cfg->asm_cfg[1].base_addr = std::string(JRXDMA_PRGM);
    asic_cfg->asm_cfg[2].name = init_params->pipeline + "_txdma";
    asic_cfg->asm_cfg[2].path = std::string("txdma_asm");
    asic_cfg->asm_cfg[2].base_addr = std::string(JTXDMA_PRGM);
}

/**
 * @brief    init routine to initialize key native table
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
apollo_impl::key_native_init_(void) {
    sdk_ret_t                  ret;
    uint32_t                   idx;
    key_native_swkey_t         key;
    key_native_swkey_mask_t    mask;
    key_native_actiondata_t    data;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

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

    ret = apollo_impl_db()->key_native_tbl()->insert(&key, &mask,
                                                     &data, (uint32_t *)&idx);
    return ret;
}

/**
 * @brief    init routine to initialize key tunnel table
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
apollo_impl::key_tunneled_init_(void) {
    sdk_ret_t                    ret;
    uint32_t                     idx;
    key_tunneled_swkey_t         key;
    key_tunneled_swkey_mask_t    mask;
    key_tunneled_actiondata_t    data;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0xFF, sizeof(mask));
    memset(&data, 0, sizeof(data));

    key.ipv4_1_valid = 0;
    key.ipv6_1_valid = 0;
    key.ethernet_2_valid = 0;
    key.ipv4_2_valid = 1;
    key.ipv6_2_valid = 0;
    data.action_id = KEY_TUNNELED_TUNNELED_IPV4_PACKET_ID;
    mask.ipv4_1_valid_mask = 0;
    mask.ipv6_1_valid_mask = 0;
    mask.ethernet_2_valid_mask = 0xFF;
    mask.ipv4_2_valid_mask = 0xFF;
    mask.ipv6_2_valid_mask = 0xFF;

    ret = apollo_impl_db()->key_tunneled_tbl()->insert(&key, &mask,
                                                       &data, (uint32_t *)&idx);
    return ret;
}

/**
 * @brief    initialize egress drop stats table
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
apollo_impl::egress_drop_stats_init_(void) {
    return SDK_RET_OK;
}

/**
 * @brief    initialize ingress drop stats table
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
apollo_impl::ingress_drop_stats_init_(void) {
    sdk_ret_t                      ret;
    p4i_drop_stats_swkey_t         key = { 0 };
    p4i_drop_stats_swkey_mask_t    key_mask = { 0 };
    p4i_drop_stats_actiondata_t    data = { 0 };

    for (uint32_t i = P4I_DROP_REASON_MIN; i <= P4I_DROP_REASON_MAX; i++) {
        key.control_metadata_p4i_drop_reason = ((uint32_t)1 << i);
        key_mask.control_metadata_p4i_drop_reason_mask = 0xFFFFFFFF;
        data.action_id = P4I_DROP_STATS_P4I_DROP_STATS_ID;
        ret =
            apollo_impl_db()->ingress_drop_stats_tbl()->insert_withid(&key,
                                                                     &key_mask,
                                                                     &data, i);
        SDK_ASSERT(ret == SDK_RET_OK);
    }
    return ret;
}

/**
 * @brief    initialize all the stats tables, where needed
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
apollo_impl::stats_init_(void) {
    ingress_drop_stats_init_();
    egress_drop_stats_init_();
    return SDK_RET_OK;
}

/**
 * @brief    program all datapath tables that require one time initialization
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
apollo_impl::table_init_(void) {
    sdk_ret_t    ret;

    ret = key_native_init_();
    if (ret != SDK_RET_OK) {
        return ret;
    }
    ret = key_tunneled_init_();
    return ret;
}

/**
 * @brief    init routine to initialize the pipeline
 * @return    SDK_RET_OK on success, failure status code on error
 */
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

    ret = sdk::asic::pd::asicpd_p4plus_table_mpu_base_init(&p4pd_cfg);
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = sdk::asic::pd::asicpd_table_mpu_base_init(&p4pd_cfg);
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = sdk::asic::pd::asicpd_program_table_mpu_pc();
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = sdk::asic::pd::asicpd_deparser_init();
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = sdk::asic::pd::asicpd_program_hbm_table_base_addr();
    SDK_ASSERT(ret == SDK_RET_OK);

    g_oci_impl_state.init(&api::g_oci_state);

    ret = init_service_lif();
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = table_init_();
    SDK_ASSERT(ret == SDK_RET_OK);

    return SDK_RET_OK;
}

/**
 * @brief    dump egress drop statistics
 * @param[in] fp       file handle
 */
void
apollo_impl::dump_egress_drop_stats_(FILE *fp) {
}

/**
 * @brief    dump ingress drop statistics
 * @param[in] fp       file handle
 */
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

/**
 * @brief    dump all the debug information to given file
 * @param[in] fp    file handle
 */
void
apollo_impl::debug_dump(FILE *fp) {
    fprintf(fp, "Ingress drop statistics\n");
    dump_ingress_drop_stats_(fp);
    fprintf(fp, "Egress drop statistics\n");
    dump_egress_drop_stats_(fp);
}

/** @} */    // end of OCI_PIPELINE_IMPL

}    // namespace impl

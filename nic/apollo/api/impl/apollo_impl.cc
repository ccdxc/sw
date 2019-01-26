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

namespace impl {

/**
 * @defgroup OCI_PIPELINE_IMPL - pipeline wrapper implementation
 * @ingroup OCI_PIPELINE
 * @{
 */

/*
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

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

/** @} */    // end of OCI_PIPELINE_IMPL

}    // namespace impl

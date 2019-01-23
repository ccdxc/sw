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
#include "nic/sdk/asic/pd/pd.hpp"

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

    return SDK_RET_OK;
}

/** @} */    // end of OCI_PIPELINE_IMPL

}    // namespace impl

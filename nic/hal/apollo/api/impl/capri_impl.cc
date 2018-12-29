/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    capri_impl.cc
 *
 * @brief   CAPRI asic implementation
 */

#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/hal/apollo/api/impl/capri_impl.hpp"

namespace impl {

/**
 * @defgroup OCI_ASIC_IMPL - asic wrapper implementation
 * @ingroup OCI_ASIC
 * @{
 */

/*
 * @brief    initialize an instance of capri impl class
 * @param[in] asic_cfg    asic information
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
capri_impl::init_(asic_cfg_t *asic_cfg) {
    return sdk::SDK_RET_OK;
}

/**
 * @brief    factory method to asic impl instance
 * @param[in] asic_cfg    asic information
 * @return    new instance of capri asic impl or NULL, in case of error
 */
capri_impl *
capri_impl::factory(asic_cfg_t *asic_cfg) {
    capri_impl    *impl;

    impl = (capri_impl *)SDK_CALLOC(SDK_MEM_ALLOC_OCI_ASIC_IMPL,
                                    sizeof(capri_impl));
    new (impl) capri_impl();
    if (impl->init_(asic_cfg) != sdk::SDK_RET_OK) {
        impl->~capri_impl();
        SDK_FREE(SDK_MEM_ALLOC_OCI_ASIC_IMPL, impl);
        return NULL;
    }
    return impl;
}

/**
 * @brief    init routine to initialize the asic
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
capri_impl::asic_init(void) {
    return sdk::SDK_RET_OK;
}

/** @} */    // end of OCI_ASIC_IMPL

}    // namespace impl

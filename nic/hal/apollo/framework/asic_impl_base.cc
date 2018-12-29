/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    asic_impl_base.cc
 *
 * @brief   implementation of asic impl methods
 */

#include "nic/hal/apollo/framework/asic_impl_base.hpp"
#include "nic/hal/apollo/api/impl/capri_impl.hpp"

namespace impl {

/**
 * @defgroup OCI_ASIC_IMPL - asic wrapper implementation
 * @ingroup OCI_ASIC
 * @{
 */

/**
 * @brief    factory method to asic impl instance
 * @param[in] asic_cfg    asic information
 * @return    new instance of asic impl or NULL, in case of error
 */
asic_impl_base *
asic_impl_base::factory(asic_cfg_t *asic_cfg) {
    switch (asic_cfg->asic_type) {
        case sdk::asic::SDK_ASIC_TYPE_CAPRI:
        return capri_impl::factory(asic_cfg);

    default:
        break;
    }
    return NULL;
}

/** @} */    // end of OCI_ASIC_IMPL

}    // namespace impl

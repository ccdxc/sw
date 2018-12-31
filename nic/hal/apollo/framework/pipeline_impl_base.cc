/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    pipeline_impl_base.cc
 *
 * @brief   implementation of pipeline impl methods
 */

#include "nic/hal/apollo/framework/pipeline_impl_base.hpp"
#include "nic/hal/apollo/api/impl/apollo_impl.hpp"

namespace impl {

/**
 * @defgroup OCI_PIPELINE_IMPL - pipeline wrapper implementation
 * @ingroup OCI_PIPELINE
 * @{
 */

/**
 * @brief    factory method to pipeline impl instance
 * @param[in] pipeline_cfg    pipeline information
 * @return    new instance of pipeline impl or NULL, in case of error
 */
pipeline_impl_base *
pipeline_impl_base::factory(pipeline_cfg_t *pipeline_cfg) {
    if (pipeline_cfg->name == "apollo") {
        return apollo_impl::factory(pipeline_cfg);
    }
    return NULL;
}

/** @} */    // end of OCI_PIPELINE_IMPL

}    // namespace impl

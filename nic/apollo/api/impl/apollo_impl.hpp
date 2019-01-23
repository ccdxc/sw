/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    apollo_impl.hpp
 *
 * @brief   apollo pipeline implementation
 */
#if !defined (__APOLLO_IMPL_HPP__)
#define __APOLLO_IMPL_HPP__

#include "nic/sdk/include/sdk/types.hpp"
#include "nic/apollo/framework/pipeline_impl_base.hpp"

namespace impl {

/**
 * @defgroup OCI_PIPELINE_IMPL - pipeline wrapper implementation
 * @ingroup OCI_PIPELINE
 * @{
 */

/**
 * @brief    pipeline implementation
 */
class apollo_impl : public pipeline_impl_base {
public:
    /**
     * @brief    factory method to pipeline impl instance
     * @param[in] pipeline_cfg    pipeline configuration information
     * @return    new instance of apollo pipeline impl or NULL, in case of error
     */
    static apollo_impl *factory(pipeline_cfg_t *pipeline_cfg);

    /**
     * @brief    init routine to initialize the pipeline
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t pipeline_init(void) override;

private:
    apollo_impl() {}

    ~apollo_impl() {}

    /*
     * @brief    initialize an instance of apollo impl class
     * @param[in] pipeline_cfg    pipeline information
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t init_(pipeline_cfg_t *pipeline_cfg);

private:
    pipeline_cfg_t           pipeline_cfg_;
};

}    // namespace impl

#endif    /** __APOLLO_IMPL_HPP__ */

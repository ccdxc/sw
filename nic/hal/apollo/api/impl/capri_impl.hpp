/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    capri_impl.hpp
 *
 * @brief   CAPRI asic implementation
 */
#if !defined (__CAPRI_IMPL_HPP__)
#define __CAPRI_IMPL_HPP__

#include "nic/sdk/include/sdk/types.hpp"
#include "nic/hal/apollo/framework/asic_impl_base.hpp"

namespace impl {

/**
 * @defgroup OCI_ASIC_IMPL - asic wrapper implementation
 * @ingroup OCI_ASIC
 * @{
 */

/**
 * @brief    asic implementation
 */
class capri_impl : public asic_impl_base {
public:
    /**
     * @brief    factory method to asic impl instance
     * @param[in] asic_cfg    asic configuration information
     * @return    new instance of capri asic impl or NULL, in case of error
     */
    static capri_impl *factory(asic_cfg_t *asic_cfg);

    /**
     * @brief    init routine to initialize the asic
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t asic_init(void) override;

private:
    capri_impl() {}

    ~capri_impl() {}

    /*
     * @brief    initialize an instance of capri impl class
     * @param[in] asic_cfg    asic information
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t init_(asic_cfg_t *asic_cfg);

private:
    asic_cfg_t           asic_cfg_;
};

}    // namespace impl

#endif    /** __CAPRI_IMPL_HPP__ */

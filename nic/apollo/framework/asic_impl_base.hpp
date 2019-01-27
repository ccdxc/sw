/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    asic_impl_base.hpp
 *
 * @brief   wrapper class for common asic APIs
 */
#if !defined (__ASIC_IMPL_HPP__)
#define __ASIC_IMPL_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/asic/asic.hpp"
#include "nic/apollo/framework/obj_base.hpp"

namespace impl {

/**
 * @defgroup OCI_ASIC_IMPL - asic wrapper implementation
 * @ingroup OCI_ASIC
 * @{
 */

/**
 * @brief    asic implementation
 */

class asic_impl_base : public obj_base {
public:
    /**
     * @brief    factory method to asic impl instance
     * @param[in] asic_cfg    asic configuration information
     * @return    new instance of asic impl or NULL, in case of error
     */
    static asic_impl_base *factory(asic_cfg_t *asic_cfg);

    /**
     * @brief    init routine to initialize the asic
     * @param[in] asic_cfg    asic configuration information
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t asic_init(asic_cfg_t *asic_cfg) {
        return sdk::SDK_RET_ERR;
    }

    /**
     * @brief    dump all the debug information to given file
     * @param[in] fp    file handle
     */
    virtual void debug_dump(FILE *fp) { }
};

/** @} */    // end of OCI_ASIC_IMPL

}    // namespace impl

using impl::asic_impl_base;

#endif    /** __ASIC_IMPL_HPP__ */

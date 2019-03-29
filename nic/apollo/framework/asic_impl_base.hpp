//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Wrapper class for common ASIC APIs
///
//----------------------------------------------------------------------------

#ifndef __FRAMEWORK_ASIC_IMPL_BASE_HPP__
#define __FRAMEWORK_ASIC_IMPL_BASE_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/asic/asic.hpp"
#include "nic/apollo/framework/obj_base.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_ASIC_IMPL ASIC wrapper implementation
/// @{

/// \brief ASIC implementation
class asic_impl_base : public obj_base {
public:
    /// \brief    factory method to instantiate ASIC impl instance
    /// \param[in] asic_cfg    ASIC configuration information
    /// \return    new instance of ASIC impl or NULL, in case of error
    static asic_impl_base *factory(asic_cfg_t *asic_cfg);

    /// \brief    init routine to initialize the asic
    /// \param[in] asic_cfg    ASIC configuration information
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t asic_init(asic_cfg_t *asic_cfg) {
        return SDK_RET_ERR;
    }

    /// \brief    monitor the asic
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t monitor(void) {
        return SDK_RET_OK;
    }

    /// \brief    dump all the debug information to given file
    /// \param[in]    fp File handle
    virtual void debug_dump(FILE *fp) {}
};

/// \@}

}    // namespace impl
}    // namespace api

using api::impl::asic_impl_base;

#endif    // __FRAMEWORK_ASIC_IMPL_BASE_HPP__

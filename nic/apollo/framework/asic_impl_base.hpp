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
    /// \brief Factory method to instantiate ASIC impl instance
    ///
    /// \param[in] asic_cfg ASIC configuration information
    ///
    /// \return New instance of ASIC impl or NULL, in case of error
    static asic_impl_base *factory(asic_cfg_t *asic_cfg);

    /// \brief Init routine to initialize the asic
    ///
    /// \param[in] asic_cfg ASIC configuration information
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t asic_init(asic_cfg_t *asic_cfg) {
        return sdk::SDK_RET_ERR;
    }

    /// \brief Dump all the debug information to given file
    ///
    /// \param[in] fp File handle
    virtual void debug_dump(FILE *fp) { }
};

/// \@}

}    // namespace impl
}    // namespace api

using api::impl::asic_impl_base;

#endif    // __FRAMEWORK_ASIC_IMPL_BASE_HPP__

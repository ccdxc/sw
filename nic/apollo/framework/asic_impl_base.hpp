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
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/asic/asic.hpp"
#include "nic/sdk/platform/asicerror/interrupts.hpp"
#include "nic/apollo/framework/obj_base.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"

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

    /// \brief    destroy method to free the ASIC impl instance
    /// \param[in] impl ASIC impl instance
    static void destroy(asic_impl_base *impl);

    /// \brief    init routine to initialize the asic
    /// \param[in] asic_cfg    ASIC configuration information
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t asic_init(asic_cfg_t *asic_cfg) {
        return SDK_RET_ERR;
    }

    /// \brief    monitor the system and asic interrupts
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t monitor(monitor_type_t monitor_type) {
        return SDK_RET_OK;
    }

    /// \brief  process the interrupts
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t process_interrupts(const intr_reg_t *reg,
                                         const intr_field_t *field) {
        return SDK_RET_OK;
    }

    /// \brief    set the core frequency with the given value
    /// \param[in] freq    clock frequency to set
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t set_frequency(pds_clock_freq_t freq) {
        return SDK_RET_ERR;
    }

    /// \brief    set the arm core frequency with the given value
    /// \param[in] freq    clock frequency to set
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t set_arm_frequency(pds_clock_freq_t freq) {
        return SDK_RET_ERR;
    }

    /// \brief    get the system temperature
    /// \param[out] temp    system temperature to be read
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t get_system_temperature(pds_system_temperature_t *temp) {
        return SDK_RET_ERR;
    }

    /// \brief     get the system power
    /// \param[out] pow    system power to be read
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t get_system_power(pds_system_power_t *pow) {
        return SDK_RET_ERR;
    }

    /// \brief     LLC setup
    /// \param[in]  llc_counters_t
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t llc_setup(sdk::asic::pd::llc_counters_t *llc_args) {
        return SDK_RET_ERR;
    }

    /// \brief      LLC Get
    /// \param[out]  llc_counters_t
    /// \return      SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t llc_get(sdk::asic::pd::llc_counters_t *llc_args) {
        return SDK_RET_ERR;
    }

    /// \brief      PB Stats Get
    /// \param[in]   cb      Callback
    ///              ctxt    Opaque context to be passed to callback
    /// \return      SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t pb_stats(debug::pb_stats_get_cb_t cb, void *ctxt) {
        return SDK_RET_ERR;
    }

private:
    sdk::platform::asic_type_t asic_type_;
};

/// \@}

}    // namespace impl
}    // namespace api

using api::impl::asic_impl_base;

#endif    // __FRAMEWORK_ASIC_IMPL_BASE_HPP__

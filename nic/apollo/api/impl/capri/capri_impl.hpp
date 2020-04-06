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
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/platform/capri/capri_tm_rw.hpp"
#include "nic/include/trace.hpp"
#include "nic/apollo/framework/asic_impl_base.hpp"
#include "nic/apollo/core/core.hpp"

namespace api {
namespace impl {

/**
 * @defgroup PDS_ASIC_IMPL - asic wrapper implementation
 * @ingroup PDS_ASIC
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
     * @brief destroy method to free the asic impl instance
     */
    static void destroy(capri_impl *impl);

    /**
     * @brief    init routine to initialize the asic
     * @param[in] asic_cfg    asic configuration information
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t asic_init(asic_cfg_t *asic_cfg) override;

    /**
     * @brief    monitor the system and asic interrupts
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t monitor(monitor_type_t monitor_type) override;

    /// \brief  process the interrupts
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t process_interrupts(const intr_reg_t *reg,
                                         const intr_field_t *field) override;

    /**
     * @brief    set the core frequency with the given value
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t set_frequency(pds_clock_freq_t freq) override;

    /**
     * @brief    set the arm core frequency with the given value
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t set_arm_frequency(pds_clock_freq_t freq) override;

    /**
     * @brief    get the system temperature
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t get_system_temperature(pds_system_temperature_t *temp) override;

    /**
     * @brief    get the system power
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t get_system_power(pds_system_power_t *pow) override;

    /**
     * @brief    LLC setup
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t llc_setup(sdk::asic::pd::llc_counters_t *llc_args) override;

    /**
     * @brief    LLC Get
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t llc_get(sdk::asic::pd::llc_counters_t *llc_args) override;

    /**
     * @brief      PB Stats Get
     * @param[in]   cb      Callback
     *              ctxt    Opaque context to be passed to callback
     * @return      SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t pb_stats(debug::pb_stats_get_cb_t cb,
                               void *ctxt) override;

private:
    capri_impl() {}

    ~capri_impl() {}

    /*
     * @brief    initialize an instance of capri impl class
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t init_(void);

private:
    asic_cfg_t           asic_cfg_;
};

}    // namespace impl
}    // namespace api

#endif    /** __CAPRI_IMPL_HPP__ */

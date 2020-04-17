/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    asic_impl.cc
 *
 * @brief   asic implementation
 */

#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/platform/sensor/sensor.hpp"
#include "nic/sdk/asic/port.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/impl/asic/asic_impl.hpp"
#include "nic/apollo/p4/include/artemis_table_sizes.h"
#include "nic/apollo/core/trace.hpp"
#include "nic/sdk/platform/sysmon/sysmon.hpp"
#include "nic/sdk/platform/asicerror/interrupts.hpp"

using namespace sdk;
using namespace sdk::asic::pd;

namespace api {
namespace impl {

/**
 * @defgroup PDS_ASIC_IMPL - asic wrapper implementation
 * @ingroup PDS_ASIC
 * @{
 */

/*
 * @brief    initialize an instance of asic impl class
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
asic_impl::init_ (void)
{
    return SDK_RET_OK;
}

/**
 * @brief    factory method to asic impl instance
 * @param[in] asic_cfg    asic information
 * @return    new instance of asic asic impl or NULL, in case of error
 */
asic_impl *
asic_impl::factory (asic_cfg_t *asic_cfg)
{
    asic_impl    *impl;

    impl = (asic_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_ASIC_IMPL,
                                    sizeof(asic_impl));
    new (impl) asic_impl();
    if (impl->init_() != SDK_RET_OK) {
        impl->~asic_impl();
        SDK_FREE(SDK_MEM_ALLOC_PDS_ASIC_IMPL, impl);
        return NULL;
    }
    return impl;
}

void
asic_impl::destroy (asic_impl *impl)
{
    asicpd_cleanup();
    sdk::lib::pal_teardown(impl->asic_cfg_.platform);
}

/**
 * @brief    init routine to initialize the asic
 * @param[in] asic_cfg    asic information
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
asic_impl::asic_init (asic_cfg_t *asic_cfg)
{
    sdk::lib::pal_ret_t    pal_ret;
    sdk_ret_t              ret;

    pal_ret = sdk::lib::pal_init(asic_cfg->platform);
    SDK_ASSERT(pal_ret == sdk::lib::PAL_RET_OK);
    if (sdk::asic::asic_is_hard_init()) {
        if (sdk::platform::upgrade_mode_none(asic_cfg->upg_init_mode)) {
            ret = asicpd_init(asic_cfg);
            // set the reserved min for uplink ports
            asicpd_tm_set_reserved_min(200);
        } else {
            ret = asicpd_upgrade_init(asic_cfg);
        }
    } else {
        ret = asicpd_soft_init(asic_cfg);
    }
    SDK_ASSERT(ret == SDK_RET_OK);

    /**< stash the config, in case we need it at later point in time */
    asic_cfg_ = *asic_cfg;

    return SDK_RET_OK;
}

static inline pd_adjust_perf_index_t
pds_clock_frequency_to_perf_id (pds_clock_freq_t freq)
{
    switch (freq) {
    case PDS_CLOCK_FREQUENCY_833:
        return PD_PERF_ID0;
        break;
    case PDS_CLOCK_FREQUENCY_900:
        return PD_PERF_ID1;
        break;
    case PDS_CLOCK_FREQUENCY_957:
        return PD_PERF_ID2;
        break;
    case PDS_CLOCK_FREQUENCY_1033:
        return PD_PERF_ID3;
        break;
    case PDS_CLOCK_FREQUENCY_1100:
        return PD_PERF_ID4;
        break;
    default:
        return PD_PERF_ID0;
        break;
    }
}

/**
 * @brief    set clock frequency
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
asic_impl::set_frequency (pds_clock_freq_t freq)
{
    sdk_ret_t ret;
    pd_adjust_perf_index_t perf_id;

    perf_id = pds_clock_frequency_to_perf_id(freq);
    ret = asicpd_adjust_perf(0, 0, perf_id, PD_PERF_SET);
    if (ret)
        PDS_TRACE_ERR("Clock frequency set failure, err %u", ret);

    return ret;
}

/**
 * @brief    set arm clock frequency
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
asic_impl::set_arm_frequency (pds_clock_freq_t freq)
{
    if (freq == PDS_CLOCK_FREQUENCY_2200) {
        PDS_TRACE_DEBUG("Setting ARM CPU freq to 2.2Ghz");
        asicpd_set_margin_by_value("arm", 950);
        asicpd_sbus_cpu_2200(0,0);
    } else if (freq == PDS_CLOCK_FREQUENCY_1666) {
        PDS_TRACE_DEBUG("Setting ARM CPU freq to 1.67Ghz");
        asicpd_set_margin_by_value("arm", 800);
        asicpd_sbus_cpu_1666(0,0);
    }
    return SDK_RET_OK;
}

/**
 * @brief    get system temperature
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
asic_impl::get_system_temperature (pds_system_temperature_t *temp)
{
    int rv;
    sdk::platform::sensor::system_temperature_t temperature;

    // read the temperatures
    rv = sdk::platform::sensor::read_temperatures(&temperature);
    if (rv == 0) {
        temp->dietemp = temperature.dietemp/1000;
        temp->localtemp = temperature.localtemp/1000;
        temp->hbmtemp = temperature.hbmtemp;
    } else {
        PDS_TRACE_ERR("Temperature reading failed");
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

/**
 * @brief    get system power
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
asic_impl::get_system_power (pds_system_power_t *pow)
{
    int rv;
    sdk::platform::sensor::system_power_t power;

    // read the power
    rv = sdk::platform::sensor::read_powers(&power);
    if (rv == 0) {
        pow->pin = power.pin/1000000;
        pow->pout1 = power.pout1/1000000;
        pow->pout2 = power.pout2/1000000;
    } else {
        PDS_TRACE_ERR("Power reading failed");
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

/**
 * @brief    LLC setup
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
asic_impl::llc_setup (sdk::asic::pd::llc_counters_t *llc_args)
{
    return asicpd_llc_setup(llc_args);
}

/**
 * @brief      LLC Get
 * @return      SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
asic_impl::llc_get (sdk::asic::pd::llc_counters_t *llc_args)
{
    return asicpd_llc_get(llc_args);
}

/**
 * @brief      PB Stats Get
 * @param[in]   cb      Callback
 *              ctxt    Opaque context to be passed to callback
 * @return      SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
asic_impl::pb_stats (debug::pb_stats_get_cb_t cb, void *ctxt)
{
    sdk_ret_t ret;
    pds_pb_debug_stats_t pb_stats = {0};
    tm_debug_stats_t tm_debug_stats;

    for (uint32_t tm_port = 0; tm_port < TM_NUM_PORTS; tm_port++) {
        memset(&pb_stats, 0, sizeof(pb_stats));
        memset(&tm_debug_stats, 0, sizeof(tm_debug_stats));
        ret = asicpd_tm_debug_stats_get(tm_port, &tm_debug_stats, false);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Get PB stats failed for port {}", tm_port);
            continue;
        }
        memcpy(&pb_stats.stats, &tm_debug_stats, sizeof(tm_debug_stats));
        ret = asicpd_queue_stats_get(tm_port,
                                     (void *) &pb_stats.qos_queue_stats);
        pb_stats.port = tm_port;
        cb(&pb_stats, ctxt);
    }
    return SDK_RET_OK;
}

/**
 * @brief    monitor the system and asic interrupts
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
asic_impl::monitor (monitor_type_t monitor_type)
{
    if (monitor_type == monitor_type_t::MONITOR_TYPE_SYSTEM) {
        sysmon_monitor();
    } else if (monitor_type == monitor_type_t::MONITOR_TYPE_INTERRUPTS) {
        traverse_interrupts();
    }
    return SDK_RET_OK;
}

/// \brief  process the interrupts
/// \return SDK_RET_OK on success, failure status code on error
sdk_ret_t
asic_impl::process_interrupts (const intr_reg_t *reg, const intr_field_t *field)
{
    bool iscattrip = false;
    bool iseccerr = false;

    // TODO use enums
    // invoke unravel interrupts for mc[0-7]_mch_int_mc registers
    switch (reg->id) {
    case 396:
    case 400:
    case 404:
    case 408:
    case 412:
    case 416:
    case 420:
    case 424:
        asicpd_unravel_hbm_intrs(&iscattrip, &iseccerr, true);
        if (iscattrip == false && iseccerr == true) {
            PDS_HMON_TRACE_ERR("ECCERR observed on the system.");
        }
        break;
    default:
        break;
    }
    return SDK_RET_OK;
}

/** @} */    // end of PDS_ASIC_IMPL

}    // namespace impl
}    // namespace api

/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    capri_impl.cc
 *
 * @brief   CAPRI asic implementation
 */

#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/platform/sensor/sensor.hpp"
#include "nic/sdk/third-party/asic/capri/verif/apis/cap_freq_api.h"
#include "nic/sdk/asic/port.hpp"
#include "nic/sdk/platform/capri/capri_tm_rw.hpp"
#include "nic/sdk/third-party/asic/capri/verif/apis/cap_freq_api.h"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/impl/capri/capri_impl.hpp"
#include "nic/apollo/p4/include/artemis_table_sizes.h"
#include "nic/apollo/core/trace.hpp"
#include "nic/sdk/third-party/asic/capri/verif/apis/cap_platform_api.h"
#include "nic/sdk/platform/sysmon/sysmon.hpp"

namespace api {
namespace impl {

/**
 * @defgroup PDS_ASIC_IMPL - asic wrapper implementation
 * @ingroup PDS_ASIC
 * @{
 */

/*
 * @brief    initialize an instance of capri impl class
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
capri_impl::init_(void) {
    return SDK_RET_OK;
}

/**
 * @brief    factory method to asic impl instance
 * @param[in] asic_cfg    asic information
 * @return    new instance of capri asic impl or NULL, in case of error
 */
capri_impl *
capri_impl::factory(asic_cfg_t *asic_cfg) {
    capri_impl    *impl;

    impl = (capri_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_ASIC_IMPL,
                                    sizeof(capri_impl));
    new (impl) capri_impl();
    if (impl->init_() != SDK_RET_OK) {
        impl->~capri_impl();
        SDK_FREE(SDK_MEM_ALLOC_PDS_ASIC_IMPL, impl);
        return NULL;
    }
    return impl;
}

void
capri_impl::destroy(capri_impl *impl) {
    sdk::asic::pd::asicpd_cleanup();
    sdk::lib::pal_teardown(impl->asic_cfg_.platform);
}

/**
 * @brief    init routine to initialize the asic
 * @param[in] asic_cfg    asic information
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
capri_impl::asic_init(asic_cfg_t *asic_cfg) {
    sdk::lib::pal_ret_t    pal_ret;
    sdk_ret_t              ret;

    pal_ret = sdk::lib::pal_init(asic_cfg->platform);
    SDK_ASSERT(pal_ret == sdk::lib::PAL_RET_OK);
    ret = sdk::asic::pd::asicpd_init(asic_cfg);
    SDK_ASSERT(ret == SDK_RET_OK);

    /**< stash the config, in case we need it at later point in time */
    asic_cfg_ = *asic_cfg;

    // set the reserved min for uplink ports
    if (sdk::asic::asic_is_hard_init()) {
        sdk::platform::capri::capri_tm_set_reserved_min(200);
    }
    return SDK_RET_OK;
}

static inline pen_adjust_index_t
pds_clock_frequency_to_perf_id (pds_clock_freq_t freq)
{
    switch (freq) {
    case PDS_CLOCK_FREQUENCY_833:
        return PEN_PERF_ID0;
        break;
    case PDS_CLOCK_FREQUENCY_900:
        return PEN_PERF_ID1;
        break;
    case PDS_CLOCK_FREQUENCY_957:
        return PEN_PERF_ID2;
        break;
    case PDS_CLOCK_FREQUENCY_1033:
        return PEN_PERF_ID3;
        break;
    case PDS_CLOCK_FREQUENCY_1100:
        return PEN_PERF_ID4;
        break;
    default:
        return PEN_PERF_ID0;
        break;
    }
}

/**
 * @brief    set clock frequency
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
capri_impl::set_frequency(pds_clock_freq_t freq) {
    pen_adjust_index_t perf_id;
    pen_adjust_perf_status_t ret;

    perf_id = pds_clock_frequency_to_perf_id(freq);
    ret = cap_top_adjust_perf(0, 0, perf_id, PEN_PERF_SET);
    if (ret != PEN_PERF_SUCCESS) {
        PDS_TRACE_ERR("Clock frequency set failure, err %u", ret);
        return SDK_RET_ERR;
    }

    return SDK_RET_OK;
}

/**
 * @brief    set arm clock frequency
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
capri_impl::set_arm_frequency(pds_clock_freq_t freq) {
    if (freq == PDS_CLOCK_FREQUENCY_2200) {
        PDS_TRACE_DEBUG("Setting ARM CPU freq to 2.2Ghz");
        cap_set_margin_by_value("arm", 950);
        cap_top_sbus_cpu_2200(0,0);
    } else if (freq == PDS_CLOCK_FREQUENCY_1666) {
        PDS_TRACE_DEBUG("Setting ARM CPU freq to 1.67Ghz");
        cap_set_margin_by_value("arm", 800);
        cap_top_sbus_cpu_1666(0,0);
    }
    return SDK_RET_OK;
}

/**
 * @brief    get system temperature
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
capri_impl::get_system_temperature(pds_system_temperature_t *temp) {
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
capri_impl::get_system_power(pds_system_power_t *pow) {
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
capri_impl::llc_setup(sdk::asic::pd::llc_counters_t *llc_args) {
    return sdk::asic::pd::asic_pd_llc_setup(llc_args);
}

/**
 * @brief      LLC Get
 * @return      SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
capri_impl::llc_get(sdk::asic::pd::llc_counters_t *llc_args) {
    return sdk::asic::pd::asic_pd_llc_get(llc_args);
}

/**
 * @brief      PB Stats Get
 * @param[in]   cb      Callback
 *              ctxt    Opaque context to be passed to callback
 * @return      SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
capri_impl::pb_stats(debug::pb_stats_get_cb_t cb, void *ctxt) {
    sdk_ret_t ret;
    pds_pb_debug_stats_t pb_stats = {0};

    for (uint32_t tm_port = 0; tm_port < TM_NUM_PORTS; tm_port++) {
        memset(&pb_stats, 0, sizeof(pb_stats));
        ret = capri_tm_get_pb_debug_stats(tm_port, &pb_stats.stats, false);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Get PB stats failed for port {}", tm_port);
            continue;
        }
        ret = capri_queue_stats_get(tm_port, &pb_stats.qos_queue_stats);
        pb_stats.port = tm_port;
        cb(&pb_stats, ctxt);
    }
    return SDK_RET_OK;
}

/**
 * @brief    monitor the asic
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
capri_impl::monitor (void) {
    sysmon_monitor();
    return SDK_RET_OK;
}

/** @} */    // end of PDS_ASIC_IMPL

}    // namespace impl
}    // namespace api

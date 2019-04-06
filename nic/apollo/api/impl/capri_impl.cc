/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    capri_impl.cc
 *
 * @brief   CAPRI asic implementation
 */

#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/platform/sensor/sensor.hpp"
#include "nic/sdk/third-party/asic/capri/verif/apis/cap_freq_api.h"
#include "nic/sdk/platform/capri/capri_tm_rw.hpp"
#include "nic/sdk/third-party/asic/capri/verif/apis/cap_freq_api.h"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/impl/capri_impl.hpp"
#include "nic/apollo/core/trace.hpp"

namespace api {
namespace impl {

#define OBFL_LOG_DEBUG(fmt, ...)                                       \
{                                                                      \
    obfl_logger_->logger()->debug(fmt, ##__VA_ARGS__);                 \
    obfl_logger_->logger()->flush();                                   \
}

#define OBFL_LOG_ERR(fmt, ...)                                         \
{                                                                      \
    obfl_logger_->logger()->error(fmt, ##__VA_ARGS__);                 \
    obfl_logger_->logger()->flush();                                   \
}

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
    if (g_pds_state.platform_type() == platform_type_t::PLATFORM_TYPE_HW) {
        obfl_logger_ =
            utils::log::factory("obfl", api::g_pds_state.control_cores_mask(),
                                utils::log_mode_sync, false,
                                "/obfl/asicmon.log", (1 << 20), 5,
                                utils::trace_debug, utils::log_none);
    }
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
    ret = sdk::asic::asic_init(asic_cfg);
    SDK_ASSERT(ret == SDK_RET_OK);

    /**< stash the config, in case we need it at later point in time */
    asic_cfg_ = *asic_cfg;

    return SDK_RET_OK;
}

/**
 * @brief    dump per TM port stats
 * @param[in] fp       file handle
 * @param[in] stats    pointer to the stats
 */
void
capri_impl::dump_tm_debug_stats_(FILE *fp, tm_pb_debug_stats_t *debug_stats)
{
    fprintf(fp, "    in  : %u\n", debug_stats->buffer_stats.sop_count_in);
    fprintf(fp, "    out : %u\n", debug_stats->buffer_stats.sop_count_out);
    fprintf(fp, "    intrinsic drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_INTRINSIC_DROP]);
    fprintf(fp, "    discarded : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_DISCARDED]);
    fprintf(fp, "    admitted : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_ADMITTED]);
    fprintf(fp, "    out of cells drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_OUT_OF_CELLS_DROP]);
    fprintf(fp, "    out of cells drop 2 : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_OUT_OF_CELLS_DROP_2]);
    fprintf(fp, "    out of credit drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_OUT_OF_CREDIT_DROP]);
    fprintf(fp, "    truncation drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_TRUNCATION_DROP]);
    fprintf(fp, "    port disabled drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_PORT_DISABLED_DROP]);
    fprintf(fp, "    copy to cpu tail drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_COPY_TO_CPU_TAIL_DROP]);
    fprintf(fp, "    span tail drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_SPAN_TAIL_DROP]);
    fprintf(fp, "    min size violation drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_MIN_SIZE_VIOLATION_DROP]);
    fprintf(fp, "    enqueue error drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_ENQUEUE_ERROR_DROP]);
    fprintf(fp, "    invalid port drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_INVALID_PORT_DROP]);
    fprintf(fp, "    invalid output queue drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_INVALID_OUTPUT_QUEUE_DROP]);
}

static inline pen_adjust_index_t
pds_clock_frequency_to_perf_id (pds_clock_freq_t freq)
{
    switch(freq) {
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
capri_impl::set_frequency (pds_clock_freq_t freq) {
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
 * @brief    get system temperature
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
capri_impl::get_system_temperature (pds_system_temperature_t *temp) {
    int rv;
    sdk::platform::sensor::system_temperature_t temperature;

    // read the temperatures
    rv = sdk::platform::sensor::read_temperatures(&temperature);
    if (rv == 0) {
        temp->dietemp = temperature.dietemp;
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
capri_impl::get_system_power (pds_system_power_t *pow) {
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
capri_impl::llc_setup (sdk::asic::pd::llc_counters_t *llc_args) {
    return sdk::asic::pd::asic_pd_llc_setup(llc_args);
}

/**
 * @brief      LLC Get
 * @return      SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
capri_impl::llc_get (sdk::asic::pd::llc_counters_t *llc_args) {
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
            OBFL_LOG_ERR("Get PB stats failed for port {}", tm_port);
            continue;
        }
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
    int rv;
    sdk::platform::sensor::system_power_t power;
    sdk::platform::sensor::system_temperature_t temperature;

    // read the temperatures
    rv = sdk::platform::sensor::read_temperatures(&temperature);
    if (rv == 0) {
        OBFL_LOG_DEBUG("Die temperature is {}C, local temperature is {}C,"
                       " HBM temperature is {}C", temperature.dietemp,
                       temperature.localtemp/1000, temperature.hbmtemp);
    } else {
        OBFL_LOG_ERR("Temperature reading failed");
    }

    // read the power
    rv = sdk::platform::sensor::read_powers(&power);
    if (rv == 0) {
        OBFL_LOG_DEBUG("Power of pin is {}W, pout1 is {}W, pout2 is {}W",
                       power.pin/1000000, power.pout1/1000000,
                       power.pout2/1000000);
    } else {
        OBFL_LOG_ERR("Power reading failed");
    }
    return SDK_RET_OK;
}

/**
 * @brief    dump all the debug information to given file
 * @param[in] fp    file handle
 */
void
capri_impl::debug_dump(FILE *fp) {
    sdk_ret_t              ret;
    bool                   reset = true;
    tm_pb_debug_stats_t    debug_stats;

    fprintf(fp, "PB/TM statistics\n");
    for (uint32_t tm_port = 0; tm_port < TM_NUM_PORTS; tm_port++) {
        memset(&debug_stats, 0, sizeof(debug_stats));
        ret = capri_tm_get_pb_debug_stats(tm_port, &debug_stats, reset);
        if ((tm_port >= TM_UPLINK_PORT_BEGIN) &&
            (tm_port <= TM_UPLINK_PORT_END)) {
            fprintf(fp, "  TM Port %u\n", tm_port);
            dump_tm_debug_stats_(fp, &debug_stats);
        } else if (tm_port == TM_DMA_PORT_BEGIN) {
            fprintf(fp, "  DMA\n");
            dump_tm_debug_stats_(fp, &debug_stats);
        } else if (tm_port == TM_PORT_INGRESS) {
            fprintf(fp, "  P4 IG\n");
            dump_tm_debug_stats_(fp, &debug_stats);
        } else if (tm_port == TM_PORT_EGRESS) {
            fprintf(fp, "  P4 EG\n");
            dump_tm_debug_stats_(fp, &debug_stats);
        }
    }
    fprintf(fp, "\n");
}

/** @} */    // end of PDS_ASIC_IMPL

}    // namespace impl
}    // namespace api

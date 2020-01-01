/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    debug.cc
 *
 * @brief   This file has helper routines for troubleshooting the system
 */

#include "nic/sdk/linkmgr/port_mac.hpp"
#include "nic/sdk/linkmgr/linkmgr.hpp"
#include "nic/sdk/include/sdk/fd.hpp"
#include "nic/sdk/platform/asicerror/interrupts.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/port.hpp"
#include "nic/apollo/api/debug.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace debug {

/**
 * @brief        set clock frequency
 * @param[in]    freq clock frequency to be set
 * @return       #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_clock_frequency_update (pds_clock_freq_t freq)
{
    return impl_base::asic_impl()->set_frequency(freq);
}

/**
 * @brief        set arm clock frequency
 * @param[in]    freq clock frequency to be set
 * @return       #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_arm_clock_frequency_update (pds_clock_freq_t freq)
{
    return impl_base::asic_impl()->set_arm_frequency(freq);
}

/**
 * @brief        get system temperature
 * @param[out]   Temperate to be read
 * @return       #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_get_system_temperature (pds_system_temperature_t *temp)
{
    return impl_base::asic_impl()->get_system_temperature(temp);
}

/**
 * @brief        get system power
 * @param[out]   Power to be read
 * @return       #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_get_system_power (pds_system_power_t *pow)
{
    return impl_base::asic_impl()->get_system_power(pow);
}

/**
 * @brief       Gets API stats for different tables
 * @param[in]   CB to fill API stats & ctxt
 * @return      #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_table_stats_get (debug::table_stats_get_cb_t cb, void *ctxt)
{
    return impl_base::pipeline_impl()->table_stats(cb, ctxt);
}

sdk_ret_t
dump_interrupts (int fd)
{
    dprintf(fd, "%s\n", std::string(80, '-').c_str());
    dprintf(fd, "%-50s%-10s%-9s%-11s\n",
            "Name", "Count", "Severity", "Description");
    dprintf(fd, "%s\n", std::string(80, '-').c_str());
    ::walk_interrupts(intr_dump_cb, &fd);
    return SDK_RET_OK;
}

sdk_ret_t
clear_interrupts (int fd)
{
    ::clear_interrupts();
    dprintf(fd, "Interrupts cleared\n");
    return SDK_RET_OK;
}

/**
 * @brief       Handles command based on ctxt
 * @param[in]   ctxt  Context for CLI handler
 * @return      #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_handle_cmd (cmd_ctxt_t *ctxt)
{
    switch (ctxt->cmd) {
    case CLI_CMD_MAPPING_DUMP:
        return impl_base::pipeline_impl()->handle_cmd(ctxt);
    case CLI_CMD_INTR_DUMP:
        dump_interrupts(ctxt->fd);
        break;
    case CLI_CMD_INTR_CLEAR:
        clear_interrupts(ctxt->fd);
        break;
    case CLI_CMD_API_ENGINE_STATS_DUMP:
        api::api_engine_get()->dump_api_counters(ctxt->fd);
        break;
    default:
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

/**
 * @brief       Setup LLC
 * @param[in]   llc_counters_t with type set
 * @return      #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_llc_setup (llc_counters_t *llc_args)
{
    return impl_base::asic_impl()->llc_setup(llc_args);
}

/**
 * @brief       LLC Stats Get
 * @param[out]  llc_counters_t to be filled
 * @return      #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_llc_get (llc_counters_t *llc_args)
{
    return impl_base::asic_impl()->llc_get(llc_args);
}

sdk_ret_t
pds_pb_stats_get (debug::pb_stats_get_cb_t cb, void *ctxt)
{
    return impl_base::asic_impl()->pb_stats(cb, ctxt);
}

sdk_ret_t
pds_meter_stats_get (debug::meter_stats_get_cb_t cb, uint32_t lowidx,
                     uint32_t highidx, void *ctxt)
{
    return impl_base::pipeline_impl()->meter_stats(cb, lowidx, highidx, ctxt);
}

sdk_ret_t
pds_session_stats_get (debug::session_stats_get_cb_t cb, uint32_t lowidx,
                       uint32_t highidx, void *ctxt)
{
    return impl_base::pipeline_impl()->session_stats(cb, lowidx, highidx, ctxt);
}

sdk_ret_t
pds_fte_api_stats_get (void)
{
    return SDK_RET_OK;
}

sdk_ret_t
pds_fte_table_stats_get (void)
{
    return SDK_RET_OK;
}

sdk_ret_t
pds_fte_api_stats_clear (void)
{
    return SDK_RET_OK;
}

sdk_ret_t
pds_fte_table_stats_clear (void)
{
    return SDK_RET_OK;
}

sdk_ret_t
pds_session_get (debug::session_get_cb_t cb, void *ctxt)
{
    return impl_base::pipeline_impl()->session(cb, ctxt);
}

sdk_ret_t
pds_flow_get (debug::flow_get_cb_t cb, void *ctxt)
{
    return impl_base::pipeline_impl()->flow(cb, ctxt);
}

sdk_ret_t
pds_session_clear (uint32_t idx)
{
    return impl_base::pipeline_impl()->session_clear(idx);
}

sdk_ret_t
pds_flow_clear (uint32_t idx)
{
    return impl_base::pipeline_impl()->flow_clear(idx);
}

/**
 * @brief    start AACS server
 * @param[in]    aacs_server_port     AACS server port
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
start_aacs_server (uint32_t aacs_server_port)
{
    sdk::linkmgr::start_aacs_server(aacs_server_port);
    return SDK_RET_OK;
}

/**
 * @brief    stop AACS server
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
stop_aacs_server (void)
{
    sdk::linkmgr::stop_aacs_server();
    return SDK_RET_OK;
}

/**
 * @brief    get slab information
 * @param[in]   cb   callback function to be called
 * @paraam[in]  ctxt context for callback function
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_slab_get (api::state_walk_cb_t cb, void *ctxt)
{
    api::g_pds_state.slab_walk(cb, ctxt);
    // TODO: Revisit logic to access pds_impl_state through pds_state
    impl_base::pipeline_impl()->impl_state_slab_walk(cb, ctxt);
    return SDK_RET_OK;
}

}    // namespace debug

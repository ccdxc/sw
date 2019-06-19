/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    debug.cc
 *
 * @brief   This file has helper routines for troubleshooting the system
 */

#include "nic/sdk/linkmgr/port_mac.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/port.hpp"
#include "nic/apollo/api/debug.hpp"

namespace debug {

/**< statistics for eth type port */
std::string eth_mac_stats[MAX_MAC_STATS] = {
    "Frames Received OK",
    "Frames Received All (Good/Bad Frames)",
    "Frames Received with Bad FCS",
    "Frames with any bad (CRC, Length, Align)",
    "Octets Received in Good Frames",
    "Octets Received (Good/Bad Frames)",
    "Frames Received with Unicast Address",
    "Frames Received with Multicast Address",
    "Frames Received with Broadcast Address",
    "Frames Received of type PAUSE",
    "Frames Received with Bad Length",
    "Frames Received Undersized",
    "Frames Received Oversized",
    "Fragments Received",
    "Jabber Received",
    "Priority Pause Frames",
    "Stomped CRC",
    "Frame Too Long",
    "Rx VLAN Frames (Good)",
    "Frames Dropped (Buffer Full)",
    "Frames Received Length<64",
    "Frames Received Length=64",
    "Frames Received Length=65~127",
    "Frames Received Length=128~255",
    "Frames Received Length=256~511",
    "Frames Received Length=512~1023",
    "Frames Received Length=1024~1518",
    "Frames Received Length=1519~2047",
    "Frames Received Length=2048~4095",
    "Frames Received Length=4096~8191",
    "Frames Received Length=8192~9215",
    "Frames Received Length>=9216",
    "Frames Transmitted OK",
    "Frames Transmitted All (Good/Bad Frames)",
    "Frames Transmitted Bad",
    "Octets Transmitted Good",
    "Octets Transmitted Total (Good/Bad)",
    "Frames Transmitted with Unicast Address",
    "Frames Transmitted with Multicast Address",
    "Frames Transmitted with Broadcast Address",
    "Frames Transmitted of type PAUSE",
    "Frames Transmitted of type PriPAUSE",
    "Frames Transmitted VLAN",
    "Frames Transmitted Length<64",
    "Frames Transmitted Length=64",
    "Frames Transmitted Length=65~127",
    "Frames Transmitted Length=128~255",
    "Frames Transmitted Length=256~511",
    "Frames Transmitted Length=512~1023",
    "Frames Transmitted Length=1024~1518",
    "Frames Transmitted Length=1519~2047",
    "Frames Transmitted Length=2048~4095",
    "Frames Transmitted Length=4096~8191",
    "Frames Transmitted Length=8192~9215",
    "Frames Transmitted Length>=9216",
    "Pri-0 Frames Transmitted",
    "Pri-1 Frames Transmitted",
    "Pri-2 Frames Transmitted",
    "Pri-3 Frames Transmitted",
    "Pri-4 Frames Transmitted",
    "Pri-5 Frames Transmitted",
    "Pri-6 Frames Transmitted",
    "Pri-7 Frames Transmitted",
    "Pri-0 Frames Received",
    "Pri-1 Frames Received",
    "Pri-2 Frames Received",
    "Pri-3 Frames Received",
    "Pri-4 Frames Received",
    "Pri-5 Frames Received",
    "Pri-6 Frames Received",
    "Pri-7 Frames Received",
    "Transmit Pri-0 Pause 1US Count",
    "Transmit Pri-1 Pause 1US Count",
    "Transmit Pri-2 Pause 1US Count",
    "Transmit Pri-3 Pause 1US Count",
    "Transmit Pri-4 Pause 1US Count",
    "Transmit Pri-5 Pause 1US Count",
    "Transmit Pri-6 Pause 1US Count",
    "Transmit Pri-7 Pause 1US Count",
    "Receive Pri-0 Pause 1US Count",
    "Receive Pri-1 Pause 1US Count",
    "Receive Pri-2 Pause 1US Count",
    "Receive Pri-3 Pause 1US Count",
    "Receive Pri-4 Pause 1US Count",
    "Receive Pri-5 Pause 1US Count",
    "Receive Pri-6 Pause 1US Count",
    "Receive Pri-7 Pause 1US Count",
    "Receive Standard Pause 1US Count",
    "Frames Truncated",
};

std::string mgmt_mac_stats[MAX_MGMT_MAC_STATS] = {
    "Frames Received OK",
    "Frames Received All (Good/Bad Frames)",
    "Frames Received with Bad FCS",
    "Frames with any bad (CRC, Length, Align)",
    "Octets Received in Good Frames",
    "Octets Received (Good/Bad Frames)",
    "Frames Received with Unicast Address",
    "Frames Received with Multicast Address",
    "Frames Received with Broadcast Address",
    "Frames Received of type PAUSE",
    "Frames Received with Bad Length",
    "Frames Received Undersized",
    "Frames Received Oversized",
    "Fragments Received",
    "Jabber Received",
    "Frames Received Length=64",
    "Frames Received Length=65~127",
    "Frames Received Length=128~255",
    "Frames Received Length=256~511",
    "Frames Received Length=512~1023",
    "Frames Received Length=1024~1518",
    "Frames Received > 1518",
    "Frames Received fifo full",
    "Frames Transmitted OK",
    "Frames Transmitted All (Good/Bad Frames)",
    "Frames Transmitted Bad",
    "Octets Transmitted Good",
    "Octets Transmitted Total (Good/Bad)",
    "Frames Transmitted with Unicast Address",
    "Frames Transmitted with Multicast Address",
    "Frames Transmitted with Broadcast Address",
    "Frames Transmitted of type PAUSE",
};

static inline void
port_info_dump (sdk::linkmgr::port_args_t *port_info, void *ctxt)
{
    FILE    *fp = (FILE *)ctxt;

    fprintf(fp, "Port %u, Type %u\n", port_info->port_num,
            (uint32_t)port_info->port_type);

    if (port_info->port_type == port_type_t::PORT_TYPE_ETH) {
        for (uint32_t i = 0; i < MAX_MAC_STATS; i++) {
            fprintf(fp, "  %-41s : %lu\n", eth_mac_stats[i].c_str(),
                    port_info->stats_data[i]);
        }
    } else if (port_info->port_type == port_type_t::PORT_TYPE_MGMT) {
        for (uint32_t i = 0; i < MAX_MGMT_MAC_STATS; i++) {
            fprintf(fp, "%-41s : %lu\n", mgmt_mac_stats[i].c_str(),
                    port_info->stats_data[i]);
        }
    }
    fprintf(fp, "\n");
}

/**
 * @brief        dump all the information needed for troubleshooting the system
 * @param[in]    filename    full path of the file to write to
 * @return       #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
system_dump (const char *filename) {
    FILE    *fp = NULL;

    if ((!filename) || (!(fp = fopen(filename, "w+")))) {
        PDS_TRACE_ERR("Cannot open output file %s", filename);
        return SDK_RET_ERR;
    }

    api::port_get(0, port_info_dump, fp);
    impl_base::debug_dump(fp);
    fflush(fp);
    // pds_state::dump()

    return SDK_RET_OK;
}

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
pds_meter_stats_get (debug::meter_stats_get_cb_t cb, uint32_t lowidx, uint32_t highidx, void *ctxt)
{
    return impl_base::asic_impl()->meter_stats(cb, lowidx, highidx, ctxt);
}

}    // namespace debug

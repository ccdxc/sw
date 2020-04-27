//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines callbacks and metrics update for system monitoring
///
//----------------------------------------------------------------------------

#include "nic/sdk/platform/sensor/sensor.hpp"

namespace api {

/// \brief     event callback for system power
/// \param[in] power system power info
static inline void
power_event_cb (system_power_t *power)
{
    PDS_HMON_TRACE_VERBOSE("Power of pin is %dW, pout1 is %dW, "
                           "pout2 is %dW", power->pin,
                           power->pout1, power->pout2);
    sdk::metrics::metrics_update(g_pds_state.power_metrics_handle(),
                                 *(sdk::metrics::key_t *)uuid_from_objid(0).id,
                                 (uint64_t *)power);
}

/// \brief      populate the array based on asic temperature fields
/// \param[in]  temperature temperature info
/// \param[out] arr array to be populated
static inline void
populate_asic_temperature (system_temperature_t *temperature,
                           uint64_t *arr)
{
    arr[asic_temp_metrics_type_t::ASIC_TEMP_METRICS_TYPE_LOCAL] =
            temperature->localtemp;
    arr[asic_temp_metrics_type_t::ASIC_TEMP_METRICS_TYPE_DIE] =
            temperature->dietemp;
    arr[asic_temp_metrics_type_t::ASIC_TEMP_METRICS_TYPE_HBM] =
            temperature->hbmtemp;
}

/// \brief      populate the array based on port temperature fields
/// \param[in]  temperature temperature info
/// \parma[in]  phy_port physical port
/// \param[out] arr array to be populated
static inline void
populate_port_temperature (system_temperature_t *temperature,
                           uint32_t phy_port, uint64_t *arr)
{
    arr[port_temp_metrics_type_t::PORT_TEMP_METRICS_TYPE_PORT] =
            temperature->xcvrtemp[phy_port].temperature;
    arr[port_temp_metrics_type_t::PORT_TEMP_METRICS_TYPE_WARN] =
            temperature->xcvrtemp[phy_port].warning_temperature;
    arr[port_temp_metrics_type_t::PORT_TEMP_METRICS_TYPE_ALARM] =
            temperature->xcvrtemp[phy_port].alarm_temperature;
}

/// \brief     update metrics for port temperature
/// \param[in] temperature temperature info
static inline void
port_temperature_metrics_update (system_temperature_t *temperature)
{
    uint64_t port_temp_metrics[
        port_temp_metrics_type_t::PORT_TEMP_METRICS_TYPE_MAX] = { 0 };
    uint32_t num_phy_ports;
    pds_ifindex_t ifindex;

    num_phy_ports = g_pds_state.catalogue()->num_fp_ports();
    for (uint32_t phy_port = 1; phy_port <= num_phy_ports; phy_port++) {
        populate_port_temperature(temperature, phy_port,
                                  port_temp_metrics);
        ifindex = ETH_IFINDEX(g_pds_state.catalogue()->slot(),
                              phy_port, ETH_IF_DEFAULT_CHILD_PORT);
        sdk::metrics::metrics_update(
            g_pds_state.port_temperature_metrics_handle(),
            *(sdk::metrics::key_t *)uuid_from_objid(ifindex).id,
            port_temp_metrics);
    }
}

/// \brief     update metrics for asic temperature
/// \param[in] temperature temperature info
static inline void
asic_temperature_metrics_update (system_temperature_t *temperature)
{
    uint64_t asic_temp_metrics[
        asic_temp_metrics_type_t::ASIC_TEMP_METRICS_TYPE_MAX] = { 0 };

    populate_asic_temperature(temperature, asic_temp_metrics);
    sdk::metrics::metrics_update(g_pds_state.asic_temperature_metrics_handle(),
                                 *(sdk::metrics::key_t *)uuid_from_objid(0).id,
                                 asic_temp_metrics);
}

/// \brief     event callback for asic and port temperature
/// \param[in] temperature temperature info
/// \param[in] hbm_event events for hbm temperature threshold
static inline void
temperature_event_cb (system_temperature_t *temperature,
                      sysmon_hbm_threshold_event_t hbm_event)
{
    PDS_HMON_TRACE_VERBOSE("Die temperature is %dC, local temperature is "
                           "%dC, HBM temperature is %dC",
                           temperature->dietemp,
                           temperature->localtemp, temperature->hbmtemp);
    asic_temperature_metrics_update(temperature);
    port_temperature_metrics_update(temperature);
}

/// \brief     event callback for asic interrupts
/// \param[in] reg interrupt register info
/// \param[in] field interrupt field info
static inline void
interrupt_event_cb (const intr_reg_t *reg, const intr_field_t *field)
{
    switch (field->severity) {
    case INTR_SEV_TYPE_HW_RMA:
    case INTR_SEV_TYPE_FATAL:
    case INTR_SEV_TYPE_ERR:
        if (field->count == 1) {
            // log to onetime interrupt error
            PDS_INTR_TRACE_ERR("name: %s_%s, count: %u, severity: %s, desc: %s",
                reg->name, field->name, field->count,
                get_severity_str(field->severity).c_str(), field->desc);
        }
        // log in hmon error
        PDS_HMON_TRACE_ERR("name: %s_%s, count: %u, severity: %s, desc: %s",
            reg->name, field->name, field->count,
            get_severity_str(field->severity).c_str(), field->desc);
        break;
    case INTR_SEV_TYPE_INFO:
    default:
        break;
    }

    // post processing of interrupts
    impl_base::asic_impl()->process_interrupts(reg, field);
}

}    // namespace api

//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------
#include "sysmond_cpld_cb.hpp"
#include "platform/pal/include/pal.h"
#include "nic/sdk/platform/sensor/sensor.hpp"
#include "platform/src/app/sysmond/sysmond_cb.hpp"

#define PORT_1 0x11010001
#define PORT_5 0x11020001
#define AUX_FAN_PRESENT 1
#define AUX_FAN_TEMP_HYSTERESIS 10

void
cpld_temp_event_cb (
                    sdk::platform::sensor::system_temperature_t *temperature)
{
    static uint32_t aux_fan_state;
    pal_write_core_temp(temperature->dietemp);
    pal_write_board_temp(temperature->localtemp);
    pal_write_hbm_temp(temperature->hbmtemp);
    pal_write_hbmwarning_temp(temperature->hbmwarningtemp);
    pal_write_hbmcritical_temp(temperature->hbmcriticaltemp);
    pal_write_hbmfatal_temp(g_catalog->hbmtemperature_threshold());

    // Adding place holders for updating qsfp temperature to cpld
    pal_write_qsfp_temp(temperature->xcvrtemp[0].temperature, QSFP_PORT1);
    pal_write_qsfp_temp(temperature->xcvrtemp[1].temperature, QSFP_PORT2);
    pal_write_qsfp_alarm_temp(temperature->xcvrtemp[0].alarm_temperature, QSFP_PORT1);
    pal_write_qsfp_alarm_temp(temperature->xcvrtemp[1].alarm_temperature, QSFP_PORT2);
    pal_write_qsfp_warning_temp(temperature->xcvrtemp[0].warning_temperature, QSFP_PORT1);
    pal_write_qsfp_warning_temp(temperature->xcvrtemp[1].warning_temperature, QSFP_PORT2);

    if (g_catalog->aux_fan() == AUX_FAN_PRESENT) {
        if (aux_fan_state == 0 && 
            temperature->hbmtemp > g_catalog->aux_fan_threshold()) {
            aux_fan_state = 1;
            pal_enable_auxiliary_fan();
        } else if (aux_fan_state == 1 &&
                   temperature->hbmtemp < (g_catalog->aux_fan_threshold() - AUX_FAN_TEMP_HYSTERESIS)) {
            aux_fan_state = 0;
            pal_disable_auxiliary_fan();
        }
    }
    return;
}

void
cpld_liveness_event_cb (void)
{
    port::PortOperState port1_status;
    port::PortOperState port5_status;
    //Update cpld health reg0
    pal_cpld_increment_liveness();

    //TODO get the port across all pipelines
    port_get(PORT_1, &port1_status);
    port_get(PORT_5, &port5_status);
    if (port1_status == port::PORT_OPER_STATUS_UP) {
        pal_cpld_set_port0_link_status(SYSMOND_HEALTH_OK);
    } else {
        pal_cpld_set_port0_link_status(SYSMOND_HEALTH_NOT_OK);
    }
    if (port5_status == port::PORT_OPER_STATUS_UP) {
        pal_cpld_set_port1_link_status(SYSMOND_HEALTH_OK);
    } else {
        pal_cpld_set_port1_link_status(SYSMOND_HEALTH_NOT_OK);
    }
    return;
}

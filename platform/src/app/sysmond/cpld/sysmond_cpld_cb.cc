//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------
#include "sysmond_cpld_cb.hpp"
#include "platform/pal/include/pal.h"
#include "nic/sdk/platform/sensor/sensor.hpp"
#include "platform/src/app/sysmond/sysmond_cb.hpp"

#define PORT_1 0x11010001
#define PORT_5 0x11020001

void
cpld_temp_event_cb (
                    sdk::platform::sensor::system_temperature_t *temperature)
{
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

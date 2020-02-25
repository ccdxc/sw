// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "lib/thread/thread.hpp"
#include "lib/periodic/periodic.hpp"
#include "platform/drivers/xcvr.hpp"
#include "platform/drivers/xcvr_qsfp.hpp"
#include "lib/pal/pal.hpp"

namespace sdk {
namespace platform {

using sdk::lib::pal_ret_t;
using sdk::lib::qsfp_page_t;

#define MAX_XCVR_ACCESS_RETRIES 5

sdk_ret_t
qsfp_read_page (int port, qsfp_page_t pgno, int offset,
                int num_bytes, uint8_t *data) {
    pal_ret_t ret = sdk::lib::pal_qsfp_read(data, num_bytes,
                                            offset, pgno,
                                            MAX_XCVR_ACCESS_RETRIES,
                                            port + 1);
    if (ret == sdk::lib::PAL_RET_OK) {
        return SDK_RET_OK;
    } else {
        return SDK_RET_ERR;
    }
}

sdk_ret_t
qsfp_write_page (int port, qsfp_page_t pgno, int offset,
                 int num_bytes, uint8_t *data) {
    pal_ret_t ret = sdk::lib::pal_qsfp_write(data, num_bytes,
                                             offset, pgno,
                                             MAX_XCVR_ACCESS_RETRIES,
                                             port + 1);
    if (ret == sdk::lib::PAL_RET_OK) {
        return SDK_RET_OK;
    } else {
        return SDK_RET_ERR;
    }
}

sdk_ret_t
qsfp_enable (int port, bool enable, uint8_t mask)
{
    uint8_t   data    = 0x0;
    sdk_ret_t sdk_ret = SDK_RET_OK;

    SDK_TRACE_DEBUG("xcvr_port: %d, enable: %d, mask: 0x%x",
                    port+1, enable, mask);

    sdk_ret = qsfp_read_page(port, qsfp_page_t::QSFP_PAGE_LOW,
                             QSFP_OFFSET_TX_DISABLE, 1, &data);
    if (sdk_ret != SDK_RET_OK) {
        SDK_TRACE_ERR ("qsfp_port: %d, failed to read offset: 0x%x, page: %d",
                       port+1, QSFP_OFFSET_TX_DISABLE,
                       qsfp_page_t::QSFP_PAGE_LOW);
        return sdk_ret;
    }

    // clear the mask bits
    data = data & ~mask;

    // set the bits to disable TX
    if (enable == false) {
        data = data | mask;
    }

    qsfp_write_page(port, qsfp_page_t::QSFP_PAGE_LOW,
                    QSFP_OFFSET_TX_DISABLE, 1, &data);
    if (sdk_ret != SDK_RET_OK) {
        SDK_TRACE_ERR ("qsfp_port: %d, failed to write offset: 0x%x, page: %d",
                       port+1, QSFP_OFFSET_TX_DISABLE,
                       qsfp_page_t::QSFP_PAGE_LOW);
    }

    return sdk_ret;
}

sdk_ret_t
qsfp_sprom_parse (int port, uint8_t *data)
{
    // SFF 8024

    if (data[QSFP_OFFSET_LENGTH_CU] != 0) {
        set_cable_type(port, cable_type_t::CABLE_TYPE_CU);
    } else {
        set_cable_type(port, cable_type_t::CABLE_TYPE_FIBER);
    }

    // TODO reset when removed?
    xcvr_set_an_args(port, 0, false, 0);

    switch (data[QSFP_OFFSET_ETH_COMPLIANCE_CODES]) {
    case 0x0:
        SDK_TRACE_DEBUG("Xcvr port %d QSFP eth spec found unspecified compliance code", port);
        break;
    case 0x1:
        // 40G Active cable
        xcvr_set_pid(port, xcvr_pid_t::XCVR_PID_QSFP_40GBASE_AOC);
        set_cable_type(port, cable_type_t::CABLE_TYPE_FIBER);
        xcvr_set_cable_speed(port, port_speed_t::PORT_SPEED_40G);
        break;

    case 0x2:
        // 40GBASE-LR4
        xcvr_set_pid(port, xcvr_pid_t::XCVR_PID_QSFP_40GBASE_LR4);
        set_cable_type(port, cable_type_t::CABLE_TYPE_FIBER);
        xcvr_set_cable_speed(port, port_speed_t::PORT_SPEED_40G);
        break;

    case 0x4:
        // 40GBASE-SR4
        xcvr_set_pid(port, xcvr_pid_t::XCVR_PID_QSFP_40GBASE_SR4);
        set_cable_type(port, cable_type_t::CABLE_TYPE_FIBER);
        xcvr_set_cable_speed(port, port_speed_t::PORT_SPEED_40G);
        break;

    case 0x8:
        // 40GBASE-CR4
        xcvr_set_pid(port, xcvr_pid_t::XCVR_PID_QSFP_40GBASE_CR4);
        xcvr_set_an_args(port,
                         AN_USER_CAP_40GBKR4 | AN_USER_CAP_40GBCR4,
                         false,
                         0);
        xcvr_set_cable_speed(port, port_speed_t::PORT_SPEED_40G);
        break;

    default:
        SDK_TRACE_DEBUG("Xcvr port %u QSFP eth spec unsupported code 0x%u",
                         port, data[QSFP_OFFSET_ETH_COMPLIANCE_CODES]);
        break;
    }

    switch (data[QSFP_OFFSET_EXT_SPEC_COMPLIANCE_CODES]) {
    case 0x0:
        SDK_TRACE_DEBUG("Xcvr port %d QSFP ext spec found unspecified compliance code", port);
        break;
    case 0x1:
        // 100G AOC
        xcvr_set_pid(port, xcvr_pid_t::XCVR_PID_QSFP_100G_AOC);
        set_cable_type(port, cable_type_t::CABLE_TYPE_FIBER);
        xcvr_set_cable_speed(port, port_speed_t::PORT_SPEED_100G);
        break;

    case 0x2:
        // 100GBASE-SR4
        xcvr_set_pid(port, xcvr_pid_t::XCVR_PID_QSFP_100G_SR4);
        set_cable_type(port, cable_type_t::CABLE_TYPE_FIBER);
        xcvr_set_cable_speed(port, port_speed_t::PORT_SPEED_100G);
        break;

    case 0x3:
        // 100GBASE-LR4
        xcvr_set_pid(port, xcvr_pid_t::XCVR_PID_QSFP_100G_LR4);
        set_cable_type(port, cable_type_t::CABLE_TYPE_FIBER);
        xcvr_set_cable_speed(port, port_speed_t::PORT_SPEED_100G);
        break;

    case 0x4:
        // 100GBASE-ER4
        xcvr_set_pid(port, xcvr_pid_t::XCVR_PID_QSFP_100G_ER4);
        set_cable_type(port, cable_type_t::CABLE_TYPE_FIBER);
        xcvr_set_cable_speed(port, port_speed_t::PORT_SPEED_100G);
        break;

    case 0x6:
        // 100GBASE-CWDM4
        xcvr_set_pid(port, xcvr_pid_t::XCVR_PID_QSFP_100G_CWDM4);
        set_cable_type(port, cable_type_t::CABLE_TYPE_FIBER);
        xcvr_set_cable_speed(port, port_speed_t::PORT_SPEED_100G);
        break;

    case 0x7:
        // 100GBASE-PSM4
        xcvr_set_pid(port, xcvr_pid_t::XCVR_PID_QSFP_100G_PSM4);
        set_cable_type(port, cable_type_t::CABLE_TYPE_FIBER);
        xcvr_set_cable_speed(port, port_speed_t::PORT_SPEED_100G);
        break;

    case 0x8:
        // 100G ACC
        xcvr_set_pid(port, xcvr_pid_t::XCVR_PID_QSFP_100G_ACC);
        set_cable_type(port, cable_type_t::CABLE_TYPE_FIBER);
        xcvr_set_cable_speed(port, port_speed_t::PORT_SPEED_100G);
        break;

    case 0xb:
        // 100GBASE-CR4
        xcvr_set_pid(port, xcvr_pid_t::XCVR_PID_QSFP_100G_CR4);
        xcvr_set_an_args(port,
                         AN_USER_CAP_100GBKR4 | AN_USER_CAP_100GBCR4,
                         true,
                         AN_FEC_REQ_25GB_RSFEC);
        xcvr_set_cable_speed(port, port_speed_t::PORT_SPEED_100G);
        break;

    case 0x10:
        // 40GBASE-ER4
        xcvr_set_pid(port, xcvr_pid_t::XCVR_PID_QSFP_40GBASE_ER4);
        set_cable_type(port, cable_type_t::CABLE_TYPE_FIBER);
        xcvr_set_cable_speed(port, port_speed_t::PORT_SPEED_40G);
        break;

    case 0x17:
        // 100G CLR4
        break;

    case 0x18:
        // 100G AOC
        xcvr_set_pid(port, xcvr_pid_t::XCVR_PID_QSFP_100G_AOC);
        set_cable_type(port, cable_type_t::CABLE_TYPE_FIBER);
        xcvr_set_cable_speed(port, port_speed_t::PORT_SPEED_100G);
        break;

    case 0x19:
        // 100G ACC
        xcvr_set_pid(port, xcvr_pid_t::XCVR_PID_QSFP_100G_ACC);
        set_cable_type(port, cable_type_t::CABLE_TYPE_FIBER);
        xcvr_set_cable_speed(port, port_speed_t::PORT_SPEED_100G);
        break;

    case 0x1A:
        // 100G DWDM2
        break;

    default:
        SDK_TRACE_DEBUG("Xcvr port %u QSFP ext spec unsupported code 0x%u",
                         port, data[QSFP_OFFSET_ETH_COMPLIANCE_CODES]);
        break;
    }

    // For unknown QSFP, set 100G CU params
    if (xcvr_pid(port) == xcvr_pid_t::XCVR_PID_UNKNOWN) {
        // 100GBASE-CR4
        set_cable_type(port, cable_type_t::CABLE_TYPE_CU);
        xcvr_set_cable_speed(port, port_speed_t::PORT_SPEED_100G);
        xcvr_set_an_args(port,
                         AN_USER_CAP_100GBKR4 | AN_USER_CAP_100GBCR4,
                         true,
                         AN_FEC_REQ_25GB_RSFEC);
    }

    return SDK_RET_OK;
}

sdk_ret_t
qsfp_read_temp (int port, uint8_t *data)
{
    pal_ret_t ret = sdk::lib::qsfp_dom_read(data, 1, QSFP_MODULE_TEMPERATURE,
                                            MAX_XCVR_ACCESS_RETRIES,
                                            port + 1);
    if (ret == sdk::lib::PAL_RET_NOK) {
        //SDK_TRACE_ERR ("qsfp_read_temp_high_warning: port %d, failed to read temp: 0x%x",
                       //port+1, QSFP_MODULE_TEMP_HIGH_WARNING);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
qsfp_read_temp_high_alarm (int port, uint8_t *data)
{
    pal_ret_t ret = sdk::lib::qsfp_dom_read(data, 1, QSFP_MODULE_TEMP_HIGH_ALARM,
                                            MAX_XCVR_ACCESS_RETRIES,
                                            port + 1);
    if (ret == sdk::lib::PAL_RET_NOK) {
        //SDK_TRACE_ERR ("qsfp_read_temp_high_warning: port %d, failed to read temp: 0x%x",
                       //port+1, QSFP_MODULE_TEMP_HIGH_WARNING);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
qsfp_read_temp_high_warning (int port, uint8_t *data)
{
    pal_ret_t ret = sdk::lib::qsfp_dom_read(data, 1, QSFP_MODULE_TEMP_HIGH_WARNING,
                                            MAX_XCVR_ACCESS_RETRIES,
                                            port + 1);
    if (ret == sdk::lib::PAL_RET_NOK) {
        //SDK_TRACE_ERR ("qsfp_read_temp_high_warning: port %d, failed to read temp: 0x%x",
                       //port+1, QSFP_MODULE_TEMP_HIGH_WARNING);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
read_qsfp_temperature(int port, qsfp_temperature_t *qsfp_temp_data)
{
    sdk_ret_t sdk_ret;
    int temperature;
    int warning_temperature;
    int alarm_temperature;
    uint8_t data;

    sdk_ret = qsfp_read_temp(port, &data);
    if (sdk_ret == SDK_RET_OK &&
        (data > QSFP_TEMP_SANE_MIN && data < QSFP_TEMP_SANE_MAX)) {
            temperature = (int) data;
    } else {
            temperature = INVALID_TEMP;
    }

    sdk_ret = qsfp_read_temp_high_warning(port, &data);
    if (sdk_ret == SDK_RET_OK &&
        (data > QSFP_TEMP_SANE_MIN && data < QSFP_TEMP_SANE_MAX)) {
            warning_temperature = (int) data;
    } else {
            warning_temperature = INVALID_TEMP;
    }

    sdk_ret = qsfp_read_temp_high_alarm(port, &data);
    if (sdk_ret == SDK_RET_OK &&
        (data > QSFP_TEMP_SANE_MIN && data < QSFP_TEMP_SANE_MAX)) {
            alarm_temperature = (int) data;
    } else {
            alarm_temperature = INVALID_TEMP;
    }

    qsfp_temp_data->temperature = temperature;
    qsfp_temp_data->warning_temperature = warning_temperature;
    qsfp_temp_data->alarm_temperature = alarm_temperature;

    return SDK_RET_OK;
}

} // namespace platform
} // namespace sdk

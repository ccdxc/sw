// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __XCVR_QSFP_HPP__
#define __XCVR_QSFP_HPP__

#include "lib/pal/pal.hpp"

namespace sdk {
namespace platform {

#define QSFP_MODULE_TEMP_HIGH_ALARM           0x0
#define QSFP_MODULE_TEMP_HIGH_WARNING         0x4
#define QSFP_OFFSET_TX_DISABLE                0x56
#define QSFP_MODULE_TEMPERATURE               0x60
#define QSFP_OFFSET_LENGTH_CU                 146 // 0x92
#define QSFP_OFFSET_ETH_COMPLIANCE_CODES      131
#define QSFP_OFFSET_EXT_SPEC_COMPLIANCE_CODES 192

#define INVALID_TEMP                 0
#define QSFP_TEMP_SANE_MIN           1
#define QSFP_TEMP_SANE_MAX           127
#define QSFP_TEMP_THRESH_SANE_MIN    50
#define QSFP_TEMP_THRESH_SANE_MAX    100

using sdk::lib::qsfp_page_t;

typedef struct qsfp_temperature {
    uint32_t temperature;
    uint32_t warning_temperature;
    uint32_t alarm_temperature;
} qsfp_temperature_t;

sdk_ret_t qsfp_read_page(int port, qsfp_page_t pgno, int offset,
                         int num_bytes, uint8_t *data);
sdk_ret_t qsfp_enable(int port, bool enable, uint8_t mask);
sdk_ret_t qsfp_sprom_parse(int port, uint8_t *data);
sdk_ret_t qsfp_read_temp(int port, int *data);
sdk_ret_t qsfp_read_temp_high_alarm(int port, int *data);
sdk_ret_t qsfp_read_temp_high_warning(int port, int *data);
sdk_ret_t read_qsfp_temperature(int port, qsfp_temperature_t *data);

} // namespace platform
} // namespace sdk

#endif

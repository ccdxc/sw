// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#ifndef __TEMPERATURE_SENSOR_TEST_H__
#define __TEMPERATURE_SENSOR_TEST_H__

#include "diag_utils.h"
namespace sdk {
namespace platform {
namespace diag {

diag_ret_e temp_sensor_test(test_mode_e mode, int argc, char* argv[]);

} // namespace diag
} // namespace platform
} // namespace sdk
#endif //__TEMPERATURE_SENSOR_TEST_H__

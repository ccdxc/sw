// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#ifndef __RTC_TEST_H__
#define __RTC_TEST_H__

#include "diag_utils.h"
namespace sdk {
namespace platform {
namespace diag {

#define RTC_DEV_NODE "/dev/rtc0"
#define SLEEP_INTERVAL  (2)

diag_ret_e rtc_test(test_mode_e mode, int argc, char* argv[]);

} // namespace diag
} // namespace platform
} // namespace sdk
#endif //__RTC_TEST_H__

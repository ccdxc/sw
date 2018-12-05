#ifndef __RTC_TEST_H__
#define __RTC_TEST_H__

#include "diag_utils.h"

#define RTC_DEV_NODE "/dev/rtc0"
#define SLEEP_INTERVAL  (2)

diag_ret_e rtc_test(test_mode_e mode, int argc, char* argv[]);

#endif //__RTC_TEST_H__

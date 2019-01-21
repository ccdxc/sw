// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#ifndef __CPLD_TEST_H__
#define __CPLD_TEST_H__

#include "diag_utils.h"

namespace sdk {
namespace platform {
namespace diag {

#define NAPLES_CPLD_ID_REG_ADDR    (0x80)
#define NAPLES_CPLD_REV_REG_ADDR   (0x0)
#define NAPLES_CPLD_ID             (0x12)

diag_ret_e cpld_test(test_mode_e mode, int argc, char* argv[]);

} // namespace diag
} // namespace platform
} // namespace sdk

#endif //__CPLD_TEST_H__

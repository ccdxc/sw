// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __XCVR_QSFP_HPP__
#define __XCVR_QSFP_HPP__

#include "lib/pal/pal.hpp"

namespace sdk {
namespace platform {

#define QSFP_TX_DISABLE_OFFSET 0x56

using sdk::lib::qsfp_page_t;

sdk_ret_t qsfp_read_page (int port, qsfp_page_t pgno, int offset,
                          int num_bytes, uint8_t *data);
sdk_ret_t qsfp_enable (int port, bool enable, uint8_t mask);

} // namespace platform
} // namespace sdk

#endif

// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __XCVR_QSFP_HPP__
#define __XCVR_QSFP_HPP__

#include "lib/pal/pal.hpp"

namespace sdk {
namespace platform {

#define QSFP_OFFSET_TX_DISABLE                0x56
#define QSFP_OFFSET_LENGTH_CU                 146 // 0x92
#define QSFP_OFFSET_ETH_COMPLIANCE_CODES      131
#define QSFP_OFFSET_EXT_SPEC_COMPLIANCE_CODES 192

using sdk::lib::qsfp_page_t;

sdk_ret_t qsfp_read_page(int port, qsfp_page_t pgno, int offset,
                         int num_bytes, uint8_t *data);
sdk_ret_t qsfp_enable(int port, bool enable, uint8_t mask);
sdk_ret_t qsfp_sprom_parse(int port, uint8_t *data);

} // namespace platform
} // namespace sdk

#endif

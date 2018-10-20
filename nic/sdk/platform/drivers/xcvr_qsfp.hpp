// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __XCVR_QSFP_HPP__
#define __XCVR_QSFP_HPP__

namespace sdk {
namespace platform {

sdk_ret_t qsfp_write(int addr, int offset, uint8_t data, int num_bytes, int port);

sdk_ret_t qsfp_read(int addr, int offset, int num_bytes, uint8_t *data, int port);

sdk_ret_t qsfp_read_page(int addr, int pgno, int num_bytes, int port, uint8_t *data);

bool is_xcvr_qsfp(uint8_t *data);

} // namespace platform
} // namespace sdk

#endif

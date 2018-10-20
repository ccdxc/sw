// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __XCVR_SFP_HPP__
#define __XCVR_SFP_HPP__

namespace sdk {
namespace platform {

sdk_ret_t sfp_write(int addr, int offset, uint8_t data, int num_bytes, int port);

sdk_ret_t sfp_read(int addr, int offset, int num_bytes, uint8_t *data, int port);

sdk_ret_t sfp_read_page(int addr, int pgno, int num_bytes, int port, uint8_t *data);

} // namespace platform
} // namespace sdk

#endif

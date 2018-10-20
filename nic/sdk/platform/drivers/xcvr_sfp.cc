// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "include/sdk/thread.hpp"
#include "include/sdk/periodic.hpp"
#include "platform/drivers/xcvr_sfp.hpp"

namespace sdk {
namespace platform {

sdk_ret_t
sfp_write (int addr, int offset, uint8_t data, int num_bytes, int port) {
    return SDK_RET_OK;
}

sdk_ret_t
sfp_read (int addr, int offset, int num_bytes, uint8_t *data, int port) {
    return SDK_RET_OK;
}

sdk_ret_t
sfp_read_page (int addr, int pgno, int num_bytes, int port, uint8_t *data) {
    sdk_ret_t ret = SDK_RET_OK;

    // This needs to be changed for SFP
    ret = sfp_write(addr, 127, pgno, 1, port);
    if (ret == SDK_RET_OK) {
        ret = sfp_read(addr, 0, num_bytes, data, port);
    }

    return ret;
}

} // namespace platform
} // namespace sdk

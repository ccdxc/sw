// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "include/sdk/thread.hpp"
#include "include/sdk/periodic.hpp"
#include "platform/drivers/xcvr_qsfp.hpp"
#include "lib/pal/pal.hpp"

namespace sdk {
namespace platform {

using sdk::lib::pal_ret_t;
using sdk::lib::qsfp_page_t;

sdk_ret_t
qsfp_read_page (int port, qsfp_page_t pgno, int offset, int num_bytes, uint8_t *data) {
    pal_ret_t ret = sdk::lib::pal_qsfp_read(data, num_bytes,
                                            offset, pgno,
                                            1, port + 1);
    if (ret == sdk::lib::PAL_RET_OK) {
        return SDK_RET_OK;
    } else {
        return SDK_RET_ERR;
    }
}

} // namespace platform
} // namespace sdk

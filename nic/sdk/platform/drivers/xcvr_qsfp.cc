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
qsfp_read_page (int port, qsfp_page_t pgno, int offset,
                int num_bytes, uint8_t *data) {
    pal_ret_t ret = sdk::lib::pal_qsfp_read(data, num_bytes,
                                            offset, pgno,
                                            1, port + 1);
    if (ret == sdk::lib::PAL_RET_OK) {
        return SDK_RET_OK;
    } else {
        return SDK_RET_ERR;
    }
}

sdk_ret_t
qsfp_write_page (int port, qsfp_page_t pgno, int offset,
                 int num_bytes, uint8_t *data) {
    pal_ret_t ret = sdk::lib::pal_qsfp_write(data, num_bytes,
                                             offset, pgno,
                                             1, port + 1);
    if (ret == sdk::lib::PAL_RET_OK) {
        return SDK_RET_OK;
    } else {
        return SDK_RET_ERR;
    }
}

sdk_ret_t
qsfp_enable (int port, bool enable, uint8_t mask)
{
    uint8_t   data    = 0x0;
    sdk_ret_t sdk_ret = SDK_RET_OK;

    sdk_ret = qsfp_read_page(port, qsfp_page_t::QSFP_PAGE_LOW,
                             QSFP_TX_DISABLE_OFFSET, 1, &data);
    if (sdk_ret != SDK_RET_OK) {
        SDK_TRACE_ERR ("qsfp_port: %d, failed to read offset: 0x%x, page: %d",
                       port+1, QSFP_TX_DISABLE_OFFSET,
                       qsfp_page_t::QSFP_PAGE_LOW);
        return sdk_ret;
    }

    // clear the mask bits
    data = data & ~mask;

    // set the bits to disable TX
    if (enable == false) {
        data = data | mask;
    }

    qsfp_write_page(port, qsfp_page_t::QSFP_PAGE_LOW,
                    QSFP_TX_DISABLE_OFFSET, 1, &data);
    if (sdk_ret != SDK_RET_OK) {
        SDK_TRACE_ERR ("qsfp_port: %d, failed to write offset: 0x%x, page: %d",
                       port+1, QSFP_TX_DISABLE_OFFSET,
                       qsfp_page_t::QSFP_PAGE_LOW);
    }

    return sdk_ret;
}

} // namespace platform
} // namespace sdk

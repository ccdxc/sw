// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "include/sdk/thread.hpp"
#include "include/sdk/periodic.hpp"
#include "include/sdk/linkmgr.hpp"
#include "platform/drivers/xcvr.hpp"
#include "platform/drivers/xcvr_qsfp.hpp"
#include "lib/pal/pal.hpp"

namespace sdk {
namespace platform {

using sdk::linkmgr::xcvr_event_notify_t;
using sdk::types::xcvr_type_t;
using sdk::types::xcvr_cable_type_t;
using sdk::types::xcvr_state_t;
using sdk::types::xcvr_pid_t;
using sdk::types::xcvr_type_t;

xcvr_t g_xcvr[XCVR_MAX_PORTS];
xcvr_event_notify_t g_xcvr_notify_cb;

xcvr_type_t
xcvr_get_type (uint8_t first_byte) {
    switch (first_byte) {
    case 0x3:
        return xcvr_type_t::XCVR_TYPE_SFP;
    case 0xc:
        return xcvr_type_t::XCVR_TYPE_QSFP;
    case 0x11:
        return xcvr_type_t::XCVR_TYPE_QSFP28;
    default:
        break;
    }

    return xcvr_type_t::XCVR_TYPE_UNKNOWN;
}

static bool
xcvr_state_change (int port) {
    // Detect xcvr and set state
    if (sdk::lib::pal_is_qsfp_port_present(port + 1)) {
        if (xcvr_state(port) == xcvr_state_t::XCVR_REMOVED) {
            SDK_TRACE_DEBUG("Xcvr port %d state changed from %d to %d", port + 1, xcvr_state(port), xcvr_state_t::XCVR_INSERTED);
            xcvr_set_state(port, xcvr_state_t::XCVR_INSERTED);
            return true;
        }
    } else {
        if (xcvr_state(port) != xcvr_state_t::XCVR_REMOVED) {
            SDK_TRACE_DEBUG("Xcvr port %d state changed from %d to %d", port + 1, xcvr_state(port), xcvr_state_t::XCVR_REMOVED);
            xcvr_set_state(port, xcvr_state_t::XCVR_REMOVED);
            return true;
        }
    }

    return false;
}

// Send notification for xcvr_state
static sdk_ret_t
xcvr_send_notification (int port) {
    if (g_xcvr_notify_cb) {
        switch (xcvr_state(port)) {
        case xcvr_state_t::XCVR_REMOVED:
            g_xcvr_notify_cb(port, xcvr_state(port), xcvr_pid_t::XCVR_PID_UNKNOWN);
            break;
        case xcvr_state_t::XCVR_INSERTED:
            g_xcvr_notify_cb(port, xcvr_state(port), xcvr_pid_t::XCVR_PID_UNKNOWN);
            break;
        case xcvr_state_t::XCVR_SPROM_READ:
            g_xcvr_notify_cb(port, xcvr_state(port), xcvr_pid_t::XCVR_PID_QSFP_100G_CR4);
            break;
        case xcvr_state_t::XCVR_SPROM_READ_ERR:
            g_xcvr_notify_cb(port, xcvr_state(port), xcvr_pid_t::XCVR_PID_UNKNOWN);
            break;
        default:
            break;
        }
    }

    return SDK_RET_OK;
}

static void
xcvr_hex_dump (int port, uint8_t *buffer) {
    char dump[100];
    int len = 0;

    SDK_TRACE_DEBUG("Xcvr port %d Hex dump", port + 1);

    for (int i = 0; i < 256; i ++) {
        if (!(i % 16)) {
            len += sprintf(dump + len, "%#04x: ", i);
        }
        len += sprintf(dump + len, "%02x ", buffer[i]);
        if ((i % 16) == 15) {
            SDK_TRACE_DEBUG("%s", dump);
            len = 0;
        }
    }
}

static uint8_t *
xcvr_sprom_read (int port, bool cached) {
    sdk_ret_t ret = SDK_RET_OK;
    uint8_t   *buffer = NULL;

    if ((xcvr_state(port) == xcvr_state_t::XCVR_SPROM_READ) && cached) {
        return xcvr_cache(port);
    }

    buffer = xcvr_cache(port);

    // Read first 128 bytes
    ret = qsfp_read_page(port, sdk::lib::QSFP_PAGE_LOW, 0, XCVR_SPROM_READ_SIZE, &buffer[0]);
    if (ret != SDK_RET_OK) {
        return NULL;
    }

    switch (xcvr_get_type(buffer[0])) {
    case xcvr_type_t::XCVR_TYPE_QSFP:
        xcvr_set_type(port, xcvr_type_t::XCVR_TYPE_QSFP);
        ret = qsfp_read_page(port, sdk::lib::QSFP_PAGE_HIGH0, 0, XCVR_SPROM_READ_SIZE, &buffer[XCVR_SPROM_READ_SIZE]);
        if (ret != SDK_RET_OK) {
            return NULL;
        }
        break;
    case xcvr_type_t::XCVR_TYPE_QSFP28:
        xcvr_set_type(port, xcvr_type_t::XCVR_TYPE_QSFP28);
        ret = qsfp_read_page(port, sdk::lib::QSFP_PAGE_HIGH0, 0, XCVR_SPROM_READ_SIZE, &buffer[XCVR_SPROM_READ_SIZE]);
        if (ret != SDK_RET_OK) {
            return NULL;
        }
        break;
    case xcvr_type_t::XCVR_TYPE_SFP:
        xcvr_set_type(port, xcvr_type_t::XCVR_TYPE_SFP);
        ret = qsfp_read_page(port, sdk::lib::QSFP_PAGE_LOW, XCVR_SPROM_READ_SIZE, XCVR_SPROM_READ_SIZE, &buffer[XCVR_SPROM_READ_SIZE]);
        if (ret != SDK_RET_OK) {
            return NULL;
        }
        break;
    default:
        SDK_TRACE_DEBUG("Xcvr port %d Xcvr type unknown. First byte is %#x", port + 1, buffer[0]);
        return NULL;
    }

    xcvr_hex_dump(port, buffer);
    
    return buffer;
}

static sdk_ret_t
xcvr_sprom_parse (uint8_t *data) {
    return SDK_RET_OK;
}

void
xcvr_init (xcvr_event_notify_t xcvr_notify_cb)
{
    g_xcvr_notify_cb = xcvr_notify_cb;
}

void
xcvr_poll_timer (void)
{
    bool state_change = false;
    uint8_t  *data = NULL;

    for (int port = 0; port < XCVR_MAX_PORTS; port ++) {
        // Detect xcvr and state change
        state_change = xcvr_state_change(port);

        // If xcvr state change send notification
        if (state_change) {
            xcvr_send_notification(port);
        }

        switch (xcvr_state(port)) {
        case xcvr_state_t::XCVR_INSERTED:
            if (!state_change) {
                // This should not really happen.
                // When xcvr is inserted we proceed to
                // sprom read and the state will not
                // remain as xcvr_state_t::XCVR_INSERTED.
                // Continue to next port
                break; 
            }
            // Bring qsfp port out of reset
            sdk::lib::pal_qsfp_set_port(port + 1); // 1-indexed
            // Reset low-powered mode
            sdk::lib::pal_qsfp_reset_low_power_mode(port + 1);
            // Set state as SPROM_PENDING
            xcvr_set_state(port, xcvr_state_t::XCVR_SPROM_PENDING);
            // fall-through
        case xcvr_state_t::XCVR_SPROM_PENDING:
            // Read sprom
            data = xcvr_sprom_read(port, false);
            if (data == NULL) {
                if (!xcvr_sprom_read_count_inc(port)) {
                    // Reached max read attempts
                    xcvr_set_state(port, xcvr_state_t::XCVR_SPROM_READ_ERR);
                    xcvr_send_notification(port);
                    SDK_TRACE_DEBUG("Xcvr port %d sprom read failed after 10 retries", port + 1);
                } else {
                    SDK_TRACE_DEBUG("Xcvr port %d sprom read error", port + 1);
                }
                break;
            }

            // Parse sprom
            xcvr_sprom_parse(data);

            // Send sprom_read notification
            xcvr_set_state(port, xcvr_state_t::XCVR_SPROM_READ);
            SDK_TRACE_DEBUG("Xcvr port %d sprom read successful", port + 1);
            xcvr_send_notification(port);
            break;
        case xcvr_state_t::XCVR_REMOVED:
            xcvr_reset(port);
            break;
        case xcvr_state_t::XCVR_SPROM_READ:
        case xcvr_state_t::XCVR_SPROM_READ_ERR:
            break;
        }
    }

    return;
}

} // namespace platform

} // namespace sdk

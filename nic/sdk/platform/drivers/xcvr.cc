// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "lib/thread/thread.hpp"
#include "lib/periodic/periodic.hpp"
#include "linkmgr/linkmgr.hpp"
#include "platform/drivers/xcvr.hpp"
#include "platform/drivers/xcvr_qsfp.hpp"
#include "platform/drivers/xcvr_sfp.hpp"
#include "lib/pal/pal.hpp"

namespace sdk {
namespace platform {

using sdk::types::xcvr_pid_t;
using sdk::linkmgr::port_args_t;

xcvr_t g_xcvr[XCVR_MAX_PORTS];
xcvr_event_notify_t g_xcvr_notify_cb;

// global var to enable/disable transceiver checks for links
bool xcvr_valid_enable = true;

/**
 * @brief   Transceiver data to be exported outside HAL
 */
static void
xcvr_sprom_get (int port, void *xcvr_sprom)
{
    uint8_t *buffer = xcvr_cache(port);

    switch (xcvr_type(port)) {
    case xcvr_type_t::XCVR_TYPE_QSFP:
    case xcvr_type_t::XCVR_TYPE_QSFP28:
        buffer = &buffer[128];
        break;

    case xcvr_type_t::XCVR_TYPE_SFP:
    default:
        break;
    }

    memcpy(xcvr_sprom, buffer, XCVR_SPROM_SIZE);

    return;

#if 0
    sfp_sprom_data_t  *sfp_sprom  = NULL;
    qsfp_sprom_data_t *qsfp_sprom = NULL;
    uint8_t           *buffer     = NULL;

    switch (xcvr_type(port)) {
        case xcvr_type_t::XCVR_TYPE_SFP:
            sfp_sprom = (sfp_sprom_data_t*)xcvr_cache(port);

            xcvr_sprom->length_smf_km = sfp_sprom->length_smf_km;
            xcvr_sprom->length_smf    = sfp_sprom->length_smf;
            xcvr_sprom->length_om1    = sfp_sprom->length_om1;
            xcvr_sprom->length_om2    = sfp_sprom->length_om2;
            xcvr_sprom->length_om3    = sfp_sprom->length_om3;
            xcvr_sprom->length_dac    = sfp_sprom->length_dac;

            memcpy(xcvr_sprom->vendor_name, sfp_sprom->vendor_name, 16);
            memcpy(xcvr_sprom->vendor_pn,   sfp_sprom->vendor_pn,   16);
            memcpy(xcvr_sprom->vendor_rev,  sfp_sprom->vendor_rev,  4);
            memcpy(xcvr_sprom->vendor_sn,   sfp_sprom->vendor_sn,   16);

            break;

        case xcvr_type_t::XCVR_TYPE_QSFP:
        case xcvr_type_t::XCVR_TYPE_QSFP28:
            buffer = xcvr_cache(port);
            qsfp_sprom = (qsfp_sprom_data_t*)&buffer[128];

            xcvr_sprom->length_smf_km = qsfp_sprom->length_smf_km;
            xcvr_sprom->length_om1    = qsfp_sprom->length_om1;
            xcvr_sprom->length_om2    = qsfp_sprom->length_om2;
            xcvr_sprom->length_om3    = qsfp_sprom->length_om3;
            xcvr_sprom->length_dac    = qsfp_sprom->length_dac;

            memcpy(xcvr_sprom->vendor_name, qsfp_sprom->vendor_name, 16);
            memcpy(xcvr_sprom->vendor_pn,   qsfp_sprom->vendor_pn,   16);
            memcpy(xcvr_sprom->vendor_rev,  qsfp_sprom->vendor_rev,  2);
            memcpy(xcvr_sprom->vendor_sn,   qsfp_sprom->vendor_sn,   16);

            break;

        default:
            break;
    }
#endif
}

sdk_ret_t
xcvr_get (int port, xcvr_event_info_t *xcvr_event_info)
{
    // Front panel ports are 1 based
    xcvr_event_info->phy_port    = port + 1;
    xcvr_event_info->state        = xcvr_state(port);
    xcvr_event_info->pid          = xcvr_pid(port);
    xcvr_event_info->cable_type   = cable_type(port);
    xcvr_event_info->port_an_args = xcvr_get_an_args(port);

    xcvr_sprom_get (port, xcvr_event_info->xcvr_sprom);

    return SDK_RET_OK;
}

xcvr_type_t
xcvr_get_type (uint8_t first_byte) {
    switch (first_byte) {
    case 0x3:
        return xcvr_type_t::XCVR_TYPE_SFP;
    case 0xc:
    case 0xd:
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
    bool present = sdk::lib::pal_is_qsfp_port_present(port + 1);
    bool debounce = xcvr_get_debounce(port);

    // TODO For testing
#if 0
    int phy_port = 0;
    int qsfp_present = 0;
    FILE *d_fp = fopen("/qsfp_presence", "r");
    if (d_fp) {
        fscanf(d_fp, "%d %d", &phy_port, &qsfp_present);
        if (phy_port == port + 1) {
            present = qsfp_present == 1? true : false;
        }
        fclose(d_fp);
    }
#endif

    if (debounce) {
        // debounce flag set. If state change proceed with steps else reset debounce
        if (present) {
            if (xcvr_state(port) == xcvr_state_t::XCVR_REMOVED) {
                SDK_TRACE_DEBUG("Xcvr port %d state changed from %d to %d",
                                port + 1,
                                xcvr_state(port),
                                xcvr_state_t::XCVR_INSERTED);
                xcvr_set_state(port, xcvr_state_t::XCVR_INSERTED);
                xcvr_reset_debounce(port);
                return true;
            }
        } else {
            if (xcvr_state(port) != xcvr_state_t::XCVR_REMOVED) {
                SDK_TRACE_DEBUG("Xcvr port %d state changed from %d to %d",
                                port + 1,
                                xcvr_state(port),
                                xcvr_state_t::XCVR_REMOVED);
                xcvr_set_state(port, xcvr_state_t::XCVR_REMOVED);
                xcvr_reset_debounce(port);
                return true;
            }
        }
        xcvr_reset_debounce(port);
    } else {
        // debounce not set. If state change set debounce
        if (present) {
            if (xcvr_state(port) == xcvr_state_t::XCVR_REMOVED) {
                xcvr_set_debounce(port);
            }
        } else {
            if (xcvr_state(port) != xcvr_state_t::XCVR_REMOVED) {
                xcvr_set_debounce(port);
            }
        }
    }
    return false;
}

// Send notification for xcvr_state
static sdk_ret_t
xcvr_send_notification (int port)
{
    xcvr_event_info_t xcvr_event_info;

    memset(&xcvr_event_info, 0, sizeof(xcvr_event_info_t));

    xcvr_get(port, &xcvr_event_info);

    if (g_xcvr_notify_cb) {
        switch (xcvr_state(port)) {
        case xcvr_state_t::XCVR_SPROM_READ:
            xcvr_event_info.pid = xcvr_pid(port);
            break;

        default:
            xcvr_event_info.pid = xcvr_pid_t::XCVR_PID_UNKNOWN;
            break;
        }
        g_xcvr_notify_cb(&xcvr_event_info);
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
    ret = qsfp_read_page(port, sdk::lib::QSFP_PAGE_LOW, 0,
                         XCVR_SPROM_READ_SIZE, &buffer[0]);
    if (ret != SDK_RET_OK) {
        return NULL;
    }

    switch (xcvr_get_type(buffer[0])) {
    case xcvr_type_t::XCVR_TYPE_QSFP:
        xcvr_set_type(port, xcvr_type_t::XCVR_TYPE_QSFP);
        ret = qsfp_read_page(port, sdk::lib::QSFP_PAGE_HIGH0,
                             XCVR_SPROM_READ_SIZE, XCVR_SPROM_READ_SIZE,
                             &buffer[XCVR_SPROM_READ_SIZE]);
        if (ret != SDK_RET_OK) {
            return NULL;
        }
        break;
    case xcvr_type_t::XCVR_TYPE_QSFP28:
        xcvr_set_type(port, xcvr_type_t::XCVR_TYPE_QSFP28);
        ret = qsfp_read_page(port, sdk::lib::QSFP_PAGE_HIGH0,
                             XCVR_SPROM_READ_SIZE, XCVR_SPROM_READ_SIZE,
                             &buffer[XCVR_SPROM_READ_SIZE]);
        if (ret != SDK_RET_OK) {
            return NULL;
        }
        break;
    case xcvr_type_t::XCVR_TYPE_SFP:
        xcvr_set_type(port, xcvr_type_t::XCVR_TYPE_SFP);
        ret = qsfp_read_page(port, sdk::lib::QSFP_PAGE_LOW,
                             XCVR_SPROM_READ_SIZE, XCVR_SPROM_READ_SIZE,
                             &buffer[XCVR_SPROM_READ_SIZE]);
        if (ret != SDK_RET_OK) {
            return NULL;
        }
        break;
    default:
        SDK_TRACE_DEBUG("Xcvr port %d Xcvr type unknown. First byte is %#x",
                        port + 1, buffer[0]);
        return NULL;
    }

    xcvr_hex_dump(port, buffer);

    return buffer;
}

static sdk_ret_t
xcvr_sprom_parse (int port, uint8_t *data) {
    switch (xcvr_type(port)) {
    case xcvr_type_t::XCVR_TYPE_QSFP:
    case xcvr_type_t::XCVR_TYPE_QSFP28:
        qsfp_sprom_parse(port, data);
        break;

    case xcvr_type_t::XCVR_TYPE_SFP:
        sfp_sprom_parse(port, data);
        break;

    default:
        break;
    }

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
                    SDK_TRACE_DEBUG("Xcvr port %d sprom read failed after "
                                    "10 retries", port + 1);
                } else {
                    SDK_TRACE_DEBUG("Xcvr port %d sprom read error", port + 1);
                }
                break;
            }

            // Parse sprom
            xcvr_sprom_parse(port, data);

            // Send sprom_read notification
            xcvr_set_state(port, xcvr_state_t::XCVR_SPROM_READ);
            SDK_TRACE_DEBUG("Xcvr port %d sprom read successful", port + 1);
            xcvr_send_notification(port);
            break;
        case xcvr_state_t::XCVR_REMOVED:
            if (state_change) {
                xcvr_reset(port);
            }
            break;
        case xcvr_state_t::XCVR_SPROM_READ:
            // TODO add function call here
            // sdk::lib::pal_cpld_write_qsfp_temp(50,port);
            break;
        case xcvr_state_t::XCVR_SPROM_READ_ERR:
            break;
        }
    }

    return;
}

bool
xcvr_valid (int port)
{
    // if the xcvr valid check is disable, return true
    if (!xcvr_valid_check_enabled()) {
        return true;
    }
    return xcvr_state(port) == xcvr_state_t::XCVR_SPROM_READ? true : false;
}

sdk_ret_t
xcvr_enable(int port, bool enable, uint8_t mask)
{
    switch(xcvr_type(port)) {
    case xcvr_type_t::XCVR_TYPE_QSFP28:
    case xcvr_type_t::XCVR_TYPE_QSFP:
        return qsfp_enable(port, enable, mask);

    default:
        break;
    }

    return SDK_RET_OK;
}

} // namespace platform
} // namespace sdk

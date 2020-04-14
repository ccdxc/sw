//
//  {C} Copyright 2020 Pensando Systems Inc. All rights reserved.
//

#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <vlib/vlib.h>

static vlib_log_class_t vpp_upg_log_fcl;

// wrapper over vlib_log callable from C++
int
upg_log_notice (const char *fmt, ...)
{
    va_list va;
    int ret;
    static char buf[128];

    va_start(va, fmt);
    ret = vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    vlib_log(VLIB_LOG_LEVEL_NOTICE, vpp_upg_log_fcl, buf);
    return ret;
}

// wrapper over vlib_log callable from C++
int
upg_log_error (const char *fmt, ...) {
    int ret;
    va_list va;
    static char buf[128];

    va_start(va, fmt);
    ret = vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    vlib_log(VLIB_LOG_LEVEL_ERR, vpp_upg_log_fcl, buf);
    return ret;
}

// Initialize the upgrade log facility
void
upg_log_facility_init (void)
{
    vpp_upg_log_fcl = vlib_log_register_class("vpp-upgrade", 0);

    assert(vpp_upg_log_fcl != 0);
    upg_log_notice("Upgrade log facility Initialized");
}


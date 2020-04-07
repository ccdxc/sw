//
//  {C} Copyright 2020 Pensando Systems Inc. All rights reserved.
//

#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <vlib/vlib.h>
#include <vnet/vnet.h>
#include <vnet/plugin/plugin.h>
#include <vppinfra/file.h>

// logging class
static vlib_log_class_t pds_flow_log = 0;

// wrapper over vlib_log callable from C++
int
flow_log_notice (const char *fmt, ...) {
    va_list va;
    int ret;
    static char buf[128];

    va_start(va, fmt);
    ret = vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    vlib_log(VLIB_LOG_LEVEL_NOTICE, pds_flow_log, buf);
    return ret;
}

// wrapper over vlib_log callable from C++
int
flow_log_error (const char *fmt, ...) {
    int ret;
    va_list va;
    static char buf[128];

    va_start(va, fmt);
    ret = vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    vlib_log(VLIB_LOG_LEVEL_ERR, pds_flow_log, buf);
    return ret;
}

// initialize the flow log class
int
pds_vpp_flow_log_init (void)
{
    pds_flow_log = vlib_log_register_class("pds-flow", 0);

    assert(pds_flow_log != 0);
    return 0;
}

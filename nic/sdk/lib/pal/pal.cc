// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <sys/mman.h>
#include <string.h>
#include "lib/pal/pal.hpp"
#include "lib/pal/pal_internal.hpp"

namespace sdk {
namespace lib {

pal_info_t   gl_pal_info;
static bool g_pal_init_done = false;

static pal_ret_t
pal_init_cfg (platform_type_t platform_type)
{
    memset(&gl_pal_info, 0, sizeof(gl_pal_info));
    gl_pal_info.platform_type = platform_type;
    return pal_default_init();
}

pal_ret_t
pal_init (platform_type_t platform_type)
{
    pal_ret_t rv;

    // on multithreaded app only the master thread init is considered
    if (g_pal_init_done) {
        return PAL_RET_OK;
    }

    if (pal_init_cfg(platform_type) != PAL_RET_OK) {
        return PAL_RET_NOK;
    }

    switch(platform_type) {
    case platform_type_t::PLATFORM_TYPE_HW:
    case platform_type_t::PLATFORM_TYPE_HAPS:
        rv = pal_hw_init();
        break;

    case platform_type_t::PLATFORM_TYPE_SIM:
    case platform_type_t::PLATFORM_TYPE_ZEBU:
        SDK_TRACE_DEBUG("Initializing PAL in SIM mode");
        rv = pal_init_sim();
        break;

    case platform_type_t::PLATFORM_TYPE_MOCK:
        SDK_TRACE_DEBUG("Initializing PAL in MOCK mode");
        rv = pal_mock_init();
        break;

    default:
        rv = PAL_RET_NOK;
        break;
    }

    if (unlikely(rv != PAL_RET_OK)) {
        SDK_TRACE_ERR("Failed to initialize PAL for platform type %d, err %d",
                      platform_type, rv);
        return rv;
    }
    g_pal_init_done = true;
    return PAL_RET_OK;
}

pal_ret_t
pal_teardown (platform_type_t platform_type)
{
    pal_ret_t rv;

    switch (platform_type) {
    case platform_type_t::PLATFORM_TYPE_SIM:
        SDK_TRACE_DEBUG("Teardown PAL in SIM mode");
        rv = pal_teardown_sim();
        break;

    default:
        rv = PAL_RET_NOK;
        break;
    }

    if (unlikely(rv != PAL_RET_OK)) {
        SDK_TRACE_ERR("Failed to teardown PAL for platform type %d, err %d",
                      platform_type, rv);
        return rv;
    }
    g_pal_init_done = false;
    return PAL_RET_OK;
}

}    // namespace lib
}    // namespace sdk

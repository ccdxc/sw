// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <sys/mman.h>
#include <string.h>
#include "lib/pal/pal.hpp"
#include "lib/pal/pal_internal.hpp"

namespace sdk {
namespace lib {

pal_info_t   gl_pal_info;

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
    if(pal_init_cfg(platform_type) != PAL_RET_OK) {
	return PAL_RET_NOK;
    }

    switch(platform_type) {
    case platform_type_t::PLATFORM_TYPE_HW:
    case platform_type_t::PLATFORM_TYPE_HAPS:
        SDK_TRACE_DEBUG("Initializing PAL in HW mode\n");
        return pal_hw_init();

    case platform_type_t::PLATFORM_TYPE_SIM:
    case platform_type_t::PLATFORM_TYPE_ZEBU:
        SDK_TRACE_DEBUG("Initializing PAL in SIM mode\n");
        return pal_init_sim();

    case platform_type_t::PLATFORM_TYPE_MOCK:
        SDK_TRACE_DEBUG("Initializing PAL in MOCK mode\n");
        return pal_mock_init();

    default:
        break;
    }

    return PAL_RET_OK;
}

}    // namespace lib
}    // namespace sdk

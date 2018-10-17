// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <sys/mman.h>
#include <string.h>
#include "lib/pal/pal.hpp"
#include "lib/pal/pal_internal.hpp"

namespace sdk {
namespace lib {

pal_info_t   gl_pal_info;

static pal_ret_t
pal_init_cfg (sdk::types::platform_type_t platform_type)
{
    memset(&gl_pal_info, 0, sizeof(gl_pal_info));
    gl_pal_info.platform_type = platform_type;
    return PAL_RET_OK;
}

pal_ret_t
pal_init (sdk::types::platform_type_t platform_type)
{
    pal_init_cfg(platform_type);

    switch(platform_type) {
    case sdk::types::platform_type_t::PLATFORM_TYPE_HW:
    case sdk::types::platform_type_t::PLATFORM_TYPE_HAPS:
        SDK_TRACE_DEBUG("Initializing PAL in HW mode\n");
        return pal_hw_init();

    case sdk::types::platform_type_t::PLATFORM_TYPE_SIM:
    case sdk::types::platform_type_t::PLATFORM_TYPE_ZEBU:
        SDK_TRACE_DEBUG("Initializing PAL in SIM mode\n");
        return pal_init_sim();

    case sdk::types::platform_type_t::PLATFORM_TYPE_MOCK:
        SDK_TRACE_DEBUG("Initializing PAL in MOCK mode\n");
        return pal_mock_init();

    default:
        break;
    }

    return PAL_RET_OK;
}

}    // namespace lib
}    // namespace sdk

//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// base SDK header file that goes into rest of the SDK
//------------------------------------------------------------------------------

#ifndef __SDK_PLATFORM_HPP__
#define __SDK_PLATFORM_HPP__

#include "include/sdk/base.hpp"

#define SDK_INVALID_HBM_ADDRESS    ((uint64_t) 0xFFFFFFFFFFFFFFFF)

#define CACHE_LINE_SIZE                       64
#define CACHE_LINE_SIZE_SHIFT                  6
#define CACHE_LINE_SIZE_MASK                   (CACHE_LINE_SIZE - 1)

namespace sdk {
namespace platform {

#define PLATFORM_TYPE(ENTRY)                                                \
    ENTRY(PLATFORM_TYPE_NONE,       0, "PLATFORM_TYPE_NONE")                \
    ENTRY(PLATFORM_TYPE_SIM,        1, "PLATFORM_TYPE_SIM")                 \
    ENTRY(PLATFORM_TYPE_HAPS,       2, "PLATFORM_TYPE_HAPS")                \
    ENTRY(PLATFORM_TYPE_HW,         3, "PLATFORM_TYPE_HW")                  \
    ENTRY(PLATFORM_TYPE_MOCK,       4, "PLATFORM_TYPE_MOCK")                \
    ENTRY(PLATFORM_TYPE_ZEBU,       5, "PLATFORM_TYPE_ZEBU")                \
    ENTRY(PLATFORM_TYPE_RTL,        6, "PLATFORM_TYPE_RTL")

SDK_DEFINE_ENUM(platform_type_t, PLATFORM_TYPE)
SDK_DEFINE_ENUM_TO_STR(platform_type_t, PLATFORM_TYPE)
SDK_DEFINE_MAP_EXTERN(platform_type_t, PLATFORM_TYPE)
// #undef PLATFORM_TYPE

#define ASIC_TYPE(ENTRY)                                                    \
    ENTRY(SDK_ASIC_TYPE_NONE,       0, "SDK_ASIC_TYPE_NONE")                \
    ENTRY(SDK_ASIC_TYPE_CAPRI,      1, "SDK_ASIC_TYPE_CAPRI")               \
    ENTRY(SDK_ASIC_TYPE_ELBA,       2, "SDK_ASIC_TYPE_ELBA")

SDK_DEFINE_ENUM(asic_type_t, ASIC_TYPE)
SDK_DEFINE_ENUM_TO_STR(asic_type_t, ASIC_TYPE)
SDK_DEFINE_MAP_EXTERN(asic_type_t, ASIC_TYPE)
// #undef ASIC_TYPE

#define UPGRADE_MODE(ENTRY)                                               \
    ENTRY(UPGRADE_MODE_NONE,       0, "UPGRADE_MODE_NONE")                \
    ENTRY(UPGRADE_MODE_GRACEFUL,   1, "UPGRADE_MODE_GRACEFUL")            \
    ENTRY(UPGRADE_MODE_HITLESS,    2, "UPGRADE_MODE_HITLESS")             \

SDK_DEFINE_ENUM(upg_mode_t,        UPGRADE_MODE)
SDK_DEFINE_ENUM_TO_STR(upg_mode_t, UPGRADE_MODE)
SDK_DEFINE_MAP_EXTERN(upg_mode_t,  UPGRADE_MODE)
// #undef UPGRADE_MODE

static inline bool
upgrade_mode_none (upg_mode_t type)
{
    return type == UPGRADE_MODE_NONE;
}

static inline bool
upgrade_mode_hitless (upg_mode_t type)
{
    return type == UPGRADE_MODE_HITLESS;
}

static inline bool
upgrade_mode_graceful (upg_mode_t type)
{
    return type == UPGRADE_MODE_GRACEFUL;
}

}    // namespace platform
}    // namespace sdk

using sdk::platform::platform_type_t;
using sdk::platform::asic_type_t;
using sdk::platform::upg_mode_t;

#endif    // __SDK_PLATFORM_HPP__


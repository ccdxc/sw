//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// base SDK header file that goes into rest of the SDK
//------------------------------------------------------------------------------

#ifndef __SDK_PLATFORM_HPP__
#define __SDK_PLATFORM_HPP__

#define SDK_INVALID_HBM_ADDRESS    ((uint64_t) 0xFFFFFFFFFFFFFFFF)

#define CACHE_LINE_SIZE                       64
#define CACHE_LINE_SIZE_SHIFT                  6
#define CACHE_LINE_SIZE_MASK                   (CACHE_LINE_SIZE - 1)

namespace sdk {
namespace platform {
enum class platform_type_t {
    PLATFORM_TYPE_NONE = 0,
    PLATFORM_TYPE_SIM  = 1,
    PLATFORM_TYPE_HAPS = 2,
    PLATFORM_TYPE_HW   = 3,
    PLATFORM_TYPE_MOCK = 4,
    PLATFORM_TYPE_ZEBU = 5,
    PLATFORM_TYPE_RTL = 6,
};

enum class asic_type_t {
    SDK_ASIC_TYPE_NONE,
    SDK_ASIC_TYPE_CAPRI,
};

}    // namespace platform
}    // namespace sdk

using sdk::platform::platform_type_t;
using sdk::platform::asic_type_t;

#endif    // __SDK_PLATFORM_HPP__


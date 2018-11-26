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

}    // namespace platform
}    // namespace sdk

#endif    // __SDK_PLATFORM_HPP__


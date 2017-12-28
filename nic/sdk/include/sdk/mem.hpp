//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __SDK_MEM_HPP__
#define __SDK_MEM_HPP__

#include "sdk/base.hpp"

namespace sdk {

#define SDK_MALLOC(alloc_id, size)    malloc(size)
#define SDK_CALLOC(alloc_id, size)    calloc(1, (size))
#define SDK_FREE(alloc_id, ptr)       ::free(ptr)

}    // namespace sdk

#endif    // __SDK_MEM_HPP__


//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __CORE_HPP__
#define __CORE_HPP__

namespace core {

#define CALLOC(id, size) calloc(size, size);
#define FREE(id, mem)    free(mem)

}    // namespace core

#endif    // __CORE_HPP__

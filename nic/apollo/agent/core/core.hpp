//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __CORE_HPP__
#define __CORE_HPP__

#include "nic/apollo/core/core.hpp"

namespace core {

enum {
    THREAD_ID_AGENT_NONE = THREAD_ID_MAX,
    THREAD_ID_AGENT_CMD_SERVER,
    THREAD_ID_AGENT_ROUTING,
    THREAD_ID_AGENT_MAX = THREAD_ID_AGENT_ROUTING,
};

#define CALLOC(id, size) calloc(size, size);
#define FREE(id, mem)    free(mem)

}    // namespace core

#endif    // __CORE_HPP__

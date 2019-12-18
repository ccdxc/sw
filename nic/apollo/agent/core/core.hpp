//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __CORE_HPP__
#define __CORE_HPP__

#include "nic/apollo/core/core.hpp"

namespace core {

enum {
    PDS_AGENT_THREAD_ID_NONE = PDS_THREAD_ID_MAX,
    PDS_AGENT_THREAD_ID_CMD_SERVER,
    PDS_AGENT_THREAD_ID_ROUTING,
    PDS_AGENT_THREAD_ID_MAX = 48,
};

#define CALLOC(id, size) calloc(size, size);
#define FREE(id, mem)    free(mem)

}    // namespace core

#endif    // __CORE_HPP__

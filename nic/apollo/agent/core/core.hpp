//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __CORE_HPP__
#define __CORE_HPP__

#include "nic/apollo/include/globals.hpp"

namespace core {

enum {
    PDS_AGENT_THREAD_ID_NONE = pds_thread_id_t::PDS_AGENT_THREAD_ID_MIN,
    PDS_AGENT_THREAD_ID_SVC_SERVER,
    PDS_AGENT_THREAD_ID_ROUTING,
    PDS_AGENT_THREAD_ID_MAX = pds_thread_id_t::PDS_AGENT_THREAD_ID_MAX,
};

#define CALLOC(id, size) calloc(size, size);
#define FREE(id, mem)    free(mem)

}    // namespace core

#endif    // __CORE_HPP__

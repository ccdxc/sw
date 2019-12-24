//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines basic types and API for messaging layer
///
//----------------------------------------------------------------------------

#ifndef __CORE_MSG_HPP__
#define __CORE_MSG_HPP__

#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/framework/api.h"

// forward declarations
typedef struct pds_msg_s pds_msg_t;
typedef struct pds_msg_list_s pds_msg_list_t;

namespace core {

pds_msg_list_t *pds_msg_list_alloc(uint32_t num_msgs);
void pds_msg_list_free(pds_msg_list_t *msg_list);
pds_msg_t *pds_msg(pds_msg_list_t *pds_msg_list, uint32_t idx);

}    // namespace core

#endif    // __CORE_MSG_HPP__

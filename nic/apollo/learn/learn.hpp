//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// common definitions needed by local and remote endpoints
///
//----------------------------------------------------------------------------

#ifndef __LEARN__LEARN_HPP__
#define __LEARN__LEARN_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"

namespace learn {

#define EP_MAX_MAC_ENTRY PDS_MAX_VNIC
#define EP_MAX_IP_ENTRY  (EP_MAX_MAC_ENTRY * 32)

/// \brief endpoint state
/// \remark
/// all endpoint state changes are done in the context on learn thread
/// if the state is LEARNING, UPDATING or DELETING, it indicates that an
/// event is currently modifying p4 one or more p4 table entries and a
/// subsequent event that needs to manipulate the same EP must wait
typedef enum {
    EP_STATE_LEARNING,      ///< EP is being programmed
    EP_STATE_CREATED,       ///< EP is programmed
    EP_STATE_PROBING,       ///< EP is awaiting arp probe response
    EP_STATE_UPDATING,      ///< EP is being updated
    EP_STATE_DELETING,      ///< EP is being deleted
    EP_STATE_DELETED,       ///< EP is deleted
} ep_state_t;

/// \brief key to L2 endpoint data base
typedef struct {
    pds_obj_key_t subnet;
    mac_addr_t mac_addr;
} __PACK__ ep_mac_key_t;

/// \brief key to L3 endpoint data base
typedef struct {
    pds_obj_key_t vpc;
    ip_addr_t ip_addr;
} __PACK__ ep_ip_key_t;

/// \brief learn type of the endpoint under process
typedef enum {
    LEARN_TYPE_INVALID = 0,      ///< learn type not available
    LEARN_TYPE_NONE,             ///< existing entry, nothing to learn
    LEARN_TYPE_NEW_LOCAL,        ///< new local learn
    LEARN_TYPE_NEW_REMOTE,       ///< new remote learn
    LEARN_TYPE_MOVE_L2L,         ///< local to local move
    LEARN_TYPE_MOVE_R2L,         ///< remote to local move
    LEARN_TYPE_MOVE_L2R,         ///< local to remote move
    LEARN_TYPE_MOVE_R2R,         ///< remote to remote move
} ep_learn_type_t;

}    // namespace learn

#endif    // __LEARN__LEARN_HPP__

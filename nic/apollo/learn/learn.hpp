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
/// subsequent event that needs to manipulate the same ep must wait
typedef enum {
    EP_STATE_LEARNING,      ///< ep is being programmed
    EP_STATE_CREATED,       ///< ep is programmed
    EP_STATE_PROBING,       ///< ep is awaiting arp probe response
    EP_STATE_UPDATING,      ///< ep is being updated
    EP_STATE_DELETING,      ///< ep is being deleted
    EP_STATE_DELETED,       ///< ep is deleted
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
    EP_LT_NONE,             ///< existing entry, nothing to learn
    EP_LT_NEW_LOCAL,        ///< new local learn
    EP_LT_NEW_REMOTE,       ///< new remote learn
    EP_LT_MOVE_L2L,         ///< local to local move
    EP_LT_MOVE_R2L,         ///< remote to local move
    EP_LT_MOVE_L2R,         ///< local to remote move
    EP_LT_MOVE_R2R,         ///< remote to remote move
} ep_learn_type_t;

/// \brief slab ids for objects allocated in learn
enum {
    LEARN_SLAB_ID_MIN = 0,
    LEARN_SLAB_ID_EP_IP = LEARN_SLAB_ID_MIN,
    LEARN_SLAB_ID_EP_MAC,
    LEARN_SLAB_ID_OBJ_MAX,
    LEARN_SLAB_ID_MAX = 4,
};

/// \brief destination for pkts sent out from learn
typedef enum {
    LEARN_PKT_NEXTHOP_NONE = 0,         ///< send pkt back to p4 ingress
    LEARN_PKT_NEXTHOP_DATAPATH_MNIC,    ///< send pkt to datapath mnic (VPP)
} learn_nexthop_t;

}    // namespace learn

#endif    // __LEARN__LEARN_HPP__

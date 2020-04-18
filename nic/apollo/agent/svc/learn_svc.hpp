//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for learn object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_LEARN_SVC_HPP__
#define __AGENT_SVC_LEARN_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/learn.hpp"
#include "nic/apollo/learn/learn.hpp"
#include "nic/apollo/learn/learn_state.hpp"

using learn::ep_mac_key_t;
using learn::ep_ip_key_t;
using learn::ep_state_t;

static inline void
pds_learn_mackey_proto_to_api (learn::ep_mac_key_t *mac_key,
                               const pds::LearnMACKey &key)
{
    MAC_UINT64_TO_ADDR(mac_key->mac_addr, key.macaddr());
    pds_obj_key_proto_to_api_spec(&mac_key->subnet, key.subnetid());
}

static inline void
pds_learn_ipkey_proto_to_api (learn::ep_ip_key_t *ip_key,
                              const pds::LearnIPKey &key)
{
    ipaddr_proto_spec_to_api_spec(&ip_key->ip_addr, key.ipaddr());
    pds_obj_key_proto_to_api_spec(&ip_key->vpc, key.vpcid());
}

static inline pds::EpState
pds_learn_state_to_proto (learn::ep_state_t state)
{
    pds::EpState proto_state;

    switch (state) {
    case learn::EP_STATE_LEARNING:
        proto_state = pds::EP_STATE_LEARNING;
        break;
    case learn::EP_STATE_CREATED:
        proto_state = pds::EP_STATE_CREATED;
        break;
    case learn::EP_STATE_PROBING:
        proto_state = pds::EP_STATE_PROBING;
        break;
    case learn::EP_STATE_UPDATING:
        proto_state = pds::EP_STATE_UPDATING;
        break;
    case learn::EP_STATE_DELETING:
        proto_state =  pds::EP_STATE_DELETING;
        break;
    case learn::EP_STATE_DELETED:
        proto_state = pds::EP_STATE_DELETED;
        break;
    default:
        proto_state = pds::EP_STATE_NONE;
        break;
    }
    return proto_state;
}

static inline pds::LearnPktDropReason
pds_learn_pkt_drop_reason_to_proto (uint8_t reason)
{
    pds::LearnPktDropReason proto_reason;

    switch (reason) {
    case learn::PKT_DROP_REASON_PARSE_ERR:
        proto_reason = pds::LEARN_PKTDROP_REASON_PARSE_ERR;
        break;
    case learn::PKT_DROP_REASON_RES_ALLOC_FAIL:
        proto_reason = pds::LEARN_PKTDROP_REASON_RES_ALLOC_FAIL;
        break;
    case learn::PKT_DROP_REASON_LEARNING_FAIL:
        proto_reason = pds::LEARN_PKTDROP_REASON_LEARNING_FAIL;
        break;
    case learn::PKT_DROP_REASON_MBUF_ERR:
        proto_reason = pds::LEARN_PKTDROP_REASON_MBUF_ERR;
        break;
    case learn::PKT_DROP_REASON_TX_FAIL:
        proto_reason = pds::LEARN_PKTDROP_REASON_TX_FAIL;
        break;
    case learn::PKT_DROP_REASON_ARP_REPLY:
        proto_reason = pds::LEARN_PKTDROP_REASON_ARP_REPLY;
        break;
    default:
        proto_reason = pds::LEARN_PKTDROP_REASON_NONE;
        break;
    }
    return proto_reason;
}

static inline pds::LearnEventType
pds_ep_learn_type_to_proto (uint8_t learn_type)
{
    switch (learn_type) {
    case learn::LEARN_TYPE_NEW_LOCAL:
        return pds::LEARN_EVENT_NEW_LOCAL;
    case learn::LEARN_TYPE_NEW_REMOTE:
        return pds::LEARN_EVENT_NEW_REMOTE;
    case learn::LEARN_TYPE_MOVE_L2L:
        return pds::LEARN_EVENT_L2L_MOVE;
    case learn::LEARN_TYPE_MOVE_R2L:
        return pds::LEARN_EVENT_R2L_MOVE;
    case learn::LEARN_TYPE_MOVE_L2R:
        return pds::LEARN_EVENT_L2R_MOVE;
    case learn::LEARN_TYPE_MOVE_R2R:
        return pds::LEARN_EVENT_R2R_MOVE;
    case learn::LEARN_TYPE_DELETE:
        return pds::LEARN_EVENT_DELETE;
    default:
        break;
    }
    return pds::LEARN_EVENT_NONE;
}

static inline pds::LearnValidationType
pds_learn_validation_type_to_proto (uint8_t validation_type)
{
    switch (validation_type) {
    case learn::UNTAGGED_MAC_LIMIT:
        return pds::LEARN_CHECK_UNTAGGED_MAC_LIMIT;
    case MAC_LIMIT:
        return pds::LEARN_CHECK_MAC_LIMIT;
    case IP_LIMIT:
        return pds::LEARN_CHECK_IP_LIMIT;
    case IP_ADDR_SUBNET_MISMATCH:
        return pds::LEARN_CHECK_IP_IN_SUBNET;
    default:
        break;
    }
    return pds::LEARN_CHECK_NONE;
}

static inline pds::LearnApiOpType
pds_learn_api_op_to_proto (uint8_t op_type)
{
    switch (op_type) {
    case OP_CREATE:
        return pds::LEARN_API_OP_CREATE;
    case OP_DELETE:
        return pds::LEARN_API_OP_DELETE;
    case OP_UPDATE:
        return pds::LEARN_API_OP_UPDATE;
    default:
        break;
    }
    return pds::LEARN_API_OP_NONE;
}

#endif    //__AGENT_SVC_LEARN_SVC_HPP__

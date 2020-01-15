//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NCSI_IPC_HPP__
#define __NCSI_IPC_HPP__

#include "gen/proto/ncsi.pb.h"

using ncsi::VlanFilterRequest;
using ncsi::VlanFilterResponse;
using ncsi::VlanFilterGetRequest;
using ncsi::VlanFilterGetResponseMsg;
using ncsi::MacFilterRequest;
using ncsi::MacFilterResponse;
using ncsi::MacFilterGetRequest;
using ncsi::MacFilterGetResponseMsg;
using ncsi::BcastFilterRequest;
using ncsi::BcastFilterResponse;
using ncsi::BcastFilterDeleteRequest;
using ncsi::BcastFilterDeleteResponse;
using ncsi::BcastFilterGetRequest;
using ncsi::BcastFilterGetResponseMsg;
using ncsi::McastFilterRequest;
using ncsi::McastFilterResponse;
using ncsi::McastFilterDeleteRequest;
using ncsi::McastFilterDeleteResponse;
using ncsi::McastFilterGetRequest;
using ncsi::McastFilterGetResponseMsg;
using ncsi::VlanModeRequest;
using ncsi::VlanModeResponse;
using ncsi::VlanModeGetRequest;
using ncsi::VlanModeGetResponseMsg;
using ncsi::ChannelRequest;
using ncsi::ChannelResponse;
using ncsi::ChannelGetRequest;
using ncsi::ChannelGetResponseMsg;
using google::protobuf::Message;

namespace hal {

#define NCSI_MSG_ID(ENTRY)                                                 \
    ENTRY(NCSI_MSG_VLAN_FILTER,            0, "NCSI_MSG_VLAN_FILTER")      \
    ENTRY(NCSI_MSG_MAC_FILTER,             1, "NCSI_MSG_MAC_FILTER")       \
    ENTRY(NCSI_MSG_BCAST_FILTER,           2, "NCSI_MSG_BCAST_FILTER")     \
    ENTRY(NCSI_MSG_MCAST_FILTER,           3, "NCSI_MSG_MCAST_FILTER")     \
    ENTRY(NCSI_MSG_VLAN_MODE,              4, "NCSI_MSG_VLAN_MODE")        \
    ENTRY(NCSI_MSG_CHANNEL,                5, "NCSI_MSG_CHANNEL") 

DEFINE_ENUM(ncsi_msg_id_t, NCSI_MSG_ID)
#undef NCSI_MSG_ID

#define NCSI_MSG_OPER(ENTRY)                                            \
    ENTRY(NCSI_MSG_OPER_CREATE,     0, "NCSI_MSG_OPER_CREATE")          \
    ENTRY(NCSI_MSG_OPER_UPDATE,     1, "NCSI_MSG_OPER_UPDATE")          \
    ENTRY(NCSI_MSG_OPER_DELETE,     2, "NCSI_MSG_OPER_DELETE")          \
    ENTRY(NCSI_MSG_OPER_GET,        3, "NCSI_MSG_OPER_GET")         

DEFINE_ENUM(ncsi_msg_oper_t, NCSI_MSG_OPER)
#undef NCSI_MSG_OPER

typedef struct ncsi_ipc_msg_s {
    ncsi_msg_id_t   msg_id;
    ncsi_msg_oper_t oper;
    Message         *msg;
    Message         *rsp;
    sdk_ret_t       rsp_ret;    // Return code from nicmgr
} ncsi_ipc_msg_t;


}    // namespace hal

#endif    // __NCSI_IPC_HPP__


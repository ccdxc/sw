/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __GRCP_IPC_SERVICE_H__
#define __GRCP_IPC_SERVICE_H__

#include <memory>
#include <string>
#include <iostream>

#include <grpc++/grpc++.h>
#include "include/sdk/base.hpp"
#include "gen/proto/ncsi.grpc.pb.h"
#include "nic/sdk/platform/ncsi/ipc_service.h"

using grpc::Status;
using ncsi::VlanFilterRequestMsg;
using ncsi::VlanFilterResponseMsg;
using ncsi::MacFilterRequestMsg;
using ncsi::MacFilterResponseMsg;
using ncsi::BcastFilterRequestMsg;
using ncsi::BcastFilterResponseMsg;
using ncsi::McastFilterRequestMsg;
using ncsi::McastFilterResponseMsg;
using ncsi::VlanModeRequestMsg;
using ncsi::VlanModeResponseMsg;
using ncsi::ChannelRequestMsg;
using ncsi::ChannelResponseMsg;

using namespace ncsi;

class grpc_ipc : public IpcService {
private:
    std::unique_ptr<ncsi::Ncsi::Stub> ncsi_stub_;
    std::shared_ptr<grpc::Channel> channel;
    // Make GRPC connection to HAL
    Status vlan_create (VlanFilterRequestMsg& req_msg, 
            VlanFilterResponseMsg& rsp_msg);
    Status vlan_delete (VlanFilterRequestMsg& req_msg, 
            VlanFilterResponseMsg& rsp_msg);

    Status vlan_mode_create (VlanModeRequestMsg& req_msg, 
            VlanModeResponseMsg& rsp_msg);
    Status vlan_mode_update (VlanModeRequestMsg& req_msg, 
            VlanModeResponseMsg& rsp_msg);

    Status mac_filter_create (MacFilterRequestMsg& req_msg, 
            MacFilterResponseMsg& rsp_msg);
    Status mac_filter_delete (MacFilterRequestMsg& req_msg, 
            MacFilterResponseMsg& rsp_msg);

    Status channel_state_create (ChannelRequestMsg& req_msg, 
            ChannelResponseMsg& rsp_msg);
    Status channel_state_update (ChannelRequestMsg& req_msg, 
            ChannelResponseMsg& rsp_msg); 
    Status channel_state_get (ChannelGetRequestMsg& req_msg, 
            ChannelGetResponseMsg& rsp_msg);
    Status bcast_filters_create(BcastFilterRequestMsg&, BcastFilterResponseMsg&);
    Status bcast_filters_update(BcastFilterRequestMsg&, BcastFilterResponseMsg&);
    Status bcast_filters_get(BcastFilterGetRequestMsg&, BcastFilterGetResponseMsg&);
    Status mcast_filters_create(McastFilterRequestMsg&, McastFilterResponseMsg&);
    Status mcast_filters_update(McastFilterRequestMsg&, McastFilterResponseMsg&);
    Status mcast_filters_get(McastFilterGetRequestMsg&, McastFilterGetResponseMsg&);

public:
    sdk_ret_t connect_hal(void);
    int PostMsg(struct VlanFilterMsg& vlan_filter);
    int PostMsg(struct VlanModeMsg& vlan_mode);
    int PostMsg(struct MacFilterMsg& mac_filter);
    int PostMsg(struct SetLinkMsg& set_link_msg);
    int PostMsg(struct EnableChanMsg& enable_ch);
    int PostMsg(struct ResetChanMsg& reset_ch);
    int PostMsg(struct EnableChanTxMsg& enable_ch_tx);
    int PostMsg(struct EnableBcastFilterMsg& bcast_filter);
    int PostMsg(struct EnableGlobalMcastFilterMsg& mcast_filter);
    int GetLinkStatus();
};

#endif //__GRCP_IPC_SERVICE_H__

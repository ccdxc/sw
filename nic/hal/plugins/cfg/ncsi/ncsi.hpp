//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NCSI_HPP__
#define __NCSI_HPP__

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

namespace hal {

hal_ret_t vlan_filter_create(VlanFilterRequest& req, VlanFilterResponse *rsp);
hal_ret_t vlan_filter_delete(VlanFilterRequest& req, VlanFilterResponse *rsp);
hal_ret_t vlan_filter_get(VlanFilterGetRequest& req, VlanFilterGetResponseMsg *rsp);

hal_ret_t mac_filter_create(MacFilterRequest& req, MacFilterResponse *rsp);
hal_ret_t mac_filter_delete(MacFilterRequest& req, MacFilterResponse *rsp);
hal_ret_t mac_filter_get(MacFilterGetRequest& req, MacFilterGetResponseMsg *rsp);

hal_ret_t bcast_filter_create(BcastFilterRequest& req, BcastFilterResponse *rsp);
hal_ret_t bcast_filter_update(BcastFilterRequest& req, BcastFilterResponse *rsp);
hal_ret_t bcast_filter_delete(BcastFilterDeleteRequest& req, BcastFilterDeleteResponse *rsp);
hal_ret_t bcast_filter_get(BcastFilterGetRequest& req, BcastFilterGetResponseMsg *rsp);

hal_ret_t mcast_filter_create(McastFilterRequest& req, McastFilterResponse *rsp);
hal_ret_t mcast_filter_update(McastFilterRequest& req, McastFilterResponse *rsp);
hal_ret_t mcast_filter_delete(McastFilterDeleteRequest& req, McastFilterDeleteResponse *rsp);
hal_ret_t mcast_filter_get(McastFilterGetRequest& req, McastFilterGetResponseMsg *rsp);

hal_ret_t vlan_mode_create(VlanModeRequest& req, VlanModeResponse *rsp);
hal_ret_t vlan_mode_update(VlanModeRequest& req, VlanModeResponse *rsp);
hal_ret_t vlan_mode_delete(VlanModeRequest& req, VlanModeResponse *rsp);
hal_ret_t vlan_mode_get(VlanModeGetRequest& req, VlanModeGetResponseMsg *rsp);

hal_ret_t channel_create(ChannelRequest& req, ChannelResponse *rsp);
hal_ret_t channel_update(ChannelRequest& req, ChannelResponse *rsp);
hal_ret_t channel_delete(ChannelRequest& req, ChannelResponse *rsp);
hal_ret_t channel_get(ChannelGetRequest& req, ChannelGetResponseMsg *rsp);

}    // namespace hal

#endif    // __NCSI_HPP__


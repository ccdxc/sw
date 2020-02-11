// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <arpa/inet.h>
#include "delphi_ipc.h"
#include "lib/logger/logger.hpp"

using namespace std;

extern delphi::SdkPtr g_sdk;
void UpdateLinkStatus(uint32_t port, bool link_status);
link_event_handler_ptr_t g_link_event_handler;

DelphiIpcService::DelphiIpcService(delphi::SdkPtr sk, delphi_cb cb)
{
    sdk_ = sk;
    callback = cb;
}

void DelphiIpcService::Init(shared_ptr<DelphiIpcService> ncsid_svc)
{
    // mount the NCSI objects
    delphi::objects::NcsiVlanFilter::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::NcsiVlanMode::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::NcsiMacFilter::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::NcsiSetLinkMsg::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::NcsiChanRx::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::NcsiResetChanMsg::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::NcsiChanTx::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::NcsiBcastFilter::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::NcsiMcastFilter::Mount(sdk_, delphi::ReadWriteMode);

    // create the PortStatus reactor
    g_link_event_handler = std::make_shared<link_event_handler>(sdk_);

    // mount port status obj for getting link change notifications
    delphi::objects::PortStatus::Mount(sdk_, delphi::ReadMode);

    // Register PortStatus reactor
    delphi::objects::PortStatus::Watch(sdk_, g_link_event_handler);

    // register ncsid as Delphi Service
    sdk_->RegisterService(ncsid_svc);
 
   // connect to delphi
    sdk_->Connect();
}

void DelphiIpcService::OnMountComplete()
{
    printf("OnMountComplete() got called\n");
    callback();
}

int DelphiIpcService::PostMsg(struct VlanFilterMsg& vlan_filter_msg)
{
    delphi::objects::NcsiVlanFilterPtr vlan_filter = 
        make_shared<delphi::objects::NcsiVlanFilter>();

    vlan_filter->set_key(vlan_filter_msg.filter_id);
    vlan_filter->set_port(vlan_filter_msg.port);
    vlan_filter->set_vlan_id(vlan_filter_msg.vlan_id);
    vlan_filter->set_enable(vlan_filter_msg.enable);

    sdk()->SetObject(vlan_filter);

    return 0;
}

int DelphiIpcService::PostMsg(struct VlanModeMsg& vlan_mode_msg)
{
    delphi::objects::NcsiVlanModePtr vlan_mode_obj = 
        make_shared<delphi::objects::NcsiVlanMode>();

    vlan_mode_obj->set_key(vlan_mode_msg.filter_id);
    vlan_mode_obj->set_port(vlan_mode_msg.port);
    vlan_mode_obj->set_vlan_mode(vlan_mode_msg.mode);
    vlan_mode_obj->set_enable(vlan_mode_msg.enable);

    sdk()->SetObject(vlan_mode_obj);

    return 0;
}

static uint8_t *
memrev (uint8_t *block, size_t elnum)
{
    uint8_t *s, *t, tmp;
    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
    return block;
}

int DelphiIpcService::PostMsg(struct MacFilterMsg& mac_filter_msg)
{
    delphi::objects::NcsiMacFilterPtr mac_filter = 
        make_shared<delphi::objects::NcsiMacFilter>();
    uint64_t mac_addr = 0;
    //memset(&mac_addr, 0, sizeof(mac_addr));
    memcpy(&mac_addr, mac_filter_msg.mac_addr, sizeof(mac_filter_msg.mac_addr));
    memrev((uint8_t* )&mac_addr, 6);

    mac_filter->set_key(mac_filter_msg.filter_id);
    mac_filter->set_port(mac_filter_msg.port);
    mac_filter->set_mac_addr(mac_addr);
    mac_filter->set_addr_type(mac_filter_msg.addr_type);
    mac_filter->set_enable(mac_filter_msg.enable);
    
    sdk()->SetObject(mac_filter);
    
    return 0;
}

int DelphiIpcService::PostMsg(struct SetLinkMsg& set_link_msg)
{
    delphi::objects::NcsiSetLinkMsgPtr set_link_obj = 
        make_shared<delphi::objects::NcsiSetLinkMsg>();

    set_link_obj->set_key(set_link_msg.filter_id);
    set_link_obj->set_port(set_link_msg.port);
    set_link_obj->set_link_up(set_link_msg.link_up);
    
    sdk()->SetObject(set_link_obj);
    
    return 0;
}

int DelphiIpcService::PostMsg(struct EnableChanMsg& enable_ch)
{
    delphi::objects::NcsiChanRxPtr enable_ch_obj = 
        make_shared<delphi::objects::NcsiChanRx>();

    enable_ch_obj->set_key(enable_ch.filter_id);
    enable_ch_obj->set_port(enable_ch.port);
    enable_ch_obj->set_enable(enable_ch.enable);
    
    sdk()->SetObject(enable_ch_obj);
    
    return 0;
}

int DelphiIpcService::PostMsg(struct ResetChanMsg& reset_ch)
{
    delphi::objects::NcsiResetChanMsgPtr reset_ch_obj = 
        make_shared<delphi::objects::NcsiResetChanMsg>();

    //FIXME: remove all filters for this channel.
    //TODO: Check if we have to disable chan and disable chan tx or not
    reset_ch_obj->set_key(reset_ch.filter_id);
    reset_ch_obj->set_port(reset_ch.port);
    reset_ch_obj->set_reset(reset_ch.reset);
    
    sdk()->SetObject(reset_ch_obj);
    
    return 0;
}

int DelphiIpcService::PostMsg(struct EnableChanTxMsg& enable_ch_tx)
{
    delphi::objects::NcsiChanTxPtr enable_ch_tx_obj = 
        make_shared<delphi::objects::NcsiChanTx>();

    enable_ch_tx_obj->set_key(enable_ch_tx.filter_id);
    enable_ch_tx_obj->set_port(enable_ch_tx.port);
    enable_ch_tx_obj->set_enable(enable_ch_tx.enable);
    
    sdk()->SetObject(enable_ch_tx_obj);
    
    return 0;
}

int DelphiIpcService::PostMsg(struct EnableBcastFilterMsg& bcast_filter)
{
    delphi::objects::NcsiBcastFilterPtr bcast_filter_obj = 
        make_shared<delphi::objects::NcsiBcastFilter>();

    bcast_filter_obj->set_key(bcast_filter.filter_id);
    bcast_filter_obj->set_port(bcast_filter.port);
    bcast_filter_obj->set_enable_arp(bcast_filter.enable_arp);
    bcast_filter_obj->set_enable_dhcp_client(bcast_filter.enable_dhcp_client);
    bcast_filter_obj->set_enable_dhcp_server(bcast_filter.enable_dhcp_server);
    bcast_filter_obj->set_enable_netbios(bcast_filter.enable_netbios);
    
    sdk()->SetObject(bcast_filter_obj);
    
    return 0;
}

int DelphiIpcService::PostMsg(struct EnableGlobalMcastFilterMsg& mcast_filter)
{
    delphi::objects::NcsiMcastFilterPtr mcast_filter_obj = 
        make_shared<delphi::objects::NcsiMcastFilter>();

    mcast_filter_obj->set_key(mcast_filter.filter_id);
    mcast_filter_obj->set_port(mcast_filter.port);
    mcast_filter_obj->set_enable_ipv6_neigh_adv(
            mcast_filter.enable_ipv6_neigh_adv);
    mcast_filter_obj->set_enable_ipv6_router_adv(
            mcast_filter.enable_ipv6_router_adv);
    mcast_filter_obj->set_enable_dhcpv6_relay(
            mcast_filter.enable_dhcpv6_relay);
    mcast_filter_obj->set_enable_dhcpv6_mcast(
            mcast_filter.enable_dhcpv6_mcast);
    mcast_filter_obj->set_enable_ipv6_mld(
            mcast_filter.enable_ipv6_mld);
    mcast_filter_obj->set_enable_ipv6_neigh_sol(
            mcast_filter.enable_ipv6_neigh_sol);

    sdk()->SetObject(mcast_filter_obj);

    return 0;
}

int DelphiIpcService::GetLinkStatus()
{
    g_link_event_handler->get_link_status();

    return 0;
}

void link_event_handler::get_link_status()
{
    std::vector <delphi::objects::PortStatusPtr> portstatus_list =
        delphi::objects::PortStatus::List(g_sdk);

    for (auto it = portstatus_list.begin(); it != portstatus_list.end(); ++it) {
        g_link_event_handler->update_link_status(*it);
    }
}

error link_event_handler::update_link_status(PortStatusPtr port_status)
{
    uint32_t port;
    bool link_status;

    //SDK_TRACE_INFO("Rcvd Link update event");

    // ignore if there is no link status in this event
    if (!port_status->has_link_status()) {
        SDK_TRACE_ERR("Rcvd link event without link status");
        return error::OK();
    }

    port = port_status->key_or_handle().port_id();
    link_status = (port_status->link_status().oper_state() == 
            1) ? true:false;

    SDK_TRACE_INFO("Rcvd link event for port: %d, link_status: %s", 
            port, link_status ? "Up":"Down");

    if (port == 1)
        port = 0;
    else if (port == 5)
        port = 1;
    else
        return error::OK();

    UpdateLinkStatus(port, link_status);

    return error::OK();
}

// OnLinkStatusUpdate gets called when PortStatus object is updated
error link_event_handler::OnPortStatusUpdate(PortStatusPtr portStatus) {
    SDK_TRACE_INFO("Rcvd link update event");
    return update_link_status(portStatus);
}

error link_event_handler::OnPortStatusCreate(PortStatusPtr portStatus) {
    SDK_TRACE_INFO("Rcvd link creation event");
    return update_link_status(portStatus);
}


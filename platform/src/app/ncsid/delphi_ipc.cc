// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include "delphi_ipc.h"

using namespace std;

DelphiIpcService::DelphiIpcService(delphi::SdkPtr sk, delphi_cb cb)
{
    sdk_ = sk;
    callback = cb;
}

void DelphiIpcService::OnMountComplete()
{
    printf("OnMountComplete() got called\n");
    callback();
}

int DelphiIpcService::PostMsg(struct VlanFilterMsg& vlan_filter_msg)
{
    delphi::objects::NcsiVlanFilterPtr vlan_filter = make_shared<delphi::objects::NcsiVlanFilter>();

    vlan_filter->set_key(vlan_filter_msg.filter_id);
    vlan_filter->set_port(vlan_filter_msg.port);
    vlan_filter->set_vlan_id(vlan_filter_msg.vlan_id);
    vlan_filter->set_enable(vlan_filter_msg.enable);

    sdk()->SetObject(vlan_filter);

    return 0;
}

int DelphiIpcService::PostMsg(struct VlanModeMsg& vlan_mode_msg)
{
    delphi::objects::NcsiVlanModePtr vlan_mode_obj = make_shared<delphi::objects::NcsiVlanMode>();

    vlan_mode_obj->set_key(vlan_mode_msg.filter_id);
    vlan_mode_obj->set_port(vlan_mode_msg.port);
    vlan_mode_obj->set_vlan_mode(vlan_mode_msg.mode);
    vlan_mode_obj->set_enable(vlan_mode_msg.enable);

    sdk()->SetObject(vlan_mode_obj);

    return 0;
}

int DelphiIpcService::PostMsg(struct MacFilterMsg& mac_filter_msg)
{
    delphi::objects::NcsiMacFilterPtr mac_filter = make_shared<delphi::objects::NcsiMacFilter>();

    mac_filter->set_key(mac_filter_msg.filter_id);
    mac_filter->set_port(mac_filter_msg.port);
    mac_filter->set_mac_addr(mac_filter_msg.mac_addr);
    mac_filter->set_addr_type(mac_filter_msg.addr_type);
    mac_filter->set_enable(mac_filter_msg.enable);
    
    sdk()->SetObject(mac_filter);
    
    return 0;

}

int DelphiIpcService::PostMsg(struct SetLinkMsg& set_link_msg)
{
    delphi::objects::NcsiSetLinkMsgPtr set_link_obj = make_shared<delphi::objects::NcsiSetLinkMsg>();

    set_link_obj->set_key(set_link_msg.filter_id);
    set_link_obj->set_port(set_link_msg.port);
    set_link_obj->set_link_up(set_link_msg.link_up);
    
    sdk()->SetObject(set_link_obj);
    
    return 0;
}

int DelphiIpcService::PostMsg(struct EnableChanMsg& enable_ch)
{
    delphi::objects::NcsiEnableChanMsgPtr enable_ch_obj = make_shared<delphi::objects::NcsiEnableChanMsg>();

    enable_ch_obj->set_key(enable_ch.filter_id);
    enable_ch_obj->set_port(enable_ch.port);
    enable_ch_obj->set_enable(enable_ch.enable);
    
    sdk()->SetObject(enable_ch_obj);
    
    return 0;
}

int DelphiIpcService::PostMsg(struct ResetChanMsg& reset_ch)
{
    delphi::objects::NcsiResetChanMsgPtr reset_ch_obj = make_shared<delphi::objects::NcsiResetChanMsg>();

    reset_ch_obj->set_key(reset_ch.filter_id);
    reset_ch_obj->set_port(reset_ch.port);
    reset_ch_obj->set_reset(reset_ch.reset);
    
    sdk()->SetObject(reset_ch_obj);
    
    return 0;
}

int DelphiIpcService::PostMsg(struct EnableChanTxMsg& enable_ch_tx)
{
    delphi::objects::NcsiEnableChanTxMsgPtr enable_ch_tx_obj = make_shared<delphi::objects::NcsiEnableChanTxMsg>();

    enable_ch_tx_obj->set_key(enable_ch_tx.filter_id);
    enable_ch_tx_obj->set_port(enable_ch_tx.port);
    enable_ch_tx_obj->set_enable(enable_ch_tx.enable);
    
    sdk()->SetObject(enable_ch_tx_obj);
    
    return 0;
}

int DelphiIpcService::PostMsg(struct EnableBcastFilterMsg& bcast_filter)
{
    delphi::objects::NcsiEnableBcastFilterPtr bcast_filter_obj = make_shared<delphi::objects::NcsiEnableBcastFilter>();

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
    delphi::objects::NcsiEnableGlobalMcastFilterPtr mcast_filter_obj = make_shared<delphi::objects::NcsiEnableGlobalMcastFilter>();

    mcast_filter_obj->set_key(mcast_filter.filter_id);
    mcast_filter_obj->set_port(mcast_filter.port);
    mcast_filter_obj->set_enable_ipv6_neigh_adv(mcast_filter.enable_ipv6_neigh_adv);
    mcast_filter_obj->set_enable_ipv6_router_adv(mcast_filter.enable_ipv6_router_adv);
    mcast_filter_obj->set_enable_dhcpv6_relay(mcast_filter.enable_dhcpv6_relay);
    mcast_filter_obj->set_enable_dhcpv6_mcast(mcast_filter.enable_dhcpv6_mcast);
    mcast_filter_obj->set_enable_ipv6_mld(mcast_filter.enable_ipv6_mld);
    mcast_filter_obj->set_enable_ipv6_neigh_sol(mcast_filter.enable_ipv6_neigh_sol);
    
    sdk()->SetObject(mcast_filter_obj);
    
    return 0;
}


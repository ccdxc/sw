/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __IPC_SERVICE_H__
#define __IPC_SERVICE_H__

#include <string>
#include <memory>

struct VlanFilterMsg {
    uint32_t filter_id;
    uint32_t port;
    uint16_t vlan_id;
    bool enable;
};

struct VlanModeMsg {
    uint32_t filter_id;
    uint32_t port;
    uint8_t mode;
    bool enable;
};

struct MacFilterMsg {
    uint32_t filter_id;
    uint32_t port;
    uint64_t mac_addr;
    uint32_t addr_type;
    bool enable;
};

struct SetLinkMsg {
    uint32_t filter_id;
    uint32_t port;
    bool link_up;
};

struct EnableChanMsg {
    uint32_t filter_id;
    uint32_t port;
    bool enable;
};

struct ResetChanMsg {
    uint32_t filter_id;
    uint32_t port;
    bool reset;
};

struct EnableChanTxMsg {
    uint32_t filter_id;
    uint32_t port;
    bool enable;
};

struct EnableBcastFilterMsg {
    uint32_t filter_id;
    uint32_t port;
    bool enable_arp;
    bool enable_dhcp_client;
    bool enable_dhcp_server;
    bool enable_netbios;
};

struct EnableGlobalMcastFilterMsg {
    uint32_t filter_id;
    uint32_t port;
    bool enable_ipv6_neigh_adv;
    bool enable_ipv6_router_adv;
    bool enable_dhcpv6_relay;
    bool enable_dhcpv6_mcast;
    bool enable_ipv6_mld;
    bool enable_ipv6_neigh_sol;
};

class IpcService {
public:
    IpcService() {};
    virtual int PostMsg(struct VlanFilterMsg& vlan_filter) = 0;
    virtual int PostMsg(struct VlanModeMsg& vlan_mode) = 0;
    virtual int PostMsg(struct MacFilterMsg& mac_filter) = 0;
    virtual int PostMsg(struct SetLinkMsg& set_link_msg) = 0;
    virtual int PostMsg(struct EnableChanMsg& enable_ch) = 0;
    virtual int PostMsg(struct ResetChanMsg& reset_ch) = 0;
    virtual int PostMsg(struct EnableChanTxMsg& enable_ch_tx) = 0;
    virtual int PostMsg(struct EnableBcastFilterMsg& bcast_filter) = 0;
    virtual int PostMsg(struct EnableGlobalMcastFilterMsg& mcast_filter) = 0;

};

#endif // __IPC_SERVICE_H__

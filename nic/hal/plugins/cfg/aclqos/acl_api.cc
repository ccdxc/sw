//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/plugins/cfg/aclqos/acl.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/plugins/cfg/aclqos/acl_api.hpp"

namespace hal {

// ----------------------------------------------------------------------------
// Acl API: Get Match spec in PI Acl
// ----------------------------------------------------------------------------
acl_match_spec_t *
acl_get_match_spec(acl_t *pi_acl)
{
    return &pi_acl->match_spec;
}

// ----------------------------------------------------------------------------
// Acl API: Get Action spec in PI Acl
// ----------------------------------------------------------------------------
acl_action_spec_t *
acl_get_action_spec(acl_t *pi_acl)
{
    return &pi_acl->action_spec;
}

// ----------------------------------------------------------------------------
// Acl API: Get priority in PI Acl
// ----------------------------------------------------------------------------
uint32_t
acl_get_priority(acl_t *pi_acl)
{
    return pi_acl->priority;
}

// ----------------------------------------------------------------------------
// Install Bcast all
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_bcast_all()
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    uint32_t         acl_id;
    uint32_t         priority;


    acl_id = ACL_NCSI_BCAST_ALL;
    priority = ACL_NCSI_BCAST_ALL_PRIORITY;

    match = spec.mutable_match();
    action = spec.mutable_action();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);
    action->mutable_internal_actions()->set_egress_drop(true);

    // Selector
    match->mutable_eth_selector()->set_dst_mac(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);

    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install bcast all. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_bcast_all()
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;

    acl_id = ACL_NCSI_BCAST_ALL;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall bcast all. err: {}", ret);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Install Bcast ARP
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_bcast_arp()
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    uint32_t         acl_id;
    uint32_t         priority;


    acl_id = ACL_NCSI_BCAST_ARP;
    priority = ACL_NCSI_BCAST_ARP_PRIORITY;

    match = spec.mutable_match();
    action = spec.mutable_action();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_eth_selector()->set_dst_mac(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_ARP);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);

    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install bcast arp. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_bcast_arp()
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;

    acl_id = ACL_NCSI_BCAST_ARP;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall bcast arp. err: {}", ret);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Install Bcast DHCP Client
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_bcast_dhcp_client()
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;


    acl_id = ACL_NCSI_BCAST_DHCP_CLIENT;
    priority = ACL_NCSI_BCAST_DHCP_CLIENT_PRIORITY;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_eth_selector()->set_dst_mac(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV4);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_low(DHCP_SERVER_PORT);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_high(DHCP_SERVER_PORT);

    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install bcast dhcp client. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_bcast_dhcp_client()
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;

    acl_id = ACL_NCSI_BCAST_DHCP_CLIENT;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall bcast dhcp client. err: {}", ret);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Install Bcast DHCP Server
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_bcast_dhcp_server()
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;


    acl_id = ACL_NCSI_BCAST_DHCP_SERVER;
    priority = ACL_NCSI_BCAST_DHCP_SERVER_PRIORITY;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_eth_selector()->set_dst_mac(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV4);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_low(DHCP_CLIENT_PORT);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_high(DHCP_CLIENT_PORT);

    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install bcast dhcp server. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_bcast_dhcp_server()
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;

    acl_id = ACL_NCSI_BCAST_DHCP_SERVER;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall bcast dhcp server. err: {}", ret);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Install Bcast Netbios
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_bcast_netbios()
{
    hal_ret_t        ret;

    ret = acl_install_bcast_netbios_ns();
    ret = acl_install_bcast_netbios_ds();

    return ret;
}
hal_ret_t
acl_uninstall_bcast_netbios()
{
    hal_ret_t        ret;

    ret = acl_uninstall_bcast_netbios_ns();
    ret = acl_uninstall_bcast_netbios_ds();

    return ret;
}

// ----------------------------------------------------------------------------
// Install Bcast Netbios nameservice
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_bcast_netbios_ns()
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;


    acl_id = ACL_NCSI_BCAST_NETBIOS_NS;
    priority = ACL_NCSI_BCAST_NETBIOS_NS_PRIORITY;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_eth_selector()->set_dst_mac(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV4);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_low(NETBIOS_NS_PORT);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_high(NETBIOS_NS_PORT);

    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install netbios ns. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_bcast_netbios_ns()
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;

    acl_id = ACL_NCSI_BCAST_NETBIOS_NS;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall netbios ns. err: {}", ret);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Install Bcast Netbios dataservice
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_bcast_netbios_ds()
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;


    acl_id = ACL_NCSI_BCAST_NETBIOS_DS;
    priority = ACL_NCSI_BCAST_NETBIOS_DS_PRIORITY;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_eth_selector()->set_dst_mac(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV4);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_low(NETBIOS_DS_PORT);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_high(NETBIOS_DS_PORT);

    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install netbios ds. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_bcast_netbios_ds()
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;

    acl_id = ACL_NCSI_BCAST_NETBIOS_DS;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall netbios ds. err: {}", ret);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Install Mcast IPv6 Neighbor Adv.
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_mcast_ipv6_nadv()
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;


    acl_id = ACL_NCSI_MCAST_IPV6_NADV;
    priority = ACL_NCSI_MCAST_IPV6_NADV_PRIORITY;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_eth_selector()->set_dst_mac(0x333300000001);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV6);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
    ip_select->mutable_icmp_selector()->set_icmp_code(0);
    ip_select->mutable_icmp_selector()->set_icmp_code_mask(0xff);
    ip_select->mutable_icmp_selector()->set_icmp_type(ICMP_NEIGHBOR_ADVERTISEMENT);
    ip_select->mutable_icmp_selector()->set_icmp_type_mask(0xff);

    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install neigh adv. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_ipv6_nadv()
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;

    acl_id = ACL_NCSI_MCAST_IPV6_NADV;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall neigh adv. err: {}", ret);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Install Mcast IPv6 Router Adv.
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_mcast_ipv6_radv()
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;

    acl_id = ACL_NCSI_MCAST_IPV6_RADV;
    priority = ACL_NCSI_MCAST_IPV6_RADV_PRIORITY;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_eth_selector()->set_dst_mac(0x333300000001);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV6);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
    ip_select->mutable_icmp_selector()->set_icmp_code(0);
    ip_select->mutable_icmp_selector()->set_icmp_code_mask(0xff);
    ip_select->mutable_icmp_selector()->set_icmp_type(ICMP_ROUTER_ADVERTISEMENT);
    ip_select->mutable_icmp_selector()->set_icmp_type_mask(0xff);

    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install router adv. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_ipv6_radv()
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;

    acl_id = ACL_NCSI_MCAST_IPV6_RADV;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall router adv. err: {}", ret);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Install DHCPV6 relay
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_mcast_dhcpv6_relay()
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;

    acl_id = ACL_NCSI_MCAST_DHCPV6_RELAY;
    priority = ACL_NCSI_MCAST_DHCPV6_RELAY_PRIORITY;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_eth_selector()->set_dst_mac(0x333300010002);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFE);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV6);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_low(DHCPV6_RELAY_PORT);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_high(DHCPV6_RELAY_PORT);

    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install dhcpv6 relay. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_dhcpv6_relay()
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;

    acl_id = ACL_NCSI_MCAST_DHCPV6_RELAY;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall dhcpv6 relay. err: {}", ret);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Install DHCPV6 mcast
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_mcast_dhcpv6_mcast()
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;

    acl_id = ACL_NCSI_MCAST_DHCPV6_MCAST;
    priority = ACL_NCSI_MCAST_DHCPV6_MCAST;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_eth_selector()->set_dst_mac(0x333300010002);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV6);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_low(DHCPV6_MCAST_PORT);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_high(DHCPV6_MCAST_PORT);

    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install dhcpv6 mcast.  err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_dhcpv6_mcast()
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;

    acl_id = ACL_NCSI_MCAST_DHCPV6_MCAST;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall dhcpv6 mcast. err: {}", ret);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Install Mcast MLD Listener Query
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_mcast_mld ()
{
    hal_ret_t ret;

    ret = acl_install_mcast_mld_lq();
    ret = acl_install_mcast_mld_lr();
    ret = acl_install_mcast_mld_ld();

    return ret;
}
hal_ret_t
acl_uninstall_mcast_mld ()
{
    hal_ret_t ret;

    ret = acl_uninstall_mcast_mld_lq();
    ret = acl_uninstall_mcast_mld_lr();
    ret = acl_uninstall_mcast_mld_ld();

    return ret;
}
hal_ret_t
acl_install_mcast_mld_lq()
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;

    acl_id = ACL_NCSI_MCAST_IPV6_MLD_LQ;
    priority = ACL_NCSI_MCAST_IPV6_MLD_LQ_PRIORITY;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_eth_selector()->set_dst_mac(0x333300000001);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV6);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
    ip_select->mutable_icmp_selector()->set_icmp_code(0);
    ip_select->mutable_icmp_selector()->set_icmp_code_mask(0xff);
    ip_select->mutable_icmp_selector()->set_icmp_type(ICMP_MCAST_LISTENER_QUERY);
    ip_select->mutable_icmp_selector()->set_icmp_type_mask(0xff);

    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install mld lq. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_mld_lq()
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;

    acl_id = ACL_NCSI_MCAST_IPV6_MLD_LQ;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall mld lq. err: {}", ret);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Install Mcast MLD Listener Report
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_mcast_mld_lr()
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;

    acl_id = ACL_NCSI_MCAST_IPV6_MLD_LR;
    priority = ACL_NCSI_MCAST_IPV6_MLD_LR_PRIORITY;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_eth_selector()->set_dst_mac(0x333300000001);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV6);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
    ip_select->mutable_icmp_selector()->set_icmp_code(0);
    ip_select->mutable_icmp_selector()->set_icmp_code_mask(0xff);
    ip_select->mutable_icmp_selector()->set_icmp_type(ICMP_MCAST_LISTENER_REPORT);
    ip_select->mutable_icmp_selector()->set_icmp_type_mask(0xff);

    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install mld lr. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_mld_lr()
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;

    acl_id = ACL_NCSI_MCAST_IPV6_MLD_LR;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall mld lr. err: {}", ret);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Install Mcast MLD Listener Done
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_mcast_mld_ld()
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;

    acl_id = ACL_NCSI_MCAST_IPV6_MLD_LD;
    priority = ACL_NCSI_MCAST_IPV6_MLD_LD_PRIORITY;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_eth_selector()->set_dst_mac(0x333300000001);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV6);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
    ip_select->mutable_icmp_selector()->set_icmp_code(0);
    ip_select->mutable_icmp_selector()->set_icmp_code_mask(0xff);
    ip_select->mutable_icmp_selector()->set_icmp_type(ICMP_MCAST_LISTENER_DONE);
    ip_select->mutable_icmp_selector()->set_icmp_type_mask(0xff);

    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install mld ld. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_mld_ld()
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;

    acl_id = ACL_NCSI_MCAST_IPV6_MLD_LD;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall mld ld. err: {}", ret);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Install Mcast Neighbor Solicit
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_mcast_ipv6_nsol()
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;

    acl_id = ACL_NCSI_MCAST_IPV6_NSOL;
    priority = ACL_NCSI_MCAST_IPV6_NSOL_PRIORITY;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_eth_selector()->set_dst_mac(0x3333FF000000);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFF000000);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV6);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
    ip_select->mutable_icmp_selector()->set_icmp_code(0);
    ip_select->mutable_icmp_selector()->set_icmp_code_mask(0xff);
    ip_select->mutable_icmp_selector()->set_icmp_type(ICMP_NEIGHBOR_SOLICITATION);
    ip_select->mutable_icmp_selector()->set_icmp_type_mask(0xff);

    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install ipv6 nsol. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_ipv6_nsol()
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;

    acl_id = ACL_NCSI_MCAST_IPV6_NSOL;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall ipv6 nsol. err: {}", ret);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Install Mcast all
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_mcast_all()
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    uint32_t         acl_id;
    uint32_t         priority;


    acl_id = ACL_NCSI_MCAST_ALL;
    priority = ACL_NCSI_MCAST_ALL_PRIORITY;

    match = spec.mutable_match();
    action = spec.mutable_action();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);
    action->mutable_internal_actions()->set_egress_drop(true);

    // Selector
    match->mutable_eth_selector()->set_dst_mac(0x010000000000);
    match->mutable_eth_selector()->set_dst_mac_mask(0x010000000000);

    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install mcast all. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_all()
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;

    acl_id = ACL_NCSI_MCAST_ALL;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall mcast all. err: {}", ret);
    }
    return ret;
}

//

} // namespace hal

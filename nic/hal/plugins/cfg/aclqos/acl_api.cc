//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/plugins/cfg/aclqos/acl.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/plugins/cfg/aclqos/acl_api.hpp"
#include "nic/sdk/platform/pal/include/pal.h"

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
// Install NCSI redirect to oob_mnic0
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_ncsi_redirect (if_t *oob_mnic_enic, if_t *oob_uplink_if)
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    uint32_t         acl_id;
    uint32_t         priority;

    acl_id = ACL_NCSI_OOB_REDIRECT_ID1;
    priority = ACL_NCSI_OOB_REDIRECT_PRIORITY;

    match = spec.mutable_match();
    action = spec.mutable_action();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_REDIRECT);
    action->mutable_redirect_if_key_handle()->set_interface_id(oob_mnic_enic->if_id);

    // Selector
    match->mutable_src_if_key_handle()->set_interface_id(oob_uplink_if->if_id);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_NCSI);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);

    HAL_TRACE_DEBUG("Installing ACL for NCSI uplink -> oob_mnic");
    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install NCSI uplink -> oob_mnic. err: {}", ret);
    }

    acl_id = ACL_NCSI_OOB_REDIRECT_ID2;
    priority = ACL_NCSI_OOB_REDIRECT_PRIORITY;

    match = spec.mutable_match();
    action = spec.mutable_action();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_REDIRECT);
    action->mutable_redirect_if_key_handle()->set_interface_id(oob_uplink_if->if_id);

    // Selector
    match->mutable_src_if_key_handle()->set_interface_id(oob_mnic_enic->if_id);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_NCSI);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);

    HAL_TRACE_DEBUG("Installing ACL for NCSI oob_mnic -> uplink");
    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install NCSI oob_mnic -> uplink. err: {}", ret);
    }

    if (pal_swm_enabled()) {
        acl_id = ACL_NCSI_OOB_REDIRECT_ID3;
        priority = ACL_NCSI_OOB_BCAST_DROP_PRIORITY;

        match = spec.mutable_match();
        action = spec.mutable_action();
        match->Clear();
        action->Clear();
        spec.mutable_key_or_handle()->set_acl_id(acl_id);
        spec.set_priority(priority);

        // Action
        action->set_action(acl::AclAction::ACL_ACTION_DENY);

        // Selector
        match->mutable_src_if_key_handle()->set_interface_id(oob_mnic_enic->if_id);
        match->mutable_internal_key()->set_outer_dst_mac(0xFFFFFFFFFFFF);
        match->mutable_internal_mask()->set_outer_dst_mac(0xFFFFFFFFFFFF);

        HAL_TRACE_DEBUG("Installing ACL for NCSI bcast drop oob_mnic -> uplink");
        ret = hal::acl_create(spec, &rsp);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to install NCSI bcast drop oob_mnic -> uplink. "
                          "err: {}", ret);
        }
    }
    return ret;
}
hal_ret_t
acl_uninstall_ncsi_redirect (void)
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;

    acl_id = ACL_NCSI_OOB_REDIRECT_ID1;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    HAL_TRACE_DEBUG("UnInstalling ACL for NCSI uplink -> oob_mnic");
    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall NCSI uplink -> oob_mnic. err: {}", ret);
    }

    acl_id = ACL_NCSI_OOB_REDIRECT_ID2;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    HAL_TRACE_DEBUG("UnInstalling ACL for ncsi oob_mnic -> uplink");
    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall NCSI oob_mnic -> uplink. err: {}", ret);
    }

    if (pal_swm_enabled()) {
        acl_id = ACL_NCSI_OOB_REDIRECT_ID3;
        req.mutable_key_or_handle()->set_acl_id(acl_id);

        HAL_TRACE_DEBUG("UnInstalling ACL for ncsi bcast drop oob_mnic -> uplink");
        ret = hal::acl_delete(req, &rsp);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to uninstall NCSI bcast drop oob_mnic -> uplink. "
                          "err: {}", ret);
        }
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Install Bcast all
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_bcast_all (if_t *hal_if)
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    uint32_t         acl_id;
    uint32_t         priority;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_BCAST_ALL + if_idx;
    priority = ACL_NCSI_BCAST_ALL_PRIORITY + if_idx;

    match = spec.mutable_match();
    action = spec.mutable_action();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);
    action->set_egress_drop(true);

    // Selector
    /*
     * Mcast/Bcast packets coming from host pinned to this interface
     * are getting replicated to swm lif. We have to prevent that traffic.
     * So all traffic coming from anywhere will hit this entry but only 
     * traffic trying to go to swm enics will get dropped.
     */
    // match->mutable_src_if_key_handle()->set_interface_id(hal_if->if_id);
    match->mutable_internal_key()->set_outer_dst_mac(0xFFFFFFFFFFFF);
    match->mutable_internal_mask()->set_outer_dst_mac(0xFFFFFFFFFFFF);
#if 0
    match->mutable_eth_selector()->set_dst_mac(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
#endif

    HAL_TRACE_DEBUG("Installing ACL for bcast-all");
    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install bcast all. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_bcast_all (if_t *hal_if)
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;
    uint32_t            if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_BCAST_ALL + if_idx;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    HAL_TRACE_DEBUG("UnInstalling ACL for bcast-all");
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
acl_install_bcast_arp (if_t *hal_if)
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    uint32_t         acl_id;
    uint32_t         priority;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_BCAST_ARP + if_idx;
    priority = ACL_NCSI_BCAST_ARP_PRIORITY + if_idx;

    match = spec.mutable_match();
    action = spec.mutable_action();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_src_if_key_handle()->set_interface_id(hal_if->if_id);
    match->mutable_eth_selector()->set_dst_mac(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_ARP);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);

    HAL_TRACE_DEBUG("Installing ACL for bcast-arp");
    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install bcast arp. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_bcast_arp (if_t *hal_if)
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_BCAST_ARP + if_idx;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    HAL_TRACE_DEBUG("UnInstalling ACL for bcast-arp");
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
acl_install_bcast_dhcp_client (if_t *hal_if)
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);


    acl_id = ACL_NCSI_BCAST_DHCP_CLIENT + if_idx;
    priority = ACL_NCSI_BCAST_DHCP_CLIENT_PRIORITY + if_idx;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_src_if_key_handle()->set_interface_id(hal_if->if_id);
    match->mutable_internal_key()->set_outer_dst_mac(0xFFFFFFFFFFFF);
    match->mutable_internal_mask()->set_outer_dst_mac(0xFFFFFFFFFFFF);
#if 0
    match->mutable_eth_selector()->set_dst_mac(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV4);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
#endif
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_low(DHCP_SERVER_PORT);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_high(DHCP_SERVER_PORT);

    HAL_TRACE_DEBUG("Installing ACL for dhcp-client");
    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install bcast dhcp client. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_bcast_dhcp_client (if_t *hal_if)
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;
    uint32_t            if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_BCAST_DHCP_CLIENT + if_idx;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    HAL_TRACE_DEBUG("UnInstalling ACL for dhcp-client");
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
acl_install_bcast_dhcp_server (if_t *hal_if)
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_BCAST_DHCP_SERVER + if_idx;
    priority = ACL_NCSI_BCAST_DHCP_SERVER_PRIORITY + if_idx;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_src_if_key_handle()->set_interface_id(hal_if->if_id);
    match->mutable_internal_key()->set_outer_dst_mac(0xFFFFFFFFFFFF);
    match->mutable_internal_mask()->set_outer_dst_mac(0xFFFFFFFFFFFF);
#if 0
    match->mutable_eth_selector()->set_dst_mac(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV4);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
#endif
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_low(DHCP_CLIENT_PORT);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_high(DHCP_CLIENT_PORT);

    HAL_TRACE_DEBUG("Installing ACL for dhcp-server");
    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install bcast dhcp server. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_bcast_dhcp_server (if_t *hal_if)
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_BCAST_DHCP_SERVER + if_idx;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    HAL_TRACE_DEBUG("UnInstalling ACL for dhcp-server");
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
acl_install_bcast_netbios (if_t *hal_if)
{
    hal_ret_t        ret;

    ret = acl_install_bcast_netbios_ns(hal_if);
    ret = acl_install_bcast_netbios_ds(hal_if);

    return ret;
}
hal_ret_t
acl_uninstall_bcast_netbios (if_t *hal_if)
{
    hal_ret_t        ret;

    ret = acl_uninstall_bcast_netbios_ns(hal_if);
    ret = acl_uninstall_bcast_netbios_ds(hal_if);

    return ret;
}

// ----------------------------------------------------------------------------
// Install Bcast Netbios nameservice
// ----------------------------------------------------------------------------
hal_ret_t
acl_install_bcast_netbios_ns (if_t *hal_if)
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);


    acl_id = ACL_NCSI_BCAST_NETBIOS_NS + if_idx;
    priority = ACL_NCSI_BCAST_NETBIOS_NS_PRIORITY + if_idx;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_src_if_key_handle()->set_interface_id(hal_if->if_id);
    match->mutable_internal_key()->set_outer_dst_mac(0xFFFFFFFFFFFF);
    match->mutable_internal_mask()->set_outer_dst_mac(0xFFFFFFFFFFFF);
#if 0
    match->mutable_eth_selector()->set_dst_mac(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV4);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
#endif
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_low(NETBIOS_NS_PORT);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_high(NETBIOS_NS_PORT);

    HAL_TRACE_DEBUG("Installing ACL for netbios-ns");
    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install netbios ns. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_bcast_netbios_ns (if_t *hal_if)
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_BCAST_NETBIOS_NS + if_idx;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    HAL_TRACE_DEBUG("UnInstalling ACL for netbios-ns");
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
acl_install_bcast_netbios_ds (if_t *hal_if)
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);


    acl_id = ACL_NCSI_BCAST_NETBIOS_DS + if_idx;
    priority = ACL_NCSI_BCAST_NETBIOS_DS_PRIORITY + if_idx;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_src_if_key_handle()->set_interface_id(hal_if->if_id);
    match->mutable_internal_key()->set_outer_dst_mac(0xFFFFFFFFFFFF);
    match->mutable_internal_mask()->set_outer_dst_mac(0xFFFFFFFFFFFF);
#if 0
    match->mutable_eth_selector()->set_dst_mac(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV4);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
#endif
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_low(NETBIOS_DS_PORT);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_high(NETBIOS_DS_PORT);

    HAL_TRACE_DEBUG("Installing ACL for netbios-ds");
    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install netbios ds. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_bcast_netbios_ds (if_t *hal_if)
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_BCAST_NETBIOS_DS + if_idx;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    HAL_TRACE_DEBUG("UnInstalling ACL for netbios-ds");
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
acl_install_mcast_ipv6_nadv (if_t *hal_if)
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);


    acl_id = ACL_NCSI_MCAST_IPV6_NADV + if_idx;
    priority = ACL_NCSI_MCAST_IPV6_NADV_PRIORITY + if_idx;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_src_if_key_handle()->set_interface_id(hal_if->if_id);
    match->mutable_internal_key()->set_outer_dst_mac(0x333300000001);
    match->mutable_internal_mask()->set_outer_dst_mac(0xFFFFFFFFFFFF);
#if 0
    match->mutable_eth_selector()->set_dst_mac(0x333300000001);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV6);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
#endif
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
    ip_select->mutable_icmp_selector()->set_icmp_code(0);
    ip_select->mutable_icmp_selector()->set_icmp_code_mask(0xff);
    ip_select->mutable_icmp_selector()->set_icmp_type(ICMP_NEIGHBOR_ADVERTISEMENT);
    ip_select->mutable_icmp_selector()->set_icmp_type_mask(0xff);

    HAL_TRACE_DEBUG("Installing ACL for NA");
    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install neigh adv. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_ipv6_nadv (if_t *hal_if)
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_MCAST_IPV6_NADV + if_idx;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    HAL_TRACE_DEBUG("UnInstalling ACL for NA");
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
acl_install_mcast_ipv6_radv (if_t *hal_if)
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_MCAST_IPV6_RADV + if_idx;
    priority = ACL_NCSI_MCAST_IPV6_RADV_PRIORITY + if_idx;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_src_if_key_handle()->set_interface_id(hal_if->if_id);
    match->mutable_internal_key()->set_outer_dst_mac(0x333300000001);
    match->mutable_internal_mask()->set_outer_dst_mac(0xFFFFFFFFFFFF);
#if 0
    match->mutable_eth_selector()->set_dst_mac(0x333300000001);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV6);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
#endif
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
    ip_select->mutable_icmp_selector()->set_icmp_code(0);
    ip_select->mutable_icmp_selector()->set_icmp_code_mask(0xff);
    ip_select->mutable_icmp_selector()->set_icmp_type(ICMP_ROUTER_ADVERTISEMENT);
    ip_select->mutable_icmp_selector()->set_icmp_type_mask(0xff);

    HAL_TRACE_DEBUG("Installing ACL for RA");
    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install router adv. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_ipv6_radv (if_t *hal_if)
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;
    uint32_t            if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_MCAST_IPV6_RADV + if_idx;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    HAL_TRACE_DEBUG("UnInstalling ACL for RA");
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
acl_install_mcast_dhcpv6_relay (if_t *hal_if)
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_MCAST_DHCPV6_RELAY + if_idx;
    priority = ACL_NCSI_MCAST_DHCPV6_RELAY_PRIORITY + if_idx;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_src_if_key_handle()->set_interface_id(hal_if->if_id);
    match->mutable_internal_key()->set_outer_dst_mac(0x333300010002);
    match->mutable_internal_mask()->set_outer_dst_mac(0xFFFFFFFFFFFE);
#if 0
    match->mutable_eth_selector()->set_dst_mac(0x333300010002);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFE);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV6);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
#endif
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_low(DHCPV6_RELAY_PORT);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_high(DHCPV6_RELAY_PORT);

    HAL_TRACE_DEBUG("Installing ACL for dhcpv6-relay");
    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install dhcpv6 relay. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_dhcpv6_relay (if_t *hal_if)
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_MCAST_DHCPV6_RELAY + if_idx;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    HAL_TRACE_DEBUG("UnInstalling ACL for dhcpv6-relay");
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
acl_install_mcast_dhcpv6_mcast (if_t *hal_if)
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_MCAST_DHCPV6_MCAST + if_idx;
    priority = ACL_NCSI_MCAST_DHCPV6_MCAST_PRIORITY + if_idx;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_src_if_key_handle()->set_interface_id(hal_if->if_id);
    match->mutable_internal_key()->set_outer_dst_mac(0x333300010002);
    match->mutable_internal_mask()->set_outer_dst_mac(0xFFFFFFFFFFFF);
#if 0
    match->mutable_eth_selector()->set_dst_mac(0x333300010002);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV6);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
#endif
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_low(DHCPV6_MCAST_PORT);
    ip_select->mutable_udp_selector()->
        mutable_dst_port_range()->set_port_high(DHCPV6_MCAST_PORT);

    HAL_TRACE_DEBUG("Installing ACL for dhcpv6-mcast");
    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install dhcpv6 mcast.  err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_dhcpv6_mcast (if_t *hal_if)
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;
    uint32_t            if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_MCAST_DHCPV6_MCAST + if_idx;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    HAL_TRACE_DEBUG("UnInstalling ACL for dhcpv6-mcast");
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
acl_install_mcast_mld (if_t *hal_if)
{
    hal_ret_t ret;

    ret = acl_install_mcast_mld_lq(hal_if);
    ret = acl_install_mcast_mld_lr(hal_if);
    ret = acl_install_mcast_mld_ld(hal_if);

    return ret;
}
hal_ret_t
acl_uninstall_mcast_mld (if_t *hal_if)
{
    hal_ret_t ret;

    ret = acl_uninstall_mcast_mld_lq(hal_if);
    ret = acl_uninstall_mcast_mld_lr(hal_if);
    ret = acl_uninstall_mcast_mld_ld(hal_if);

    return ret;
}
hal_ret_t
acl_install_mcast_mld_lq (if_t *hal_if)
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_MCAST_IPV6_MLD_LQ + if_idx;
    priority = ACL_NCSI_MCAST_IPV6_MLD_LQ_PRIORITY + if_idx;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_src_if_key_handle()->set_interface_id(hal_if->if_id);
    match->mutable_internal_key()->set_outer_dst_mac(0x333300000001);
    match->mutable_internal_mask()->set_outer_dst_mac(0xFFFFFFFFFFFF);
#if 0
    match->mutable_eth_selector()->set_dst_mac(0x333300000001);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV6);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
#endif
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
    ip_select->mutable_icmp_selector()->set_icmp_code(0);
    ip_select->mutable_icmp_selector()->set_icmp_code_mask(0xff);
    ip_select->mutable_icmp_selector()->set_icmp_type(ICMP_MCAST_LISTENER_QUERY);
    ip_select->mutable_icmp_selector()->set_icmp_type_mask(0xff);

    HAL_TRACE_DEBUG("Installing ACL for mld-lq");
    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install mld lq. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_mld_lq (if_t *hal_if)
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;
    uint32_t            if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_MCAST_IPV6_MLD_LQ + if_idx;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    HAL_TRACE_DEBUG("UnInstalling ACL for mld-lq");
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
acl_install_mcast_mld_lr (if_t *hal_if)
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_MCAST_IPV6_MLD_LR + if_idx;
    priority = ACL_NCSI_MCAST_IPV6_MLD_LR_PRIORITY + if_idx;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_src_if_key_handle()->set_interface_id(hal_if->if_id);
    match->mutable_internal_key()->set_outer_dst_mac(0x333300000001);
    match->mutable_internal_mask()->set_outer_dst_mac(0xFFFFFFFFFFFF);
#if 0
    match->mutable_eth_selector()->set_dst_mac(0x333300000001);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV6);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
#endif
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
    ip_select->mutable_icmp_selector()->set_icmp_code(0);
    ip_select->mutable_icmp_selector()->set_icmp_code_mask(0xff);
    ip_select->mutable_icmp_selector()->set_icmp_type(ICMP_MCAST_LISTENER_REPORT);
    ip_select->mutable_icmp_selector()->set_icmp_type_mask(0xff);

    HAL_TRACE_DEBUG("Installing ACL for mld-lr");
    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install mld lr. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_mld_lr (if_t *hal_if)
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;
    uint32_t            if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_MCAST_IPV6_MLD_LR + if_idx;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    HAL_TRACE_DEBUG("UnInstalling ACL for mld-lr");
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
acl_install_mcast_mld_ld (if_t *hal_if)
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_MCAST_IPV6_MLD_LD + if_idx;
    priority = ACL_NCSI_MCAST_IPV6_MLD_LD_PRIORITY + if_idx;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_src_if_key_handle()->set_interface_id(hal_if->if_id);
    match->mutable_internal_key()->set_outer_dst_mac(0x333300000001);
    match->mutable_internal_mask()->set_outer_dst_mac(0xFFFFFFFFFFFF);
#if 0
    match->mutable_eth_selector()->set_dst_mac(0x333300000001);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFFFFFFFF);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV6);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
#endif
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
    ip_select->mutable_icmp_selector()->set_icmp_code(0);
    ip_select->mutable_icmp_selector()->set_icmp_code_mask(0xff);
    ip_select->mutable_icmp_selector()->set_icmp_type(ICMP_MCAST_LISTENER_DONE);
    ip_select->mutable_icmp_selector()->set_icmp_type_mask(0xff);

    HAL_TRACE_DEBUG("Installing ACL for mld-ld");
    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install mld ld. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_mld_ld (if_t *hal_if)
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;
    uint32_t            if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_MCAST_IPV6_MLD_LD + if_idx;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    HAL_TRACE_DEBUG("UnInstalling ACL for mld-ld");
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
acl_install_mcast_ipv6_nsol (if_t *hal_if)
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    IPSelector       *ip_select;
    uint32_t         acl_id;
    uint32_t         priority;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_MCAST_IPV6_NSOL + if_idx;
    priority = ACL_NCSI_MCAST_IPV6_NSOL_PRIORITY + if_idx;

    match = spec.mutable_match();
    action = spec.mutable_action();
    ip_select = match->mutable_ip_selector();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);

    // Selector
    match->mutable_src_if_key_handle()->set_interface_id(hal_if->if_id);
    match->mutable_internal_key()->set_outer_dst_mac(0x3333FF000000);
    match->mutable_internal_mask()->set_outer_dst_mac(0xFFFFFF000000);
#if 0
    match->mutable_eth_selector()->set_dst_mac(0x3333FF000000);
    match->mutable_eth_selector()->set_dst_mac_mask(0xFFFFFF000000);
    match->mutable_eth_selector()->set_eth_type(ETH_TYPE_IPV6);
    match->mutable_eth_selector()->set_eth_type_mask(0xffff);
#endif
    ip_select->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
    ip_select->mutable_icmp_selector()->set_icmp_code(0);
    ip_select->mutable_icmp_selector()->set_icmp_code_mask(0xff);
    ip_select->mutable_icmp_selector()->set_icmp_type(ICMP_NEIGHBOR_SOLICITATION);
    ip_select->mutable_icmp_selector()->set_icmp_type_mask(0xff);

    HAL_TRACE_DEBUG("Installing ACL for ipv6-nsol");
    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install ipv6 nsol. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_ipv6_nsol (if_t *hal_if)
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;
    uint32_t            if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_MCAST_IPV6_NSOL + if_idx;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    HAL_TRACE_DEBUG("UnInstalling ACL for ipv6-nsol");
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
acl_install_mcast_all (if_t *hal_if)
{
    hal_ret_t        ret;
    AclSpec          spec;
    AclResponse      rsp;
    AclSelector      *match;
    AclActionInfo    *action;
    uint32_t         acl_id;
    uint32_t         priority;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_MCAST_ALL + if_idx;
    priority = ACL_NCSI_MCAST_ALL_PRIORITY + if_idx;

    match = spec.mutable_match();
    action = spec.mutable_action();
    spec.mutable_key_or_handle()->set_acl_id(acl_id);
    spec.set_priority(priority);

    // Action
    action->set_action(acl::AclAction::ACL_ACTION_PERMIT);
    action->set_egress_drop(true);

    // Selector
    /*
     * Mcast/Bcast packets coming from host pinned to this interface
     * are getting replicated to swm lif. We have to prevent that traffic.
     * So all traffic coming from anywhere will hit this entry but only 
     * traffic trying to go to swm enics will get dropped.
     */
    // match->mutable_src_if_key_handle()->set_interface_id(hal_if->if_id);
    match->mutable_internal_key()->set_outer_dst_mac(0x010000000000);
    match->mutable_internal_mask()->set_outer_dst_mac(0x010000000000);
#if 0
    match->mutable_eth_selector()->set_dst_mac(0x010000000000);
    match->mutable_eth_selector()->set_dst_mac_mask(0x010000000000);
#endif

    HAL_TRACE_DEBUG("Installing ACL for mcast-all");
    ret = hal::acl_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to install mcast all. err: {}", ret);
    }
    return ret;
}
hal_ret_t
acl_uninstall_mcast_all (if_t *hal_if)
{
    hal_ret_t           ret;
    AclDeleteRequest    req;
    AclDeleteResponse   rsp;
    uint32_t            acl_id;
    uint32_t         if_idx;

    if_idx = uplink_if_get_idx(hal_if);

    acl_id = ACL_NCSI_MCAST_ALL + if_idx;
    req.mutable_key_or_handle()->set_acl_id(acl_id);

    HAL_TRACE_DEBUG("UnInstalling ACL for mcast-all");
    ret = hal::acl_delete(req, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to uninstall mcast all. err: {}", ret);
    }
    return ret;
}

hal_ret_t
hal_acl_micro_seg_init (void)
{
    hal_ret_t ret = HAL_RET_OK;

    hal::hal_cfg_db_open(CFG_OP_WRITE);

    ret = hal::aclqos::hal_smart_nic_acl_config_init();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error creating smart nic acl entries ret {}", ret);
        goto end;
    }

    ret = hal::aclqos::hal_eplearn_acl_config_init();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Eplearn acl entry create failed ret {}", ret);
        goto end;
    }
    HAL_TRACE_DEBUG("Eplearn acl entry created");

end:
    hal::hal_cfg_db_close();
    return ret;
}

hal_ret_t
hal_acl_micro_seg_deinit (void)
{
    hal_ret_t ret = HAL_RET_OK;

    hal::hal_cfg_db_open(CFG_OP_WRITE);

    ret = hal::aclqos::hal_smart_nic_acl_config_deinit();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error removing smart nic acl entries ret {}", ret);
        goto end;
    }

    ret = hal::aclqos::hal_eplearn_acl_config_deinit();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Eplearn acl entry remove failed ret {}", ret);
        goto end;
    }

end:
    hal::hal_cfg_db_close();
    return ret;
}


} // namespace hal

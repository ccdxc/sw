//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// ACL APIs exported by PI to PD.
//-----------------------------------------------------------------------------

#ifndef __ACL_API_HPP__
#define __ACL_API_HPP__

namespace hal {

using hal::acl_t;

// Acl APIs
acl_match_spec_t *acl_get_match_spec(acl_t *pi_acl);
acl_action_spec_t *acl_get_action_spec(acl_t *pi_acl);
uint32_t acl_get_priority(acl_t *pi_acl);

hal_ret_t acl_install_bcast_all(if_t *hal_if);
hal_ret_t acl_install_bcast_arp(if_t *hal_if);
hal_ret_t acl_install_bcast_dhcp_client(if_t *hal_if);
hal_ret_t acl_install_bcast_dhcp_server(if_t *hal_if);
hal_ret_t acl_install_bcast_netbios(if_t *hal_if);
hal_ret_t acl_install_bcast_netbios_ns(if_t *hal_if);
hal_ret_t acl_install_bcast_netbios_ds(if_t *hal_if);
hal_ret_t acl_install_mcast_ipv6_nadv(if_t *hal_if);
hal_ret_t acl_install_mcast_ipv6_radv(if_t *hal_if);
hal_ret_t acl_install_mcast_dhcpv6_relay(if_t *hal_if);
hal_ret_t acl_install_mcast_dhcpv6_mcast(if_t *hal_if);
hal_ret_t acl_install_mcast_mld(if_t *hal_if);
hal_ret_t acl_install_mcast_mld_lq(if_t *hal_if);
hal_ret_t acl_install_mcast_mld_lr(if_t *hal_if);
hal_ret_t acl_install_mcast_mld_ld(if_t *hal_if);
hal_ret_t acl_install_mcast_ipv6_nsol(if_t *hal_if);
hal_ret_t acl_install_mcast_all(if_t *hal_if);
hal_ret_t acl_install_ncsi_redirect(if_t *oob_mnic_enic, if_t *oob_uplink_if);

hal_ret_t acl_uninstall_bcast_all(if_t *hal_if);
hal_ret_t acl_uninstall_bcast_arp(if_t *hal_if);
hal_ret_t acl_uninstall_bcast_dhcp_client(if_t *hal_if);
hal_ret_t acl_uninstall_bcast_dhcp_server(if_t *hal_if);
hal_ret_t acl_uninstall_bcast_netbios(if_t *hal_if);
hal_ret_t acl_uninstall_bcast_netbios_ns(if_t *hal_if);
hal_ret_t acl_uninstall_bcast_netbios_ds(if_t *hal_if);
hal_ret_t acl_uninstall_mcast_ipv6_nadv(if_t *hal_if);
hal_ret_t acl_uninstall_mcast_ipv6_radv(if_t *hal_if);
hal_ret_t acl_uninstall_mcast_dhcpv6_relay(if_t *hal_if);
hal_ret_t acl_uninstall_mcast_dhcpv6_mcast(if_t *hal_if);
hal_ret_t acl_uninstall_mcast_mld(if_t *hal_if);
hal_ret_t acl_uninstall_mcast_mld_lq(if_t *hal_if);
hal_ret_t acl_uninstall_mcast_mld_lr(if_t *hal_if);
hal_ret_t acl_uninstall_mcast_mld_ld(if_t *hal_if);
hal_ret_t acl_uninstall_mcast_ipv6_nsol(if_t *hal_if);
hal_ret_t acl_uninstall_mcast_all(if_t *hal_if);
hal_ret_t acl_uninstall_ncsi_redirect(void);

// Micro seg init
hal_ret_t hal_acl_micro_seg_init(void);
}    // namespace hal

#endif    // __ACL_API_HPP__


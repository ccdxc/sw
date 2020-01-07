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

hal_ret_t acl_install_bcast_all();
hal_ret_t acl_install_bcast_arp();
hal_ret_t acl_install_bcast_dhcp_client();
hal_ret_t acl_install_bcast_dhcp_server();
hal_ret_t acl_install_bcast_netbios();
hal_ret_t acl_install_bcast_netbios_ns();
hal_ret_t acl_install_bcast_netbios_ds();
hal_ret_t acl_install_mcast_ipv6_nadv();
hal_ret_t acl_install_mcast_ipv6_radv();
hal_ret_t acl_install_mcast_dhcpv6_relay();
hal_ret_t acl_install_mcast_dhcpv6_mcast();
hal_ret_t acl_install_mcast_mld();
hal_ret_t acl_install_mcast_mld_lq();
hal_ret_t acl_install_mcast_mld_lr();
hal_ret_t acl_install_mcast_mld_ld();
hal_ret_t acl_install_mcast_ipv6_nsol();
hal_ret_t acl_install_mcast_all();

hal_ret_t acl_uninstall_bcast_all();
hal_ret_t acl_uninstall_bcast_arp();
hal_ret_t acl_uninstall_bcast_dhcp_client();
hal_ret_t acl_uninstall_bcast_dhcp_server();
hal_ret_t acl_uninstall_bcast_netbios();
hal_ret_t acl_uninstall_bcast_netbios_ns();
hal_ret_t acl_uninstall_bcast_netbios_ds();
hal_ret_t acl_uninstall_mcast_ipv6_nadv();
hal_ret_t acl_uninstall_mcast_ipv6_radv();
hal_ret_t acl_uninstall_mcast_dhcpv6_relay();
hal_ret_t acl_uninstall_mcast_dhcpv6_mcast();
hal_ret_t acl_uninstall_mcast_mld();
hal_ret_t acl_uninstall_mcast_mld_lq();
hal_ret_t acl_uninstall_mcast_mld_lr();
hal_ret_t acl_uninstall_mcast_mld_ld();
hal_ret_t acl_uninstall_mcast_ipv6_nsol();
hal_ret_t acl_uninstall_mcast_all();

}    // namespace hal

#endif    // __ACL_API_HPP__


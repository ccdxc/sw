#ifndef __TEST_HPP_
#define __TEST_HPP_

#include <stdio.h>
#include <getopt.h>
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"

sdk_ret_t create_route_tables(uint32_t num_teps, uint32_t num_vpcs,
                              uint32_t num_subnets, uint32_t num_routes,
                              ip_prefix_t *tep_pfx, ip_prefix_t *route_pfx,
                              ip_prefix_t *v6_route_pfx);
sdk_ret_t create_mappings(uint32_t num_teps, uint32_t num_vpcs,
                          uint32_t num_subnets, uint32_t num_vnics,
                          uint32_t num_ip_per_vnic, ip_prefix_t *teppfx,
                          ip_prefix_t *natpfx, ip_prefix_t *v6_natpfx,
                 uint32_t num_remote_mappings);
sdk_ret_t create_vnics(uint32_t num_vpcs, uint32_t num_subnets,
                       uint32_t num_vnics, uint16_t vlan_start);
sdk_ret_t create_subnets(uint32_t vpc_id, uint32_t num_subnets,
                         ip_prefix_t *vpc_pfx);
sdk_ret_t create_vpcs(uint32_t num_vpcs, ip_prefix_t *ip_pfx,
                      uint32_t num_subnets);
sdk_ret_t create_teps(uint32_t num_teps, ip_prefix_t *ip_pfx);
sdk_ret_t create_device_cfg(ipv4_addr_t ipaddr, uint64_t macaddr,
                            ipv4_addr_t gwip);
sdk_ret_t create_security_policy(uint32_t num_vpcs, uint32_t num_subnets,
                                 uint32_t num_rules, uint32_t ip_af,
                                 bool ingress);
sdk_ret_t create_flows(uint32_t num_tcp, uint32_t num_udp, uint32_t num_icmp,
                       uint16_t sport_base, uint16_t dport_base);
sdk_ret_t create_objects(void);
sdk_ret_t delete_objects(void);

#endif

//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------

#include <iostream>
#include "nic/apollo/test/scale/test.hpp"
#include "nic/apollo/include/api/pds_tep.hpp"
#include "nic/apollo/include/api/pds_vcn.hpp"
#include "nic/apollo/include/api/pds_subnet.hpp"
#include "nic/apollo/include/api/pds_vnic.hpp"
#include "nic/apollo/include/api/pds_mapping.hpp"
#include "nic/apollo/include/api/pds_policy.hpp"
#include "nic/apollo/include/api/pds_device.hpp"
#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/apollo/agent/test/scale/app.hpp"
#ifndef TEST_GRPC_APP
#include "nic/apollo/test/flow_test/flow_test.hpp"
#endif

#define VNID_BASE                      1000

using std::string;
namespace pt = boost::property_tree;

extern char *g_input_cfg_file;
pds_device_spec_t g_device = {0};
#ifndef TEST_GRPC_APP
flow_test *g_flow_test_obj;
#endif

typedef struct test_params_s {
    // device config
    struct {
        uint32_t device_ip;
        uint64_t device_mac;
        uint32_t device_gw_ip;
        pds_tep_encap_type_t tep_encap;
    };
    // TEP config
    struct {
        uint32_t num_teps;
        ip_prefix_t tep_pfx;
    };
    // route config
    struct {
        uint32_t num_routes;
        ip_prefix_t route_pfx;
        ip_prefix_t v6_route_pfx;
    };
    // policy config
    struct {
        uint32_t num_rules;
    };
    // vcn config
    struct {
        uint32_t num_vcns;
        ip_prefix_t vcn_pfx;
        ip_prefix_t v6_vcn_pfx;
        uint32_t num_subnets;
    };
    // vnic config
    struct {
        uint32_t num_vnics;
        uint32_t vlan_start;
    };
    // mapping config
    struct {
        ip_prefix_t nat_pfx;
        ip_prefix_t v6_nat_pfx;
        uint32_t num_ip_per_vnic;
        uint32_t num_remote_mappings;
    };
    // flow config
    struct {
        uint32_t num_tcp;
        uint32_t num_udp;
        uint32_t num_icmp;
        uint16_t sport_base;
        uint16_t dport_base;
    };
} test_params_t;
test_params_t g_test_params = { 0 };

#define CONVERT_TO_V4_MAPPED_V6_ADDRESS(_v6pfx, _v4addr) {\
    _v6pfx.addr8[12] = (_v4addr >> 24) & 0xFF;\
    _v6pfx.addr8[13] = (_v4addr >> 16) & 0xFF;\
    _v6pfx.addr8[14] = (_v4addr >> 8) & 0xFF;\
    _v6pfx.addr8[15] = (_v4addr) & 0xFF;\
}

//----------------------------------------------------------------------------
// create route tables
//------------------------------------------------------------------------------
sdk_ret_t
create_route_tables (uint32_t num_teps, uint32_t num_vcns, uint32_t num_subnets,
                     uint32_t num_routes, ip_prefix_t *tep_pfx,
                     ip_prefix_t *route_pfx, ip_prefix_t *v6_route_pfx)
{
    uint32_t ntables = num_vcns * num_subnets;
    uint32_t tep_offset = 3;
    static uint32_t rtnum = 0;
    static uint32_t v6rtnum = 0;
    pds_route_table_spec_t route_table;
    pds_route_table_spec_t v6route_table;
    sdk_ret_t rv = SDK_RET_OK;

    route_table.af = IP_AF_IPV4;
    route_table.routes =
        (pds_route_t *)malloc((num_routes * sizeof(pds_route_t)));
    route_table.num_routes = num_routes;
    for (uint32_t i = 1; i <= ntables; i++) {
        route_table.key.id = i;
        for (uint32_t j = 0; j < num_routes; j++) {
            route_table.routes[j].prefix.len = 24;
            route_table.routes[j].prefix.addr.af = IP_AF_IPV4;
            route_table.routes[j].prefix.addr.addr.v4_addr =
                ((0xC << 28) | (rtnum++ << 8));
            route_table.routes[j].nh_ip.af = IP_AF_IPV4;
            route_table.routes[j].nh_ip.addr.v4_addr =
                tep_pfx->addr.addr.v4_addr + tep_offset++;

            tep_offset %= (num_teps + 3);
            if (tep_offset == 0) {
                // skip MyTEP and gateway IPs
                tep_offset += 3;
            }
            route_table.routes[j].nh_type = PDS_NH_TYPE_REMOTE_TEP;
            route_table.routes[j].vcn_id = PDS_VCN_ID_INVALID;
        }
#ifdef TEST_GRPC_APP
        rv = create_route_table_grpc(&route_table);
        if (rv != SDK_RET_OK) {
            return rv;
        }
#else
        rv = pds_route_table_create(&route_table);
        if (rv != SDK_RET_OK) {
            return rv;
        }
#endif
    }

    tep_offset = 3;
    v6route_table.af = IP_AF_IPV6;
    v6route_table.routes =
            (pds_route_t *)malloc((num_routes * sizeof(pds_route_t)));
    v6route_table.num_routes = num_routes;
    for (uint32_t i = 1; i <= ntables; i++) {
        v6route_table.key.id = ntables + i;
        for (uint32_t j = 0; j < num_routes; j++) {
            v6route_table.routes[j].prefix = *v6_route_pfx;
            v6route_table.routes[j].prefix.addr.addr.v6_addr.addr32[IP6_ADDR32_LEN-1] =
                    htonl((0xC << 28) | (v6rtnum++ << 8));
            v6route_table.routes[j].prefix.len = 120;

            v6route_table.routes[j].nh_ip.af = IP_AF_IPV4;
            v6route_table.routes[j].nh_ip.addr.v4_addr =
                    tep_pfx->addr.addr.v4_addr + tep_offset++;

            tep_offset %= (num_teps + 3);
            if (tep_offset == 0) {
                // skip MyTEP and gateway IPs
                tep_offset += 3;
            }

            v6route_table.routes[j].nh_type = PDS_NH_TYPE_REMOTE_TEP;
            v6route_table.routes[j].vcn_id = PDS_VCN_ID_INVALID;
        }

#ifdef TEST_GRPC_APP
        rv = create_route_table_grpc(&v6route_table);
        if (rv != SDK_RET_OK) {
            return rv;
        }
#else
        rv = pds_route_table_create(&v6route_table);
        if (rv != SDK_RET_OK) {
            return rv;
        }
#endif
    }

    return rv;
}

//----------------------------------------------------------------------------
// 1. create 1 primary + 32 secondary IP for each of 1K local vnics
// 2. create 1023 remote mappings per VCN
//------------------------------------------------------------------------------
sdk_ret_t
create_mappings (uint32_t num_teps, uint32_t num_vcns, uint32_t num_subnets,
                 uint32_t num_vnics, uint32_t num_ip_per_vnic,
                 ip_prefix_t *teppfx, ip_prefix_t *natpfx, ip_prefix_t *v6_natpfx,
                 uint32_t num_remote_mappings)
{
    sdk_ret_t rv;
    pds_mapping_spec_t pds_mapping;
    pds_mapping_spec_t pds_v6_mapping;
    uint16_t vnic_key = 1, ip_base;
    uint32_t ip_offset = 0, remote_slot = 1025;
    uint32_t tep_offset = 0, v6_tep_offset = 0;

    // ensure a max. of 32 IPs per VNIC
    SDK_ASSERT(num_vcns * num_subnets * num_vnics * num_ip_per_vnic <=
               (32 * 1024));
    // create local vnic IP mappings first
    for (uint32_t i = 1; i <= num_vcns; i++) {
        for (uint32_t j = 1; j <= num_subnets; j++) {
            for (uint32_t k = 1; k <= num_vnics; k++) {
                for (uint32_t l = 1; l <= num_ip_per_vnic; l++) {
                    memset(&pds_mapping, 0, sizeof(pds_mapping));
                    pds_mapping.key.vcn.id = i;
                    pds_mapping.key.ip_addr.af = IP_AF_IPV4;
                    pds_mapping.key.ip_addr.addr.v4_addr =
                        (g_test_params.vcn_pfx.addr.addr.v4_addr | ((j - 1) << 14)) |
                        (((k - 1) * num_ip_per_vnic) + l);
                    pds_mapping.subnet.id = (i - 1) * num_subnets + j;
                    if (g_test_params.tep_encap == PDS_TEP_ENCAP_TYPE_VXLAN) {
                        pds_mapping.fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
                        //pds_mapping.fabric_encap.val.vnid = VNID_BASE + pds_mapping.subnet.id;
                        pds_mapping.fabric_encap.val.vnid = vnic_key;
                    } else {
                        pds_mapping.fabric_encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
                        pds_mapping.fabric_encap.val.mpls_tag = vnic_key;
                    }
                    pds_mapping.tep.ip_addr = g_device.switch_ip_addr;
                    MAC_UINT64_TO_ADDR(pds_mapping.overlay_mac,
                                       (((((uint64_t)i & 0x7FF) << 22) |
                                         ((j & 0x7FF) << 11) | (k & 0x7FF))));
                    pds_mapping.vnic.id = vnic_key;
                    if (natpfx) {
                        pds_mapping.public_ip_valid = true;
                        pds_mapping.public_ip.addr.v4_addr =
                            natpfx->addr.addr.v4_addr + ip_offset++;
                    }

#ifdef TEST_GRPC_APP
                    rv = create_mapping_grpc(&pds_mapping);
                    if (rv != SDK_RET_OK) {
                        SDK_ASSERT(0);
                        return rv;
                    }
#else
                    rv = pds_mapping_create(&pds_mapping);
                    if (rv != SDK_RET_OK) {
                        SDK_ASSERT(0);
                        return rv;
                    }
                    g_flow_test_obj->add_local_ep(pds_mapping.key.vcn.id,
                                                  pds_mapping.key.ip_addr);
#endif
                    // V6 mapping
                    pds_v6_mapping = pds_mapping;
                    pds_v6_mapping.key.ip_addr.af = IP_AF_IPV6;
                    pds_v6_mapping.key.ip_addr.addr.v6_addr = 
                           g_test_params.v6_vcn_pfx.addr.addr.v6_addr;
                    CONVERT_TO_V4_MAPPED_V6_ADDRESS(pds_v6_mapping.key.ip_addr.addr.v6_addr,
                                                    pds_mapping.key.ip_addr.addr.v4_addr);
                    if (natpfx) {
                        pds_v6_mapping.public_ip.addr.v6_addr = v6_natpfx->addr.addr.v6_addr;
                        CONVERT_TO_V4_MAPPED_V6_ADDRESS(pds_v6_mapping.public_ip.addr.v6_addr,
                                                        pds_mapping.public_ip.addr.v4_addr);
                    }
#ifdef TEST_GRPC_APP
                    rv = create_mapping_grpc(&pds_v6_mapping);
                    if (rv != SDK_RET_OK) {
                        SDK_ASSERT(0);
                        return rv;
                    }
#else
                    rv = pds_mapping_create(&pds_v6_mapping);
                    if (rv != SDK_RET_OK) {
                        SDK_ASSERT(0);
                        return rv;
                    }
                    g_flow_test_obj->add_local_ep(pds_mapping.key.vcn.id,
                                                  pds_v6_mapping.key.ip_addr);
#endif
                }
                vnic_key++;
            }
        }
    }

    // create remote mappings
    SDK_ASSERT(num_vcns * num_remote_mappings <= (1 << 20));
    for (uint32_t i = 1; i <= num_vcns; i++) {
        tep_offset = 3;
        v6_tep_offset = tep_offset + num_teps / 2;
        for (uint32_t j = 1; j <= num_subnets; j++) {
            ip_base = num_vnics * num_ip_per_vnic + 1;
            for (uint32_t k = 1; k <= num_remote_mappings; k++) {
                memset(&pds_mapping, 0, sizeof(pds_mapping));
                pds_mapping.key.vcn.id = i;
                pds_mapping.key.ip_addr.af = IP_AF_IPV4;
                pds_mapping.key.ip_addr.addr.v4_addr =
                    (g_test_params.vcn_pfx.addr.addr.v4_addr | ((j - 1) << 14)) |
                    ip_base++;
                pds_mapping.subnet.id = (i - 1) * num_subnets + j;
                if (g_test_params.tep_encap == PDS_TEP_ENCAP_TYPE_VXLAN) {
                    pds_mapping.fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
                    //pds_mapping.fabric_encap.val.vnid = VNID_BASE + pds_mapping.subnet.id;
                    pds_mapping.fabric_encap.val.vnid = remote_slot++;
                } else {
                    pds_mapping.fabric_encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
                    pds_mapping.fabric_encap.val.mpls_tag = remote_slot++;
                }
                pds_mapping.tep.ip_addr =
                    teppfx->addr.addr.v4_addr + tep_offset;
                MAC_UINT64_TO_ADDR(
                    pds_mapping.overlay_mac,
                    (((((uint64_t)i & 0x7FF) << 22) | ((j & 0x7FF) << 11) |
                      ((num_vnics + k) & 0x7FF))));

#ifdef TEST_GRPC_APP
                rv = create_mapping_grpc(&pds_mapping);
                if (rv != SDK_RET_OK) {
                    return rv;
                }
#else
                rv = pds_mapping_create(&pds_mapping);
                if (rv != SDK_RET_OK) {
                    return rv;
                }
                g_flow_test_obj->add_remote_ep(pds_mapping.key.vcn.id,
                                               pds_mapping.key.ip_addr);
#endif
                // V6 mapping
                pds_v6_mapping = pds_mapping;
                pds_v6_mapping.key.ip_addr.af = IP_AF_IPV6;
                pds_v6_mapping.key.ip_addr.addr.v6_addr =
                      g_test_params.v6_vcn_pfx.addr.addr.v6_addr;
                CONVERT_TO_V4_MAPPED_V6_ADDRESS(pds_v6_mapping.key.ip_addr.addr.v6_addr,
                                                pds_mapping.key.ip_addr.addr.v4_addr);
                pds_v6_mapping.tep.ip_addr = teppfx->addr.addr.v4_addr + v6_tep_offset;
#ifdef TEST_GRPC_APP
                rv = create_mapping_grpc(&pds_v6_mapping);
                if (rv != SDK_RET_OK) {
                    return rv;
                }
#else
                rv = pds_mapping_create(&pds_v6_mapping);
                if (rv != SDK_RET_OK) {
                    return rv;
                }
                g_flow_test_obj->add_remote_ep(pds_mapping.key.vcn.id,
                                               pds_v6_mapping.key.ip_addr);
#endif
                tep_offset++;
                tep_offset %= num_teps;
                tep_offset = tep_offset ? tep_offset : 3;

                v6_tep_offset++;
                v6_tep_offset %= num_teps;
                v6_tep_offset = v6_tep_offset ? v6_tep_offset : 3;
            }
        }
    }
    return SDK_RET_OK;
}

// MAC address is encoded like below:
// bits 0-10 have vnic number in the subnet
// bits 11-21 have subnet number in the vcn
// bits 22-32 have vcn number
sdk_ret_t
create_vnics (uint32_t num_vcns, uint32_t num_subnets,
              uint32_t num_vnics, uint16_t vlan_start)
{
    sdk_ret_t rv = SDK_RET_OK;
    pds_vnic_spec_t pds_vnic;
    uint16_t vnic_key = 1;

    SDK_ASSERT(num_vcns * num_subnets * num_vnics <= 1024);
    for (uint32_t i = 1; i <= (uint64_t)num_vcns; i++) {
        for (uint32_t j = 1; j <= num_subnets; j++) {
            for (uint32_t k = 1; k <= num_vnics; k++) {
                memset(&pds_vnic, 0, sizeof(pds_vnic));
                pds_vnic.vcn.id = i;
                pds_vnic.subnet.id = (i - 1) * num_subnets + j;
                pds_vnic.key.id = vnic_key;
                pds_vnic.wire_vlan = vlan_start + vnic_key - 1;
                if (g_test_params.tep_encap == PDS_TEP_ENCAP_TYPE_VXLAN) {
                    pds_vnic.fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
                    //pds_vnic.fabric_encap.val.vnid = VNID_BASE + pds_vnic.subnet.id;
                    pds_vnic.fabric_encap.val.vnid = vnic_key;
                } else {
                    pds_vnic.fabric_encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
                    pds_vnic.fabric_encap.val.mpls_tag = vnic_key;
                }
                MAC_UINT64_TO_ADDR(pds_vnic.mac_addr,
                                   (((((uint64_t)i & 0x7FF) << 22) |
                                     ((j & 0x7FF) << 11) | (k & 0x7FF))));
                pds_vnic.rsc_pool_id = 1;
                pds_vnic.src_dst_check = false; //(k & 0x1);
#ifdef TEST_GRPC_APP
                rv = create_vnic_grpc(&pds_vnic);
                if (rv != SDK_RET_OK) {
                    return rv;
                }
#else
                rv = pds_vnic_create(&pds_vnic);
                if (rv != SDK_RET_OK) {
                    return rv;
                }
#endif
                vnic_key++;
            }
        }
    }

    return rv;
}

// VCN prefix is /8, subnet id is encoded in next 10 bits (making it /18 prefix)
// leaving LSB 14 bits for VNIC IPs
sdk_ret_t
create_subnets (uint32_t vcn_id, uint32_t num_vcns,
                uint32_t num_subnets, ip_prefix_t *vcn_pfx)
{
    sdk_ret_t rv;
    pds_subnet_spec_t pds_subnet;
    static uint32_t route_table_id = 1;
    static uint32_t        id = 1;

    for (uint32_t i = 1; i <= num_subnets; i++) {
        memset(&pds_subnet, 0, sizeof(pds_subnet));
        pds_subnet.key.id = (vcn_id - 1) * num_subnets + i;
        pds_subnet.vcn.id = vcn_id;
        pds_subnet.pfx = *vcn_pfx;
        pds_subnet.pfx.addr.addr.v4_addr =
            (pds_subnet.pfx.addr.addr.v4_addr) | ((i - 1) << 14);
        pds_subnet.pfx.len = 18;
        pds_subnet.vr_ip.af = IP_AF_IPV4;
        // pds_subnet.vr_ip.addr.v4_addr = pds_subnet.pfx.addr.addr.v4_addr |
        // 0x1;
        pds_subnet.vr_ip.addr.v4_addr = pds_subnet.pfx.addr.addr.v4_addr;
        MAC_UINT64_TO_ADDR(pds_subnet.vr_mac,
                           (uint64_t)pds_subnet.vr_ip.addr.v4_addr);
        pds_subnet.v6_route_table.id = route_table_id + (num_subnets * num_vcns);
        pds_subnet.v4_route_table.id = route_table_id++;
        pds_subnet.egr_v4_policy.id = id++;
#ifdef TEST_GRPC_APP
        rv = create_subnet_grpc(&pds_subnet);
        if (rv != SDK_RET_OK) {
            return rv;
        }
#else
        rv = pds_subnet_create(&pds_subnet);
        if (rv != SDK_RET_OK) {
            return rv;
        }
#endif
    }
    return SDK_RET_OK;
}

sdk_ret_t
create_vcns (uint32_t num_vcns, ip_prefix_t *ip_pfx, uint32_t num_subnets)
{
    sdk_ret_t rv;
    pds_vcn_spec_t pds_vcn;

    SDK_ASSERT(num_vcns <= 1024);
    for (uint32_t i = 1; i <= num_vcns; i++) {
        memset(&pds_vcn, 0, sizeof(pds_vcn));
        pds_vcn.type = PDS_VCN_TYPE_TENANT;
        pds_vcn.key.id = i;
        pds_vcn.pfx = *ip_pfx;
        pds_vcn.pfx.len = 8; // fix this to /8
        pds_vcn.pfx.addr.addr.v4_addr &= 0xFF000000;
#ifdef TEST_GRPC_APP
        rv = create_vcn_grpc(&pds_vcn);
        if (rv != SDK_RET_OK) {
            return rv;
        }
#else
        rv = pds_vcn_create(&pds_vcn);
        if (rv != SDK_RET_OK) {
            return rv;
        }
#endif
        for (uint32_t j = 1; j <= num_subnets; j++) {
            rv = create_subnets(i, num_vcns, j, &pds_vcn.pfx);
            if (rv != SDK_RET_OK) {
                return rv;
            }
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
create_teps (uint32_t num_teps, ip_prefix_t *ip_pfx)
{
    sdk_ret_t      rv;
    pds_tep_spec_t pds_tep;

    // leave the 1st IP in this prefix for MyTEP
    for (uint32_t i = 1; i <= num_teps; i++) {
        memset(&pds_tep, 0, sizeof(pds_tep));
        // 1st IP in the TEP prefix is gateway IP, 2nd is MyTEP IP,
        // so we skip the 1st (even for MyTEP we create a TEP)
        pds_tep.key.ip_addr = ip_pfx->addr.addr.v4_addr + 1 + i;
        if (g_test_params.tep_encap == PDS_TEP_ENCAP_TYPE_VXLAN) {
            pds_tep.encap_type = PDS_TEP_ENCAP_TYPE_VXLAN;
        } else {
            pds_tep.encap_type = PDS_TEP_ENCAP_TYPE_VNIC;
        }
#ifdef TEST_GRPC_APP
        rv = create_tunnel_grpc(&pds_tep);
        if (rv != SDK_RET_OK) {
            return rv;
        }
#else
        rv = pds_tep_create(&pds_tep);
        if (rv != SDK_RET_OK) {
            return rv;
        }
#endif
    }
    return SDK_RET_OK;
}

sdk_ret_t
create_device_cfg (ipv4_addr_t ipaddr, uint64_t macaddr, ipv4_addr_t gwip)
{
    sdk_ret_t            rv;
    memset(&g_device, 0, sizeof(g_device));
    g_device.switch_ip_addr = ipaddr;
    MAC_UINT64_TO_ADDR(g_device.switch_mac_addr, macaddr);
    g_device.gateway_ip_addr = gwip;

#ifdef TEST_GRPC_APP
    rv = create_switch_grpc(&g_device);
#else
    rv = pds_device_create(&g_device);
#endif

    return rv;
}

sdk_ret_t
create_security_policy (uint32_t num_vcns, uint32_t num_subnets,
                        uint32_t num_rules, uint32_t ip_af, bool ingress)
{
#ifndef TEST_GRPC_APP
    sdk_ret_t            rv;
    pds_policy_spec_t    policy;
    uint32_t             policy_id = 1;
    rule_t               *rule;

    if (num_rules == 0) {
        return SDK_RET_OK;
    }

    policy.policy_type = POLICY_TYPE_FIREWALL;
    policy.af = ip_af;
    policy.direction = ingress ? RULE_DIR_INGRESS : RULE_DIR_EGRESS;
    policy.num_rules = num_rules;
    policy.rules = (rule_t *)malloc(num_rules * sizeof(rule_t));
    for (uint32_t i = 1; i <= num_vcns; i++) {
        for (uint32_t j = 1; j <= num_subnets; j++) {
            memset(policy.rules, 0, num_rules * sizeof(rule_t));
            policy.key.id = policy_id++;
            for (uint32_t k = 0; k < num_rules; k++) {
                rule = &policy.rules[k];
                rule->stateful = false;
                rule->match.l3_match.ip_proto = 17;    // UDP
                rule->match.l3_match.ip_pfx = g_test_params.vcn_pfx;
                rule->match.l3_match.ip_pfx.addr.addr.v4_addr =
                    rule->match.l3_match.ip_pfx.addr.addr.v4_addr |
                    ((j - 1) << 14) | (k << 4);
                rule->match.l3_match.ip_pfx.len = 28;
                rule->match.l4_match.sport_range.port_lo = 0;
                rule->match.l4_match.sport_range.port_hi = 65535;
                rule->match.l4_match.dport_range.port_lo = 10000;
                rule->match.l4_match.dport_range.port_hi = 20000;
                rule->action_data.fw_action.action = SECURITY_RULE_ACTION_ALLOW;
            }
            rv = pds_policy_create(&policy);
            if (rv != SDK_RET_OK) {
                printf("Failed to create security policy, vcn %u, subnet %u, "
                       "err %u\n", i, j, rv);
                return rv;
            }
        }
    }
#endif
    return SDK_RET_OK;
}

static sdk_ret_t
create_flows (uint32_t num_tcp, uint32_t num_udp, uint32_t num_icmp,
              uint16_t sport_base, uint16_t dport_base, bool ipv6)
{
#ifndef TEST_GRPC_APP
    sdk_ret_t ret = SDK_RET_OK;

    if (num_tcp) {
        ret = g_flow_test_obj->create_flows(num_tcp, 6, sport_base,
                                            dport_base, ipv6);
        if (ret != SDK_RET_OK) {
            return ret;
        }
    }

    if (num_udp) {
        ret = g_flow_test_obj->create_flows(num_udp, 17, sport_base,
                                            dport_base, ipv6);
        if (ret != SDK_RET_OK) {
            return ret;
        }
    }

    if (num_icmp) {
        ret = g_flow_test_obj->create_flows(num_icmp, 1, 0, 0, ipv6);
        if (ret != SDK_RET_OK) {
            return ret;
        }
    }
#endif
    return SDK_RET_OK;
}

sdk_ret_t
create_objects (void)
{
    pt::ptree json_pt;
    string pfxstr;
    sdk_ret_t ret;
    
#ifndef TEST_GRPC_APP
    g_flow_test_obj = new flow_test();
#endif

    // parse the config and create objects
    std::ifstream json_cfg(g_input_cfg_file);
    read_json(json_cfg, json_pt);
    try {
        BOOST_FOREACH (pt::ptree::value_type &obj,
                       json_pt.get_child("objects")) {
            std::string kind = obj.second.get<std::string>("kind");
            if (kind == "device") {
                struct in_addr ipaddr, gwip;

                g_test_params.device_mac =
                    std::stoull(obj.second.get<std::string>("mac-addr"), 0, 0);

                inet_aton(obj.second.get<std::string>("ip-addr").c_str(),
                          &ipaddr);
                g_test_params.device_ip = ntohl(ipaddr.s_addr);

                inet_aton(obj.second.get<std::string>("gw-ip-addr").c_str(),
                          &gwip);
                g_test_params.device_gw_ip = ntohl(gwip.s_addr);

                if (!obj.second.get<std::string>("encap").compare("vxlan")) {
                    g_test_params.tep_encap = PDS_TEP_ENCAP_TYPE_VXLAN;
                } else {
                    g_test_params.tep_encap = PDS_TEP_ENCAP_TYPE_VNIC;
                }
            } else if (kind == "tep") {
                g_test_params.num_teps = std::stol(obj.second.get<std::string>("count"));
                if (g_test_params.num_teps <= 2) {
                    printf("No. of TEPs must be greater than 2\n");
                    exit(1);
                }
                // reduce num_teps by 2, (MyTEP and GW-TEP)
                g_test_params.num_teps -= 2;
                pfxstr = obj.second.get<std::string>("prefix");
                assert(str2ipv4pfx((char *)pfxstr.c_str(), &g_test_params.tep_pfx) == 0);
            } else if (kind == "route-table") {
                g_test_params.num_routes = std::stol(obj.second.get<std::string>("count"));
                g_test_params.num_vcns = std::stol(obj.second.get<std::string>("num_vcns", "1"));
                pfxstr = obj.second.get<std::string>("prefix-start");
                assert(str2ipv4pfx((char *)pfxstr.c_str(), &g_test_params.route_pfx) == 0);
                pfxstr = obj.second.get<std::string>("v6-prefix-start");
                assert(str2ipv6pfx((char *)pfxstr.c_str(), &g_test_params.v6_route_pfx) == 0);
            } else if (kind == "security-policy") {
                g_test_params.num_rules = std::stol(obj.second.get<std::string>("count"));
            } else if (kind == "vcn") {
                g_test_params.num_vcns = std::stol(obj.second.get<std::string>("count"));
                pfxstr = obj.second.get<std::string>("prefix");
                assert(str2ipv4pfx((char *)pfxstr.c_str(), &g_test_params.vcn_pfx) == 0);
                pfxstr = obj.second.get<std::string>("v6-prefix");
                assert(str2ipv6pfx((char *)pfxstr.c_str(), &g_test_params.v6_vcn_pfx) == 0);
                g_test_params.num_subnets = std::stol(obj.second.get<std::string>("subnets"));
            } else if (kind == "vnic") {
                g_test_params.num_vnics = std::stol(obj.second.get<std::string>("count"));
                g_test_params.vlan_start =
                    std::stol(obj.second.get<std::string>("vlan-start"));
            } else if (kind == "mappings") {
                pfxstr = obj.second.get<std::string>("nat-prefix");
                assert(str2ipv4pfx((char *)pfxstr.c_str(), &g_test_params.nat_pfx) == 0);
                pfxstr = obj.second.get<std::string>("v6-nat-prefix");
                assert(str2ipv6pfx((char *)pfxstr.c_str(), &g_test_params.v6_nat_pfx) == 0);
                g_test_params.num_remote_mappings =
                    std::stol(obj.second.get<std::string>("remotes"));
                g_test_params.num_ip_per_vnic =
                    std::stol(obj.second.get<std::string>("locals"));
            } else if (kind == "flows") {
                g_test_params.num_tcp = std::stol(obj.second.get<std::string>("num_tcp"));
                g_test_params.num_udp = std::stol(obj.second.get<std::string>("num_udp"));
                g_test_params.num_icmp = std::stol(obj.second.get<std::string>("num_icmp"));
                g_test_params.sport_base = std::stol(obj.second.get<std::string>("sport_base"));
                g_test_params.dport_base = std::stol(obj.second.get<std::string>("dport_base"));
            }
        }
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }

    // create device config
    ret = create_device_cfg(g_test_params.device_ip, g_test_params.device_mac,
                            g_test_params.device_gw_ip);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // create TEPs including MyTEP
    ret = create_teps(g_test_params.num_teps + 1, &g_test_params.tep_pfx);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // create route tables
    ret = create_route_tables(g_test_params.num_teps, g_test_params.num_vcns,
                              g_test_params.num_subnets, g_test_params.num_routes,
                              &g_test_params.tep_pfx, &g_test_params.route_pfx,
                              &g_test_params.v6_route_pfx);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // create security policies
    ret = create_security_policy(g_test_params.num_vcns, g_test_params.num_subnets,
                                 g_test_params.num_rules, IP_AF_IPV4, false);
    if (ret != SDK_RET_OK) {
        return ret;
    }
#if 0
    ret = create_security_policy(g_test_params.num_vcns, g_test_params.num_subnets,
                                 g_test_params.num_rules, IP_AF_IPV4, true);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    ret = create_security_policy(g_test_params.num_vcns, g_test_params.num_subnets,
                                 g_test_params.num_rules, IP_AF_IPV6, false);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    ret = create_security_policy(g_test_params.num_vcns, g_test_params.num_subnets,
                                 g_test_params.num_rules, IP_AF_IPV6, true);
    if (ret != SDK_RET_OK) {
        return ret;
    }
#endif
    // create vcns and subnets
    ret = create_vcns(g_test_params.num_vcns, &g_test_params.vcn_pfx,
                      g_test_params.num_subnets);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // create vnics
    ret = create_vnics(g_test_params.num_vcns, g_test_params.num_subnets,
                       g_test_params.num_vnics, g_test_params.vlan_start);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // create mappings
    ret = create_mappings(g_test_params.num_teps, g_test_params.num_vcns,
                          g_test_params.num_subnets, g_test_params.num_vnics,
                          g_test_params.num_ip_per_vnic, &g_test_params.tep_pfx,
                          &g_test_params.nat_pfx, &g_test_params.v6_nat_pfx,
                          g_test_params.num_remote_mappings);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // create V6 flows
    ret = create_flows(g_test_params.num_tcp, g_test_params.num_udp,
                       g_test_params.num_icmp, g_test_params.sport_base,
                       g_test_params.dport_base, true);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // create V4 flows
    ret = create_flows(g_test_params.num_tcp, g_test_params.num_udp,
                       g_test_params.num_icmp, g_test_params.sport_base,
                       g_test_params.dport_base, false);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    return ret;
}




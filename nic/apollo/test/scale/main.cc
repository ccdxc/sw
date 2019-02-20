//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the base class implementation of test classes
///
//----------------------------------------------------------------------------

#include <stdio.h>
#include <getopt.h>
#include <gtest/gtest.h>
#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/platform/capri/capri_p4.hpp"
#include "nic/sdk/model_sim/include/lib_model_client.h"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/include/api/oci_batch.hpp"
#include "nic/apollo/include/api/oci_switchport.hpp"
#include "nic/apollo/include/api/oci_tep.hpp"
#include "nic/apollo/include/api/oci_vcn.hpp"
#include "nic/apollo/include/api/oci_subnet.hpp"
#include "nic/apollo/include/api/oci_vnic.hpp"
#include "nic/apollo/include/api/oci_mapping.hpp"
#include "nic/apollo/include/api/oci_policy.hpp"
#include "nic/apollo/test/flow_test/flow_test.hpp"

using std::string;
namespace pt = boost::property_tree;

char *g_input_cfg_file = NULL;
char *g_cfg_file = NULL;
bool g_daemon_mode = false;
ip_prefix_t g_vcn_ippfx = {0};
oci_switchport_t g_swport = {0};
flow_test *g_flow_test_obj;

uint8_t g_snd_pkt1[] = {
    0x00, 0x00, 0xF1, 0xD0, 0xD1, 0xD0, 0x00, 0x00, 0x00, 0x40, 0x08,
    0x01, 0x81, 0x00, 0x00, 0x01, 0x08, 0x00, 0x45, 0x00, 0x00, 0x50,
    0x00, 0x01, 0x00, 0x00, 0x40, 0x11, 0xB6, 0x9A, 0x02, 0x00, 0x00,
    0x01, 0xC0, 0x00, 0x02, 0x01, 0x03, 0xE8, 0x27, 0x10, 0x00, 0x3C,
    0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64, 0x65,
    0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x7A, 0x78, 0x79
};

uint8_t g_rcv_pkt1[] = {
    0x00, 0x02, 0x0B, 0x0A, 0x0D, 0x0E, 0x00, 0x02, 0x01, 0x00, 0x00, 0x01,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x05, 0x8A, 0x9F,
    0x19, 0xEB, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x45, 0x00, 0x00, 0x50, 0x00, 0x01, 0x00, 0x00, 0x40, 0x11,
    0xB6, 0x9A, 0x02, 0x00, 0x00, 0x01, 0xC0, 0x00, 0x02, 0x01, 0x03, 0xE8,
    0x27, 0x10, 0x00, 0x3C, 0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72,
    0x73, 0x74, 0x75, 0x76, 0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x7A, 0x78, 0x79
};

uint8_t g_snd_pkt2[] = {
    0x00, 0x02, 0x01, 0x00, 0x00, 0x01, 0x00, 0x02, 0x0B, 0x0A, 0x0D, 0x0E,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x01, 0x92, 0xBA,
    0x19, 0xEB, 0x00, 0x60, 0x00, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0x00,
    0x11, 0x00, 0x45, 0x00, 0x00, 0x50, 0x00, 0x01, 0x00, 0x00, 0x40, 0x11,
    0x76, 0x9A, 0x02, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x01, 0x27, 0x10,
    0x03, 0xE8, 0x00, 0x3C, 0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72,
    0x73, 0x74, 0x75, 0x76, 0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x7A, 0x78, 0x79
};

uint8_t g_rcv_pkt2[] = {
    0x00, 0x00, 0x00, 0x10, 0x04, 0x01, 0x00, 0x00, 0x00, 0x10, 0x04,
    0x02, 0x81, 0x00, 0x00, 0x01, 0x08, 0x00, 0x45, 0x00, 0x00, 0x50,
    0x00, 0x01, 0x00, 0x00, 0x40, 0x11, 0x76, 0x9A, 0x02, 0x00, 0x00,
    0x02, 0x02, 0x00, 0x00, 0x01, 0x27, 0x10, 0x03, 0xE8, 0x00, 0x3C,
    0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64, 0x65,
    0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x7A, 0x78, 0x79
};

uint8_t g_snd_pkt3[] = {
    0x00, 0x00, 0xF1, 0xD0, 0xD1, 0xD0, 0x00, 0x00, 0x00, 0x40, 0x08,
    0x01, 0x81, 0x00, 0x00, 0x01, 0x08, 0x00, 0x45, 0x00, 0x00, 0x50,
    0x00, 0x01, 0x00, 0x00, 0x40, 0x11, 0x76, 0x7B, 0x02, 0x00, 0x00,
    0x01, 0x02, 0x00, 0x00, 0x21, 0x03, 0xE8, 0x27, 0x10, 0x00, 0x3C,
    0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64, 0x65,
    0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x7A, 0x78, 0x79
};

uint8_t g_rcv_pkt3[] = {
    0x00, 0x02, 0x0B, 0x0A, 0x0D, 0x0E, 0x00, 0x02, 0x01, 0x00, 0x00, 0x01,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x03, 0x1F, 0xEF,
    0x19, 0xEB, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x40,
    0x11, 0x00, 0x45, 0x00, 0x00, 0x50, 0x00, 0x01, 0x00, 0x00, 0x40, 0x11,
    0x76, 0x7B, 0x02, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x21, 0x03, 0xE8,
    0x27, 0x10, 0x00, 0x3C, 0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72,
    0x73, 0x74, 0x75, 0x76, 0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x7A, 0x78, 0x79
};

class scale_test : public oci_test_base {
protected:
    scale_test() {}
    virtual ~scale_test() {}
    /**< called immediately after the constructor before each test */
    virtual void SetUp() {}
    /**< called immediately after each test before the destructor */
    virtual void TearDown() {}
    /**< called at the beginning of all test cases in this class */
    static void SetUpTestCase() {
        /**< call base class function */
        oci_test_base::SetUpTestCase(g_cfg_file, false);
    }
};

//----------------------------------------------------------------------------
// create route tables
//------------------------------------------------------------------------------
static sdk_ret_t
create_route_tables (uint32_t num_teps, uint32_t num_vcns, uint32_t num_subnets,
                     uint32_t num_routes, ip_prefix_t *tep_pfx,
                     ip_prefix_t *route_pfx)
{
    uint32_t ntables = num_vcns * num_subnets;
    uint32_t tep_offset = 3;
    static uint32_t rtnum = 0;
    oci_route_table_t route_table;
    sdk_ret_t rv = SDK_RET_OK;

    route_table.af = IP_AF_IPV4;
    route_table.routes =
        (oci_route_t *)malloc((num_routes * sizeof(oci_route_t)));
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
            route_table.routes[j].nh_type = OCI_NH_TYPE_REMOTE_TEP;
            route_table.routes[j].vcn_id = OCI_VCN_ID_INVALID;
        }
        rv = oci_route_table_create(&route_table);
        if (rv != SDK_RET_OK) {
            return rv;
        }
    }
    return rv;
}

//----------------------------------------------------------------------------
// 1. create 1 primary + 32 secondary IP for each of 1K local vnics
// 2. create 1023 remote mappings per VCN
//------------------------------------------------------------------------------
static sdk_ret_t
create_mappings (uint32_t num_teps, uint32_t num_vcns, uint32_t num_subnets,
                 uint32_t num_vnics, uint32_t num_ip_per_vnic,
                 ip_prefix_t *teppfx, ip_prefix_t *natpfx,
                 uint32_t num_remote_mappings)
{
    sdk_ret_t rv;
    oci_mapping_t oci_mapping;
    uint16_t vnic_key = 1, ip_base, mac_offset = 1025;
    uint32_t ip_offset = 0, remote_slot = 1025, tep_offset = 3;

    // ensure a max. of 32 IPs per VNIC
    SDK_ASSERT(num_vcns * num_subnets * num_vnics * num_ip_per_vnic <=
               (32 * 1024));
    // create local vnic IP mappings first
    for (uint32_t i = 1; i <= num_vcns; i++) {
        for (uint32_t j = 1; j <= num_subnets; j++) {
            for (uint32_t k = 1; k <= num_vnics; k++) {
                for (uint32_t l = 1; l <= num_ip_per_vnic; l++) {
                    memset(&oci_mapping, 0, sizeof(oci_mapping));
                    oci_mapping.key.vcn.id = i;
                    oci_mapping.key.ip_addr.af = IP_AF_IPV4;
                    oci_mapping.key.ip_addr.addr.v4_addr =
                        (g_vcn_ippfx.addr.addr.v4_addr | ((j - 1) << 14)) |
                        (((k - 1) * num_ip_per_vnic) + l);
                    oci_mapping.subnet.id = (i - 1) * num_subnets + j;
                    oci_mapping.slot = vnic_key;
                    oci_mapping.tep.ip_addr = g_swport.switch_ip_addr;
                    MAC_UINT64_TO_ADDR(oci_mapping.overlay_mac,
                                       (((((uint64_t)i & 0x7FF) << 22) |
                                         ((j & 0x7FF) << 11) | (k & 0x7FF))));
                    oci_mapping.vnic.id = vnic_key;
                    if (natpfx) {
                        oci_mapping.public_ip_valid = true;
                        oci_mapping.public_ip.addr.v4_addr =
                            natpfx->addr.addr.v4_addr + ip_offset++;
                    }
                    rv = oci_mapping_create(&oci_mapping);
                    if (rv != SDK_RET_OK) {
                        return rv;
                    }

                    g_flow_test_obj->add_local_ep(oci_mapping.key.vcn.id,
                                         oci_mapping.key.ip_addr.addr.v4_addr);
                }
                vnic_key++;
            }
        }
    }

    // create remote mappings
    SDK_ASSERT(num_vcns * num_remote_mappings <= (1 << 20));
    for (uint32_t i = 1; i <= num_vcns; i++) {
        for (uint32_t j = 1; j <= num_subnets; j++) {
            ip_base = num_vnics * num_ip_per_vnic + 1;
            for (uint32_t k = 1; k <= num_remote_mappings; k++) {
                memset(&oci_mapping, 0, sizeof(oci_mapping));
                oci_mapping.key.vcn.id = i;
                oci_mapping.key.ip_addr.af = IP_AF_IPV4;
                oci_mapping.key.ip_addr.addr.v4_addr =
                    (g_vcn_ippfx.addr.addr.v4_addr | ((j - 1) << 14)) |
                    ip_base++;
                oci_mapping.subnet.id = (i - 1) * num_subnets + j;
                oci_mapping.slot = remote_slot++;
                oci_mapping.tep.ip_addr =
                    teppfx->addr.addr.v4_addr + tep_offset++;
                tep_offset %= num_teps;
                if (tep_offset == 0) {
                    // skip MyTEP and gateway IPs
                    tep_offset += 3;
                }
                MAC_UINT64_TO_ADDR(
                    oci_mapping.overlay_mac,
                    (((((uint64_t)i & 0x7FF) << 22) | ((j & 0x7FF) << 11) |
                      ((num_vnics + k) & 0x7FF))));
                rv = oci_mapping_create(&oci_mapping);
                if (rv != SDK_RET_OK) {
                    return rv;
                }

                g_flow_test_obj->add_remote_ep(oci_mapping.key.vcn.id,
                                     oci_mapping.key.ip_addr.addr.v4_addr);
            }
        }
    }
    return SDK_RET_OK;
}

// MAC address is encoded like below:
// bits 0-10 have vnic number in the subnet
// bits 11-21 have subnet number in the vcn
// bits 22-32 have vcn number
static sdk_ret_t
create_vnics (uint32_t num_vcns, uint32_t num_subnets,
              uint32_t num_vnics, uint16_t vlan_start)
{
    sdk_ret_t rv;
    oci_vnic_t oci_vnic;
    uint16_t vnic_key = 1;

    SDK_ASSERT(num_vcns * num_subnets * num_vnics <= 1024);
    for (uint32_t i = 1; i <= (uint64_t)num_vcns; i++) {
        for (uint32_t j = 1; j <= num_subnets; j++) {
            for (uint32_t k = 1; k <= num_vnics; k++) {
                memset(&oci_vnic, 0, sizeof(oci_vnic));
                oci_vnic.vcn.id = i;
                oci_vnic.subnet.id = (i - 1) * num_subnets + j;
                oci_vnic.key.id = vnic_key;
                oci_vnic.wire_vlan = vlan_start + vnic_key - 1;
                oci_vnic.slot = vnic_key;
                MAC_UINT64_TO_ADDR(oci_vnic.mac_addr,
                                   (((((uint64_t)i & 0x7FF) << 22) |
                                     ((j & 0x7FF) << 11) | (k & 0x7FF))));
                oci_vnic.rsc_pool_id = 1;
                oci_vnic.src_dst_check = false; //(k & 0x1);
                rv = oci_vnic_create(&oci_vnic);
                if (rv != SDK_RET_OK) {
                    return rv;
                }
                vnic_key++;
            }
        }
    }

    return rv;
}

// VCN prefix is /8, subnet id is encoded in next 10 bits (making it /18 prefix)
// leaving LSB 14 bits for VNIC IPs
static sdk_ret_t
create_subnets (uint32_t vcn_id, uint32_t num_subnets, ip_prefix_t *vcn_pfx)
{
    sdk_ret_t rv;
    oci_subnet_t oci_subnet;
    static uint32_t route_table_id = 1;

    for (uint32_t i = 1; i <= num_subnets; i++) {
        memset(&oci_subnet, 0, sizeof(oci_subnet));
        oci_subnet.key.id = (vcn_id - 1) * num_subnets + i;
        oci_subnet.vcn.id = vcn_id;
        oci_subnet.pfx = *vcn_pfx;
        oci_subnet.pfx.addr.addr.v4_addr =
            (oci_subnet.pfx.addr.addr.v4_addr) | ((i - 1) << 14);
        oci_subnet.pfx.len = 18;
        oci_subnet.vr_ip.af = IP_AF_IPV4;
        // oci_subnet.vr_ip.addr.v4_addr = oci_subnet.pfx.addr.addr.v4_addr |
        // 0x1;
        oci_subnet.vr_ip.addr.v4_addr = oci_subnet.pfx.addr.addr.v4_addr;
        MAC_UINT64_TO_ADDR(oci_subnet.vr_mac,
                           (uint64_t)oci_subnet.vr_ip.addr.v4_addr);
        oci_subnet.v4_route_table.id = route_table_id++;
        rv = oci_subnet_create(&oci_subnet);
        if (rv != SDK_RET_OK) {
            return rv;
        }
    }
    return SDK_RET_OK;
}

static sdk_ret_t
create_vcns (uint32_t num_vcns, ip_prefix_t *ip_pfx, uint32_t num_subnets)
{
    sdk_ret_t rv;
    oci_vcn_t oci_vcn;

    SDK_ASSERT(num_vcns <= 1024);
    for (uint32_t i = 1; i <= num_vcns; i++) {
        memset(&oci_vcn, 0, sizeof(oci_vcn));
        oci_vcn.type = OCI_VCN_TYPE_TENANT;
        oci_vcn.key.id = i;
        oci_vcn.pfx = *ip_pfx;
        oci_vcn.pfx.len = 8; // fix this to /8
        oci_vcn.pfx.addr.addr.v4_addr &= 0xFF000000;
        rv = oci_vcn_create(&oci_vcn);
        if (rv != SDK_RET_OK) {
            return rv;
        }
        for (uint32_t j = 1; j <= num_subnets; j++) {
            rv = create_subnets(i, j, &oci_vcn.pfx);
            if (rv != SDK_RET_OK) {
                return rv;
            }
        }
    }
    return SDK_RET_OK;
}

static sdk_ret_t
create_teps (uint32_t num_teps, ip_prefix_t *ip_pfx)
{
    sdk_ret_t rv;
    oci_tep_t oci_tep;

    // leave the 1st IP in this prefix for MyTEP
    for (uint32_t i = 1; i <= num_teps; i++) {
        memset(&oci_tep, 0, sizeof(oci_tep));
        // 1st IP in the TEP prefix is local TEP, 2nd is gateway IP,
        // so skip them
        oci_tep.key.ip_addr = ip_pfx->addr.addr.v4_addr + 2 + i;
        oci_tep.type = OCI_ENCAP_TYPE_VNIC;
        rv = oci_tep_create(&oci_tep);
        if (rv != SDK_RET_OK) {
            return rv;
        }
    }
    return SDK_RET_OK;
}

static sdk_ret_t
create_switchport_cfg (ipv4_addr_t ipaddr, uint64_t macaddr, ipv4_addr_t gwip)
{
    sdk_ret_t rv;

    memset(&g_swport, 0, sizeof(g_swport));
    g_swport.switch_ip_addr = ipaddr;
    MAC_UINT64_TO_ADDR(g_swport.switch_mac_addr, macaddr);
    g_swport.gateway_ip_addr = gwip;
    return oci_switchport_create(&g_swport);
}

static sdk_ret_t
create_security_policy (uint32_t num_vcns, uint32_t num_subnets,
                        uint32_t num_rules, uint32_t ip_af, bool ingress)
{
    sdk_ret_t       rv;
    oci_policy_t    policy;
    uint32_t        id = 1;
    rule_t          *rule;

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
            policy.key.id = id++;
            for (uint32_t k = 0; k < num_rules; k++) {
                rule = &policy.rules[k];
                rule->stateful = false;
                rule->match.l3_match.ip_proto = 17;    // UDP
                rule->match.l3_match.ip_pfx = g_vcn_ippfx;
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
            rv = oci_policy_create(&policy);
            if (rv != SDK_RET_OK) {
                printf("Failed to create security policy, vcn %u, subnet %u, "
                       "err %u\n", i, j, rv);
                return rv;
            }
        }
    }
    return SDK_RET_OK;
}

static sdk_ret_t
create_flows (uint32_t num_tcp, uint32_t num_udp, uint32_t num_icmp)
{
    sdk_ret_t ret = SDK_RET_OK;

    ret = g_flow_test_obj->create_flows(num_tcp, 6);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = g_flow_test_obj->create_flows(num_udp, 17);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = g_flow_test_obj->create_flows(num_icmp, 1);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    return SDK_RET_OK;
}

static sdk_ret_t
create_objects (void)
{
    uint32_t num_vcns = 0, num_subnets = 0, num_vnics = 0, num_teps = 0;
    uint32_t num_rules = 0;
    uint32_t num_remote_mappings = 0, num_routes = 0, num_ip_per_vnic = 1;
    uint16_t vlan_start = 1;
    pt::ptree json_pt;
    ip_prefix_t teppfx, natpfx, routepfx;
    string pfxstr;
    sdk_ret_t ret = SDK_RET_OK;
    uint32_t num_tcp, num_udp, num_icmp;
    
    g_flow_test_obj = new flow_test();

    // parse the config and create objects
    std::ifstream json_cfg(g_input_cfg_file);
    read_json(json_cfg, json_pt);
    try {
        BOOST_FOREACH (pt::ptree::value_type &obj,
                       json_pt.get_child("objects")) {
            std::string kind = obj.second.get<std::string>("kind");
            if (kind == "switchport") {
                struct in_addr ipaddr, gwip;
                uint64_t macaddr;

                macaddr =
                    std::stoull(obj.second.get<std::string>("mac-addr"), 0, 0);
                inet_aton(obj.second.get<std::string>("ip-addr").c_str(),
                          &ipaddr);
                inet_aton(obj.second.get<std::string>("gw-ip-addr").c_str(),
                          &gwip);
                ret = create_switchport_cfg(ntohl(ipaddr.s_addr), macaddr,
                                            ntohl(gwip.s_addr));
            } else if (kind == "tep") {
                num_teps = std::stol(obj.second.get<std::string>("count"));
                if (num_teps <= 2) {
                    printf("No. of TEPs must be greater than 2\n");
                    exit(1);
                }
                // reduce num_teps by 2, (MyTEP and GW-TEP)
                num_teps -= 2;
                pfxstr = obj.second.get<std::string>("prefix");
                assert(str2ipv4pfx((char *)pfxstr.c_str(), &teppfx) == 0);
                ret = create_teps(num_teps, &teppfx);
            } else if (kind == "route-table") {
                num_routes = std::stol(obj.second.get<std::string>("count"));
                pfxstr = obj.second.get<std::string>("prefix-start");
                assert(str2ipv4pfx((char *)pfxstr.c_str(), &routepfx) == 0);
                ret = create_route_tables(num_teps, 1024, 1, num_routes, &teppfx,
                                    &routepfx);
            } else if (kind == "vcn") {
                num_vcns = std::stol(obj.second.get<std::string>("count"));
                pfxstr = obj.second.get<std::string>("prefix");
                assert(str2ipv4pfx((char *)pfxstr.c_str(), &g_vcn_ippfx) == 0);
                num_subnets = std::stol(obj.second.get<std::string>("subnets"));
                ret = create_vcns(num_vcns, &g_vcn_ippfx, num_subnets);
            } else if (kind == "vnic") {
                num_vnics = std::stol(obj.second.get<std::string>("count"));
                vlan_start =
                    std::stol(obj.second.get<std::string>("vlan-start"));
                ret = create_vnics(num_vcns, num_subnets, num_vnics, vlan_start);
            } else if (kind == "mappings") {
                pfxstr = obj.second.get<std::string>("nat-prefix");
                assert(str2ipv4pfx((char *)pfxstr.c_str(), &natpfx) == 0);
                num_remote_mappings =
                    std::stol(obj.second.get<std::string>("remotes"));
                num_ip_per_vnic =
                    std::stol(obj.second.get<std::string>("locals"));
                ret = create_mappings(num_teps, num_vcns, num_subnets, num_vnics,
                                      num_ip_per_vnic, &teppfx, &natpfx,
                                      num_remote_mappings);
            } else if (kind == "security-policy") {
                num_rules = std::stol(obj.second.get<std::string>("count"));
                ret = create_security_policy(num_vcns, num_subnets, num_rules,
                                             IP_AF_IPV4, false);
                //ret = create_security_policy(num_vcns, num_subnets, num_rules,
                                             //IP_AF_IPV6, true);
            } else if (kind == "flows") {
                num_tcp = std::stol(obj.second.get<std::string>("num_tcp"));
                num_udp = std::stol(obj.second.get<std::string>("num_udp"));
                num_icmp = std::stol(obj.second.get<std::string>("num_icmp"));
                ret = create_flows(num_tcp, num_udp, num_icmp);
            }

            if (ret != SDK_RET_OK) {
                return ret;
            }
        }
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }

    return ret;
}

#ifdef SIM
static void
send_packet (void)
{
    uint32_t port = 0;
    uint32_t cos = 0;
    std::vector<uint8_t> ipkt;
    std::vector<uint8_t> opkt;
    std::vector<uint8_t> epkt;
    uint32_t i = 0;
    uint32_t tcscale = 1;
    int tcid = 0;
    int tcid_filter = 0;

    if (getenv("TCSCALE")) {
        tcscale = atoi(getenv("TCSCALE"));
    }

    if (getenv("TCID")) {
        tcid_filter = atoi(getenv("TCID"));
    }

#if 0
    tcid++;
    if (tcid_filter == 0 || tcid == tcid_filter) {
        ipkt.resize(sizeof(g_snd_pkt1));
        memcpy(ipkt.data(), g_snd_pkt1, sizeof(g_snd_pkt1));
        epkt.resize(sizeof(g_rcv_pkt1));
        memcpy(epkt.data(), g_rcv_pkt1, sizeof(g_rcv_pkt1));
        std::cout << "Testing Host to Switch" << std::endl;
        for (i = 0; i < tcscale; i++) {
            testcase_begin(tcid, i + 1);
            step_network_pkt(ipkt, TM_PORT_UPLINK_0);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(opkt == epkt);
                EXPECT_TRUE(port == TM_PORT_UPLINK_1);
            }
            testcase_end(tcid, i + 1);
        }
    }

    tcid++;
    if (tcid_filter == 0 || tcid == tcid_filter) {
        ipkt.resize(sizeof(g_snd_pkt2));
        memcpy(ipkt.data(), g_snd_pkt2, sizeof(g_snd_pkt2));
        epkt.resize(sizeof(g_rcv_pkt2));
        memcpy(epkt.data(), g_rcv_pkt2, sizeof(g_rcv_pkt2));
        std::cout << "Testing Switch to Host" << std::endl;
        for (i = 0; i < tcscale; i++) {
            testcase_begin(tcid, i+1);
            step_network_pkt(ipkt, TM_PORT_UPLINK_1);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(opkt == epkt);
                EXPECT_TRUE(port == TM_PORT_UPLINK_0);
            }
            testcase_end(tcid, i+1);
        }
    }
#endif

    tcid++;
    if (tcid_filter == 0 || tcid == tcid_filter) {
        ipkt.resize(sizeof(g_snd_pkt3));
        memcpy(ipkt.data(), g_snd_pkt3, sizeof(g_snd_pkt3));
        epkt.resize(sizeof(g_rcv_pkt3));
        memcpy(epkt.data(), g_rcv_pkt3, sizeof(g_rcv_pkt3));
        std::cout << "Testing Host to Switch" << std::endl;
        for (i = 0; i < tcscale; i++) {
            testcase_begin(tcid, i + 1);
            step_network_pkt(ipkt, TM_PORT_UPLINK_0);
            if (!getenv("SKIP_VERIFY")) {
                get_next_pkt(opkt, port, cos);
                EXPECT_TRUE(opkt == epkt);
                EXPECT_TRUE(port == TM_PORT_UPLINK_1);
            }
            testcase_end(tcid, i + 1);
        }
    }

    exit_simulation();
}
#endif

/// \defgroup Scale
/// @{

/// Scale test
TEST_F(scale_test, scale_test_create)
{
    sdk_ret_t rv;
    oci_batch_params_t batch_params = {0};

    batch_params.epoch = 1;
    rv = oci_batch_start(&batch_params);
    ASSERT_TRUE(rv == SDK_RET_OK);
    rv = create_objects();
    ASSERT_TRUE(rv == SDK_RET_OK);
    rv = oci_batch_commit();
    ASSERT_TRUE(rv == SDK_RET_OK);

#ifdef SIM
    send_packet();
#endif

    if (g_daemon_mode) {
        printf("Entering forever loop ...\n");
        fflush(stdout);
        while (1)
            ;
    }
}

/// @}

// print help message showing usage of this gtest
static void inline print_usage(char **argv)
{
    fprintf(stdout, "Usage : %s -c <hal.json> -i <test-config.json>\n",
            argv[0]);
}

int
main (int argc, char **argv)
{
    int oc;
    struct option longopts[] = {
        {"config", required_argument, NULL, 'c'},
        {"daemon", required_argument, NULL, 'd'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}
    };

    // parse CLI options
    while ((oc = getopt_long(argc, argv, ":hdc:i:W;", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            g_cfg_file = optarg;
            if (!g_cfg_file) {
                fprintf(stderr, "HAL config file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'd':
            g_daemon_mode = true;
            break;

        case 'i':
            g_input_cfg_file = optarg;
            if (!g_input_cfg_file) {
                fprintf(stderr, "test config file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        default:
            // ignore all other options
            break;
        }
    }

    // make sure cfg files exist
    if (access(g_input_cfg_file, R_OK) < 0) {
        fprintf(stderr, "Config file %s doesn't exist or not accessible\n",
                g_input_cfg_file);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

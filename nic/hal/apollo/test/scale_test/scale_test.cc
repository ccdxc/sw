// Copyright (c) 2018 Pensando Systems, Inc.

#include <stdio.h>
#include <getopt.h>
#include <gtest/gtest.h>
#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/hal/apollo/test/oci_test_base.hpp"
#include "nic/hal/apollo/include/api/oci_batch.hpp"
#include "nic/hal/apollo/include/api/oci_switchport.hpp"
#include "nic/hal/apollo/include/api/oci_tep.hpp"
#include "nic/hal/apollo/include/api/oci_vcn.hpp"
#include "nic/hal/apollo/include/api/oci_subnet.hpp"
#include "nic/hal/apollo/include/api/oci_vnic.hpp"
#include "nic/hal/apollo/include/api/oci_mapping.hpp"

using std::string;
namespace pt = boost::property_tree;

char                *g_input_cfg_file = NULL;
char                *g_cfg_file = NULL;
ip_prefix_t         g_vcn_ippfx = { 0 };
oci_switchport_t    g_sw_port = { 0 };

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

//------------------------------------------------------------------------------
// create route tables
//------------------------------------------------------------------------------
static void
create_route_tables (uint32_t num_teps, uint32_t num_vcns, uint32_t num_subnets,
                     uint32_t num_routes, ip_prefix_t *tep_pfx,
                     ip_prefix_t *route_pfx)
{
    uint32_t             ntables = num_vcns * num_subnets;
    uint32_t             tep_offset = 3;
    oci_route_table_t    route_table;

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
                (route_pfx->addr.addr.v4_addr & 0xFF000000) | (j << 16);
            route_table.routes[j].nh_ip.af = IP_AF_IPV4;
            route_table.routes[j].nh_ip.addr.v4_addr =
                tep_pfx->addr.addr.v4_addr + tep_offset++;
            tep_offset %= num_teps;
            if (tep_offset == 0) {
                // skip MyTEP and gateway IPs
                tep_offset += 3;
            }
            route_table.routes[j].nh_type = OCI_NH_TYPE_REMOTE_TEP;
            route_table.routes[j].vcn_id = OCI_VCN_ID_INVALID;
        }
        SDK_ASSERT(oci_route_table_create(&route_table) == SDK_RET_OK);
    }
    return;
}

//------------------------------------------------------------------------------
// 1. create 1 primary + 32 secondary IP for each of 1K local vnics
// 2. create 1023 remote mappings per VCN
//------------------------------------------------------------------------------
static void
create_mappings (uint32_t num_teps, uint32_t num_vcns, uint32_t num_subnets,
                 uint32_t num_vnics, uint32_t num_ip_per_vnic,
                 ip_prefix_t *teppfx, ip_prefix_t *natpfx,
                 uint32_t num_remote_mappings)
{
    sdk_ret_t        rv;
    oci_mapping_t    oci_mapping;
    uint16_t         vnic_key = 1, ip_base, mac_offset = 1025;
    uint32_t         ip_offset = 0, remote_slot = 1025, tep_offset = 3;

    // ensure a max. of 32 IPs per VNIC
    SDK_ASSERT(num_vcns * num_subnets * num_vnics * num_ip_per_vnic <= (32 * 1024));
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
                    oci_mapping.subnet.vcn_id = i;
                    oci_mapping.subnet.id = j;
                    oci_mapping.slot = vnic_key;
                    oci_mapping.tep.ip_addr = g_sw_port.switch_ip_addr;
                    MAC_UINT64_TO_ADDR(oci_mapping.overlay_mac,
                                       (((((uint64_t)i & 0x3FF) << 20) |
                                         ((j & 0x3FF) << 10) | (k & 0x3FF))));
                    oci_mapping.vnic.id = vnic_key;
                    if (natpfx) {
                        oci_mapping.public_ip_valid = true;
                        oci_mapping.public_ip.addr.v4_addr =
                            natpfx->addr.addr.v4_addr + ip_offset++;
                    }
                    rv = oci_mapping_create(&oci_mapping);
                    ASSERT_TRUE(rv == SDK_RET_OK);
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
                oci_mapping.subnet.vcn_id = i;
                oci_mapping.subnet.id = j;
                oci_mapping.slot = remote_slot++;
                oci_mapping.tep.ip_addr =
                    teppfx->addr.addr.v4_addr + tep_offset++;
                tep_offset %= num_teps;
                if (tep_offset == 0) {
                    // skip MyTEP and gateway IPs
                    tep_offset += 3;
                }
                MAC_UINT64_TO_ADDR(oci_mapping.overlay_mac,
                                   (((((uint64_t)i & 0x3FF) << 20) |
                                     ((j & 0x3FF) << 10) |
                                     ((num_vnics + k) & 0x3FF))));
                rv = oci_mapping_create(&oci_mapping);
                ASSERT_TRUE(rv == SDK_RET_OK);
            }
        }
    }
}

static void
create_vnics (uint32_t num_vcns, uint32_t num_subnets,
              uint32_t num_vnics, uint16_t vlan_start)
{
    sdk_ret_t     rv;
    oci_vnic_t    oci_vnic;
    uint16_t      vnic_key = 1;

    SDK_ASSERT(num_vcns * num_subnets * num_vnics <= 1024);
    for (uint32_t i = 1; i <= (uint64_t)num_vcns; i++) {
        for (uint32_t j = 1; j <= num_subnets; j++) {
            for (uint32_t k = 1; k <= num_vnics; k++) {
                memset(&oci_vnic, 0, sizeof(oci_vnic));
                oci_vnic.vcn.id = i;
                oci_vnic.subnet.vcn_id = i;
                oci_vnic.subnet.id = j;
                oci_vnic.key.id = vnic_key;
                oci_vnic.wire_vlan = vlan_start + vnic_key - 1;
                oci_vnic.slot = vnic_key;
                MAC_UINT64_TO_ADDR(oci_vnic.mac_addr,
                                   (((((uint64_t)i & 0x3FF) << 20) |
                                     ((j & 0x3FF) << 10) | (k & 0x3FF))));
                oci_vnic.rsc_pool_id = 1;
                oci_vnic.src_dst_check = (k & 0x1);
                rv = oci_vnic_create(&oci_vnic);
                ASSERT_TRUE(rv == SDK_RET_OK);
                vnic_key++;
            }
        }
    }
}

// VCN prefix is /8, subnet id is encoded in next 10 bits (making it /18 prefix)
// leaving LSB 14 bits for VNIC IPs
static void
create_subnets (uint32_t vcn_id, uint32_t num_subnets, ip_prefix_t *vcn_pfx)
{
    sdk_ret_t       rv;
    oci_subnet_t    oci_subnet;
    uint32_t        route_table_id = 1;

    for (uint32_t i = 1; i <= num_subnets; i++) {
        memset(&oci_subnet, 0, sizeof(oci_subnet));
        oci_subnet.key.vcn_id = vcn_id;
        oci_subnet.key.id = i;
        oci_subnet.pfx = *vcn_pfx;
        oci_subnet.pfx.addr.addr.v4_addr =
            (oci_subnet.pfx.addr.addr.v4_addr) | ((i - 1) << 14);
        oci_subnet.pfx.len = 18;
        oci_subnet.vr_ip.af = IP_AF_IPV4;
        //oci_subnet.vr_ip.addr.v4_addr = oci_subnet.pfx.addr.addr.v4_addr | 0x1;
        oci_subnet.vr_ip.addr.v4_addr = oci_subnet.pfx.addr.addr.v4_addr;
        MAC_UINT64_TO_ADDR(oci_subnet.vr_mac,
                           (uint64_t)oci_subnet.vr_ip.addr.v4_addr);
        oci_subnet.route_table.id = route_table_id++;
        rv = oci_subnet_create(&oci_subnet);
        ASSERT_TRUE(rv == SDK_RET_OK);
    }
}

static void
create_vcns (uint32_t num_vcns, ip_prefix_t *ip_pfx, uint32_t num_subnets)
{
    sdk_ret_t    rv;
    oci_vcn_t    oci_vcn;

    SDK_ASSERT(num_vcns <= 1024);
    for (uint32_t i = 1; i <= num_vcns; i++) {
        memset(&oci_vcn, 0, sizeof(oci_vcn));
        oci_vcn.type = OCI_VCN_TYPE_TENANT;
        oci_vcn.key.id = i;
        oci_vcn.pfx = *ip_pfx;
        oci_vcn.pfx.len = 8;    // fix this to /8
        oci_vcn.pfx.addr.addr.v4_addr &= 0xFF000000;
        rv = oci_vcn_create(&oci_vcn);
        ASSERT_TRUE(rv == SDK_RET_OK);
        for (uint32_t j = 1; j <= num_subnets; j++) {
            create_subnets(i, j, &oci_vcn.pfx);
        }
    }
}

static void
create_teps (uint32_t num_teps, ip_prefix_t *ip_pfx)
{
    sdk_ret_t    rv;
    oci_tep_t    oci_tep;

    // leave the 1st IP in this prefix for MyTEP
    for (uint32_t i = 1; i <= num_teps; i++) {
        memset(&oci_tep, 0, sizeof(oci_tep));
        // 1st IP in the TEP prefix is local TEP, 2nd is gateway IP,
        // so skip them
        oci_tep.key.ip_addr = ip_pfx->addr.addr.v4_addr + 2 + i;
        oci_tep.type = OCI_ENCAP_TYPE_IPINIP_GRE;
        rv = oci_tep_create(&oci_tep);
        ASSERT_TRUE(rv == SDK_RET_OK);
    }
}

static void
create_switchport_cfg (ipv4_addr_t ipaddr, uint64_t macaddr, ipv4_addr_t gwip)
{
    sdk_ret_t           rv;

    memset(&g_sw_port, 0, sizeof(g_sw_port));
    g_sw_port.switch_ip_addr = ipaddr;
    MAC_UINT64_TO_ADDR(g_sw_port.switch_mac_addr, macaddr);
    g_sw_port.gateway_ip_addr = gwip;
    rv = oci_switchport_create(&g_sw_port);
    ASSERT_TRUE(rv == SDK_RET_OK);
}

static void
create_objects (void)
{
    uint32_t       num_vcns = 0, num_subnets = 0, num_vnics = 0, num_teps = 0;
    uint32_t       num_remote_mappings = 0, num_routes = 0;
    uint16_t       vlan_start = 1;
    pt::ptree      json_pt;
    ip_prefix_t    teppfx, natpfx, routepfx;
    string         pfxstr;

    // parse the config and create objects
    std::ifstream json_cfg(g_input_cfg_file);
    read_json(json_cfg, json_pt);
    try {
        BOOST_FOREACH(pt::ptree::value_type& obj, json_pt.get_child("objects")) {
            std::string kind = obj.second.get<std::string>("kind");
            if (kind == "switchport") {
                struct in_addr    ipaddr, gwip;
                uint64_t          macaddr;

                macaddr = std::stol(obj.second.get<std::string>("mac-addr"));
                inet_aton(obj.second.get<std::string>("ip-addr").c_str(),
                          &ipaddr);
                inet_aton(obj.second.get<std::string>("gw-ip-addr").c_str(),
                          &gwip);
                create_switchport_cfg(ntohl(ipaddr.s_addr), macaddr,
                                      ntohl(gwip.s_addr));
            } else if (kind == "tep") {
                num_teps = std::stol(obj.second.get<std::string>("count"));
                pfxstr = obj.second.get<std::string>("prefix");
                ASSERT_TRUE(str2ipv4pfx((char *)pfxstr.c_str(), &teppfx) == 0);
                create_teps(num_teps, &teppfx);
            } else if (kind == "route-table") {
                num_routes = std::stol(obj.second.get<std::string>("count"));
                pfxstr = obj.second.get<std::string>("prefix-start");
                ASSERT_TRUE(str2ipv4pfx((char *)pfxstr.c_str(), &routepfx) == 0);
                create_route_tables(num_teps, 1024, 1,
                                    num_routes, &teppfx, &routepfx);
            } else if (kind == "vcn") {
                num_vcns = std::stol(obj.second.get<std::string>("count"));
                pfxstr = obj.second.get<std::string>("prefix");
                ASSERT_TRUE(str2ipv4pfx((char *)pfxstr.c_str(), &g_vcn_ippfx) == 0);
                num_subnets = std::stol(obj.second.get<std::string>("subnets"));
                create_vcns(num_vcns, &g_vcn_ippfx, num_subnets);
            } else if (kind == "vnic") {
                num_vnics = std::stol(obj.second.get<std::string>("count"));
                vlan_start = std::stol(obj.second.get<std::string>("vlan-start"));
                create_vnics(num_vcns, num_subnets, num_vnics, vlan_start);
            } else if (kind == "remote-mapping") {
                pfxstr = obj.second.get<std::string>("nat-prefix");
                ASSERT_TRUE(str2ipv4pfx((char *)pfxstr.c_str(), &natpfx) == 0);
                num_remote_mappings = std::stol(obj.second.get<std::string>("count"));
                create_mappings(num_teps, num_vcns, num_subnets, num_vnics, 32,
                                &teppfx, &natpfx, num_remote_mappings);
            }
        }
    } catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
}

TEST_F(scale_test, scale_test_create) {
    sdk_ret_t             rv;
    oci_batch_params_t    batch_params = { 0 };

    batch_params.epoch = 1;
    rv = oci_batch_start(&batch_params);
    ASSERT_TRUE(rv == SDK_RET_OK);
    create_objects();
    rv = oci_batch_commit();
    ASSERT_TRUE(rv == SDK_RET_OK);

    //printf("Entering forever loop ...\n");
    //fflush(stdout);
    //while (1);
}

// print help message showing usage of HAL
static void inline
print_usage (char **argv)
{
    fprintf(stdout,
            "Usage : %s -c <hal.json> -i <object-config.json>\n", argv[0]);
}

int
main (int argc, char **argv)
{
    int               oc;
    struct option longopts[] = {
       { "config",    required_argument, NULL, 'c' },
       { "help",      no_argument,       NULL, 'h' },
       { 0,           0,                 0,     0 }
    };

    // parse CLI options
    while ((oc = getopt_long(argc, argv, ":hc:i:W;", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            g_cfg_file = optarg;
            if (!g_cfg_file) {
                fprintf(stderr, "HAL config file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'i':
            g_input_cfg_file = optarg;
            if (!g_input_cfg_file) {
                fprintf(stderr, "object config file is not specified\n");
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

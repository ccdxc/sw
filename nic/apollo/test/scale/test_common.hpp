//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __APOLLO_SCALE_TEST_COMMON_HPP__
#define __APOLLO_SCALE_TEST_COMMON_HPP__

#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/include/pds_meter.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/test/utils/base.hpp"

#define TESTAPP_METER_NUM_PREFIXES                         16
#define TESTAPP_SWITCH_VNIC_VLAN                           128
#define TESTAPP_SUBSTRATE_VNID                             0x6666
#define TESTAPP_MAX_SERVICE_TEP                            64
#define TESTAPP_MAX_REMOTE_SERVICE_TEP                     64
#define TEST_APP_VIP_PORT                                  32
#define TEST_APP_DIP_PORT                                  64

// reserved VPC ids
#define TEST_APP_S1_REMOTE_SVC_TUNNEL_IN_OUT               59
#define TEST_APP_S1_SVC_TUNNEL_IN_OUT                      60
#define TEST_APP_S1_SLB_IN_OUT                             61
#define TEST_APP_S3_VNET_IN_OUT_V6_OUTER                   62
#define TEST_APP_S2_INTERNET_IN_OUT_VPC_VIP_VPC            63
#define TEST_APP_S2_INTERNET_IN_OUT_FLOATING_IP_VPC        64

#define TESTAPP_V4ROUTE_PREFIX_LEN                         28
#define TESTAPP_V4ROUTE_PREFIX_VAL(rtnum) { \
    ((0xC << TESTAPP_V4ROUTE_PREFIX_LEN) | ((rtnum) << (32 - TESTAPP_V4ROUTE_PREFIX_LEN))) \
}
#define TESTAPP_V4ROUTE_VAL(rtnum) { \
    ((0xC << TESTAPP_V4ROUTE_PREFIX_LEN) | (uint32_t)(rtnum)) \
}

namespace pt = boost::property_tree;

typedef struct test_params_s {
    std::string pipeline;
    // device config
    struct {
        ip_addr_t device_ip;
        uint64_t device_mac;
        ip_addr_t device_gw_ip;
        pds_encap_t fabric_encap;
        bool v4_outer;
        bool dual_stack;
    };
    // TEP config
    struct {
        uint32_t num_teps;
        ip_prefix_t tep_pfx;
        ip_prefix_t svc_tep_pfx;
        ip_prefix_t remote_svc_public_ip_pfx;
        uint32_t svc_tep_vnid_base;
        uint32_t remote_svc_tep_vnid_base;
    };
    // route config
    struct {
        uint32_t num_routes;
        ip_prefix_t route_pfx;
        ip_prefix_t v6_route_pfx;
    };
    // policy config
    struct {
        uint32_t num_ipv4_rules;
        uint32_t num_ipv6_rules;
        bool stateful;
    };
    // vpc config
    struct {
        uint32_t num_vpcs;
        ip_prefix_t vpc_pfx;
        ip_prefix_t v6_vpc_pfx;
        ip_prefix_t nat46_vpc_pfx;
        uint32_t num_subnets;
    };
    // vnic config
    struct {
        uint32_t num_vnics;
        uint32_t vlan_start;
        bool tag_vnics;
    };
    // mapping config
    struct {
        ip_prefix_t nat_pfx;
        ip_prefix_t v6_nat_pfx;
        ip_prefix_t provider_pfx;
        ip_prefix_t v6_provider_pfx;
        uint32_t num_ip_per_vnic;
        uint32_t num_remote_mappings;
    };
    // flow config
    struct {
        uint32_t num_tcp;
        uint32_t num_udp;
        uint32_t num_icmp;
        uint16_t sport_lo;
        uint16_t sport_hi;
        uint16_t dport_lo;
        uint16_t dport_hi;
    };
    // mirror config
    struct {
        bool mirror_en;
        uint32_t num_rspan;
        uint32_t num_erspan;
        uint8_t rspan_bmap;
        uint8_t erspan_bmap;
    };
    // metering config
    struct {
        uint32_t num_meter;
        uint32_t meter_scale;
        pds_meter_type_t meter_type;
        uint64_t pps_bps;
        uint64_t burst;
    };
    // tags config
    struct {
        uint32_t num_tag_trees;
        uint32_t tags_v4_scale;
        uint32_t tags_v6_scale;
    };
    // nexthop config
    struct {
        uint32_t num_nh;
    };
    // service mapping
    struct {
        ip_prefix_t v4_vip_pfx;
        ip_prefix_t v6_vip_pfx;
    };
} test_params_t;

#define CONVERT_TO_V4_MAPPED_V6_ADDRESS(_v6pfx, _v4addr) {         \
    _v6pfx.addr8[12] = (_v4addr >> 24) & 0xFF;                     \
    _v6pfx.addr8[13] = (_v4addr >> 16) & 0xFF;                     \
    _v6pfx.addr8[14] = (_v4addr >> 8) & 0xFF;                      \
    _v6pfx.addr8[15] = (_v4addr) & 0xFF;                           \
}

static inline void
compute_remote46_addr (ip_addr_t *addr, ip_prefix_t *initial_pfx,
                       uint32_t shift_val)
{
    *addr = initial_pfx->addr;
    addr->addr.v6_addr.addr32[IP6_ADDR32_LEN-2] = htonl(0xF1D0D1D0);
    addr->addr.v6_addr.addr32[IP6_ADDR32_LEN-1] =
        htonl((0xC << 28) | (shift_val));
}

static inline void
compute_ipv6_prefix (ip_prefix_t *pfx, ip_prefix_t *initial_pfx,
                     uint32_t shift_val, uint32_t len)
{
    *pfx = *initial_pfx;
    pfx->addr.addr.v6_addr.addr32[IP6_ADDR32_LEN-2] = htonl(0xF1D0D1D0);
    pfx->addr.addr.v6_addr.addr32[IP6_ADDR32_LEN-1] =
        htonl((0xC << 28) | (shift_val << (128 - len)));
    pfx->len = len;
}

static void
meter_str_to_type (std::string meter_type_str,
                   pds_meter_type_t *meter_type)
{
    if (meter_type_str == "pps") {
        *meter_type = PDS_METER_TYPE_PPS_POLICER;
    } else if (meter_type_str == "bps") {
        *meter_type = PDS_METER_TYPE_BPS_POLICER;
    } else if (meter_type_str == "account") {
        *meter_type = PDS_METER_TYPE_ACCOUNTING;
    } else {
        *meter_type = PDS_METER_TYPE_NONE;
    }
}

inline sdk_ret_t
parse_test_cfg (const char *cfg_file, test_params_t *test_params)
{
    pt::ptree json_pt;
    string pfxstr, str;
    uint32_t i;
    char *tep_encap_env;

    // parse the config and create objects
    std::ifstream json_cfg(cfg_file);
    read_json(json_cfg, json_pt);
    try {
        BOOST_FOREACH (pt::ptree::value_type &obj,
                       json_pt.get_child("objects")) {
            std::string kind = obj.second.get<std::string>("kind");
            if (kind == "device") {
                str = obj.second.get<std::string>("outer-v4", "");
                if (str.empty() || !str.compare("true")) {
                    // fabric is IPv4 only
                    test_params->v4_outer = true;
                } else {
                    // fabric is IPv6 only
                    test_params->v4_outer = false;
                }
                test_params->device_mac =
                    std::stoull(obj.second.get<std::string>("mac-addr"), 0, 0);
                str2ipaddr(obj.second.get<std::string>("ip-addr").c_str(),
                           &test_params->device_ip);
                str2ipaddr(obj.second.get<std::string>("gw-ip-addr").c_str(),
                           &test_params->device_gw_ip);
                if (test_params->v4_outer) {
                    assert(test_params->device_ip.af == IP_AF_IPV4);
                    assert(test_params->device_gw_ip.af == IP_AF_IPV4);
                } else {
                    assert(test_params->device_ip.af == IP_AF_IPV6);
                    assert(test_params->device_gw_ip.af == IP_AF_IPV6);
                }
                if (!obj.second.get<std::string>("encap", "").compare("vxlan")) {
                    test_params->fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
                } else {
                    test_params->fabric_encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
                }
                test_params->dual_stack = false;
                if (!obj.second.get<std::string>("dual-stack").compare("true")) {
                    test_params->dual_stack = true;
                }

                // If env var is set, it overrides the json value
                if (getenv("APOLLO_TEST_TEP_ENCAP")) {
                    tep_encap_env = getenv("APOLLO_TEST_TEP_ENCAP");
                    if (!strcmp(tep_encap_env, "vxlan")) {
                        test_params->fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
                    } else {
                        test_params->fabric_encap.type =
                            PDS_ENCAP_TYPE_MPLSoUDP;
                    }
                    printf("TEP encap env var %s, encap %u\n",
                           tep_encap_env, test_params->fabric_encap.type);
                }
            } else if (kind == "tep") {
                test_params->num_teps =
                    std::stol(obj.second.get<std::string>("count"));
                if (test_params->num_teps <= 2) {
                    printf("No. of TEPs must be greater than 2\n");
                    exit(1);
                }
                // reduce num_teps by 2, (MyTEP and GW-TEP)
                test_params->num_teps -= 2;
                pfxstr = obj.second.get<std::string>("prefix");
                if (test_params->v4_outer) {
                    assert(str2ipv4pfx((char *)pfxstr.c_str(),
                                       &test_params->tep_pfx) == 0);
                } else {
                    assert(str2ipv6pfx((char *)pfxstr.c_str(),
                                       &test_params->tep_pfx) == 0);
                }
                pfxstr = obj.second.get<std::string>("svc-prefix", "");
                if (pfxstr.empty() == false) {
                    assert(str2ipv6pfx((char *)pfxstr.c_str(),
                                       &test_params->svc_tep_pfx) == 0);
                }
                pfxstr = obj.second.get<std::string>("remote-svc-public-prefix",
                                                     "");
                if (pfxstr.empty() == false) {
                    assert(str2ipv4pfx((char *)pfxstr.c_str(),
                                       &test_params->remote_svc_public_ip_pfx) == 0);
                }
            } else if (kind == "route-table") {
                test_params->num_routes = std::stol(obj.second.get<std::string>("count"));
                pfxstr = obj.second.get<std::string>("prefix-start");
                assert(str2ipv4pfx((char *)pfxstr.c_str(), &test_params->route_pfx) == 0);
                pfxstr = obj.second.get<std::string>("v6-prefix-start");
                assert(str2ipv6pfx((char *)pfxstr.c_str(), &test_params->v6_route_pfx) == 0);
            } else if (kind == "security-policy") {
                test_params->num_ipv4_rules = std::stol(obj.second.get<std::string>("v4-count"));
                if (test_params->num_ipv4_rules < 4) {
                    printf("Number of IPv4 rules in the policy table must be >= 4\n");
                }
                test_params->num_ipv6_rules = std::stol(obj.second.get<std::string>("v6-count"));
                if (test_params->num_ipv6_rules < 4) {
                    printf("Number of IPv6 rules in the policy table must be >= 4\n");
                }
                if (!artemis()) {
                    if (!obj.second.get<std::string>("stateful").compare("true")) {
                        test_params->stateful = true;
                    } else {
                        test_params->stateful = false;
                    }
                } else {
                    test_params->stateful = true;
                }
            } else if (kind == "vpc") {
                test_params->num_vpcs = std::stol(obj.second.get<std::string>("count"));
                pfxstr = obj.second.get<std::string>("prefix");
                assert(str2ipv4pfx((char *)pfxstr.c_str(), &test_params->vpc_pfx) == 0);
                pfxstr = obj.second.get<std::string>("v6-prefix");
                assert(str2ipv6pfx((char *)pfxstr.c_str(), &test_params->v6_vpc_pfx) == 0);
                test_params->num_subnets = std::stol(obj.second.get<std::string>("subnets"));
                pfxstr = obj.second.get<std::string>("nat46-prefix", "");
                if (pfxstr.empty() == false) {
                    assert(str2ipv6pfx((char *)pfxstr.c_str(),
                                       &test_params->nat46_vpc_pfx) == 0);
                }
            } else if (kind == "vnic") {
                string tag;
                test_params->num_vnics = std::stol(obj.second.get<std::string>("count"));
                tag = obj.second.get<std::string>("tagged", "");
                if (tag.empty() || !tag.compare("true")) {
                    test_params->tag_vnics = true;
                    test_params->vlan_start =
                        std::stol(obj.second.get<std::string>("vlan-start"));
                } else if (!tag.compare("false")) {
                    test_params->tag_vnics = false;
                } else {
                    printf("Unknown value %s for atrribute tagged under vnics, "
                           "value must be true | false", tag.c_str());
                    exit(1);
                }
            } else if (kind == "mappings") {
                pfxstr = obj.second.get<std::string>("nat-prefix");
                assert(str2ipv4pfx((char *)pfxstr.c_str(), &test_params->nat_pfx) == 0);
                pfxstr = obj.second.get<std::string>("v6-nat-prefix");
                assert(str2ipv6pfx((char *)pfxstr.c_str(), &test_params->v6_nat_pfx) == 0);
                test_params->num_remote_mappings =
                    std::stol(obj.second.get<std::string>("remotes"));
                test_params->num_ip_per_vnic =
                    std::stol(obj.second.get<std::string>("locals"));
                pfxstr = obj.second.get<std::string>("provider-prefix", "");
                if (!pfxstr.empty()) {
                    assert(str2ipv4pfx((char *)pfxstr.c_str(), &test_params->provider_pfx) == 0);
                }
                pfxstr = obj.second.get<std::string>("v6-provider-prefix", "");
                if (!pfxstr.empty()) {
                    assert(str2ipv6pfx((char *)pfxstr.c_str(), &test_params->v6_provider_pfx) == 0);
                }
            } else if (kind == "flows") {
                test_params->num_tcp = std::stol(obj.second.get<std::string>("num_tcp"));
                test_params->num_udp = std::stol(obj.second.get<std::string>("num_udp"));
                test_params->num_icmp = std::stol(obj.second.get<std::string>("num_icmp"));
                test_params->sport_lo = std::stol(obj.second.get<std::string>("sport_lo"));
                test_params->sport_hi = std::stol(obj.second.get<std::string>("sport_hi"));
                test_params->dport_lo = std::stol(obj.second.get<std::string>("dport_lo"));
                test_params->dport_hi = std::stol(obj.second.get<std::string>("dport_hi"));
            } else if (kind == "mirror") {
                if (!obj.second.get<std::string>("enable").compare("true")) {
                    test_params->mirror_en = true;
                    test_params->num_rspan = std::stol(obj.second.get<std::string>("rspan"));
                    test_params->num_erspan = std::stol(obj.second.get<std::string>("erspan"));
                    if ((test_params->num_rspan + test_params->num_erspan) >
                            PDS_MAX_MIRROR_SESSION) {
                        printf("Total no. of mirror sessions can't exceed %u",
                               PDS_MAX_MIRROR_SESSION);
                    }
                    for (i = 0; i < test_params->num_rspan; i++) {
                        test_params->rspan_bmap |= (1 << i);
                    }
                    for (; i < test_params->num_rspan + test_params->num_erspan; i++) {
                        //test_params->erspan_bmap |= (1 << i);
                    }
                }
            } else if (kind == "metering") {
                test_params->num_meter = std::stol(obj.second.get<std::string>("count"));
                test_params->meter_scale = std::stol(obj.second.get<std::string>("scale"));
                meter_str_to_type(obj.second.get<std::string>("type"), &test_params->meter_type);
                test_params->pps_bps = std::stol(obj.second.get<std::string>("pps_bps"));
                test_params->burst = std::stol(obj.second.get<std::string>("burst"));
            } else if (kind == "tags") {
                test_params->num_tag_trees = std::stol(obj.second.get<std::string>("count"));
                test_params->tags_v4_scale = std::stol(obj.second.get<std::string>("v4_scale"));
                test_params->tags_v6_scale = std::stol(obj.second.get<std::string>("v6_scale"));
            } else if (kind == "nexthop") {
                test_params->num_nh = std::stol(obj.second.get<std::string>("count"));
                // 1st IP in the TEP prefix is gateway IP, 2nd is MyTEP IP,
                // so skip the first 2 IPs
                test_params->num_nh -= 2;
            } else if (kind == "svc-mappings") {
                pfxstr = obj.second.get<std::string>("v4-vip-prefix");
                assert(str2ipv4pfx((char *)pfxstr.c_str(),
                                   &test_params->v4_vip_pfx) == 0);
                pfxstr = obj.second.get<std::string>("v6-vip-prefix");
                assert(str2ipv6pfx((char *)pfxstr.c_str(),
                                   &test_params->v6_vip_pfx) == 0);
            }
        }
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        exit(1);
        return SDK_RET_ERR;
    }
    test_params->svc_tep_vnid_base = 10000;
    test_params->remote_svc_tep_vnid_base = 20000;
    return SDK_RET_OK;
}

inline const char *
get_cfg_json_name (void) {
    return "scale_cfg.json";
}

static inline string
get_cfg_json (const char *pipeline) {
    const char *cfg_file_name = get_cfg_json_name();

    // read from ENV if set
    const char *test_cfg = getenv("TEST_CFG");
    if (test_cfg != NULL) {
        return string(test_cfg);
    }

    // read from /tmp/ if present
    string tmp_cfg_file = "/tmp/" + string(cfg_file_name);
    if (access(tmp_cfg_file.c_str(), R_OK) == 0) {
        return tmp_cfg_file;
    }

    // read from cfg path
    assert(getenv("CONFIG_PATH"));
#ifdef SIM
    return string(getenv("CONFIG_PATH")) + "/../apollo/test/scale/" +
           string(pipeline) + "/" + string(cfg_file_name);
#else
    return string(getenv("CONFIG_PATH")) + "/" + string(cfg_file_name);
#endif // SIM

    return "";
}

inline sdk_ret_t
parse_test_cfg (test_params_t *test_params, string pipeline)
{
    string cfg_file = get_cfg_json(pipeline.c_str());
    return parse_test_cfg(cfg_file.c_str(), test_params);
}

#endif // __APOLLO_SCALE_TEST_COMMON_HPP__

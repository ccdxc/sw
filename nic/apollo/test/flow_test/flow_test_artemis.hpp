//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __FLOW_TEST_ARTEMIS_HPP__
#define __FLOW_TEST_ARTEMIS_HPP__

#include <iostream>
#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "include/sdk/base.hpp"
#include "include/sdk/ip.hpp"
#include "include/sdk/table.hpp"
#include "lib/table/ftl/ftl_base.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/test/base/utils.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/test/scale/test_common.hpp"
#include "nic/apollo/p4/include/artemis_defines.h"
#include "gen/p4gen/artemis/include/p4pd.h"
#include "gen/p4gen/p4/include/ftl.h"
#include "gen/p4gen/p4/include/ftl_table.hpp"

using sdk::table::ftl_base;
using sdk::table::sdk_table_api_params_t;
using sdk::table::sdk_table_api_stats_t;
using sdk::table::sdk_table_stats_t;
using sdk::table::sdk_table_factory_params_t;

namespace pt = boost::property_tree;
#define FLOW_TEST_CHECK_RETURN(_exp, _ret) if (!(_exp)) return (_ret)
#define MAX_NEXTHOP_GROUP_INDEX 1024

FILE *g_fp;

char *
flow_key2str(void *key) {
    static char str[256];
    flow_swkey_t *k = (flow_swkey_t *)key;
    char srcstr[INET6_ADDRSTRLEN];
    char dststr[INET6_ADDRSTRLEN];

    if (k->key_metadata_ktype == 2) {
        inet_ntop(AF_INET6, k->key_metadata_src, srcstr, INET6_ADDRSTRLEN);
        inet_ntop(AF_INET6, k->key_metadata_dst, dststr, INET6_ADDRSTRLEN);
    } else {
        inet_ntop(AF_INET, k->key_metadata_src, srcstr, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, k->key_metadata_dst, dststr, INET_ADDRSTRLEN);
    }
#if 0
    sprintf(str, "T:%d SA:%s DA:%s DP:%d SP:%d P:%d VN:%d",
            k->key_metadata_ktype, srcstr, dststr,
            k->key_metadata_dport, k->key_metadata_sport,
            k->key_metadata_proto, k->vnic_metadata_vpc_id);
#else
#endif
    return str;
}

char *
flow_appdata2str(void *appdata) {
    static char str[512];
    flow_appdata_t *d = (flow_appdata_t *)appdata;
    sprintf(str, "I:%d R:%d",
            d->session_index, d->flow_role);
    return str;
}

static void
dump_flow_entry(flow_hash_entry_t *entry, ip_addr_t sip, ip_addr_t dip) {
#ifdef SIM
    if (g_fp) {
        fprintf(g_fp, "vpc %lu, proto %lu, session_index %lu, sip %s, dip %s, "
                "sport %lu, dport %lu, epoch %lu, flow %lu, ktype %lu\n",
                entry->vnic_metadata_vpc_id, entry->key_metadata_proto, entry->session_index,
                ipaddr2str(&sip), ipaddr2str(&dip), entry->key_metadata_sport, entry->key_metadata_dport,
                entry->epoch, entry->flow_role, entry->key_metadata_ktype);
        fflush(g_fp);
    }
#endif
}

static void
dump_flow_entry(ipv4_flow_hash_entry_t *entry, ip_addr_t sip, ip_addr_t dip) {
#ifdef SIM
    if (g_fp) {
        fprintf(g_fp, "vpc %lu, proto %lu, session_index %lu, sip %s, dip %s, "
                "sport %lu, dport %lu, epoch %lu, role %lu\n",
                entry->vnic_metadata_vpc_id, entry->key_metadata_proto, entry->session_index,
                ipaddr2str(&sip), ipaddr2str(&dip), entry->key_metadata_sport, entry->key_metadata_dport,
                entry->epoch, entry->flow_role);
        fflush(g_fp);
    }
#endif
}

static void
dump_session_info(uint32_t vpc, ip_addr_t ip_addr,
                  session_actiondata_t *actiondata)
{
    //if (vpc != TEST_APP_S3_VNET_IN_OUT_V6_OUTER) {
        //return;
    //}
#ifdef SIM
    if (g_fp) {
        fprintf(g_fp, "vpc %u, meter_idx %u, nh_idx %u, tx rewrite flags 0x%x, "
                "rx rewrite flags 0x%x, tx_dst_ip %s\n", vpc,
                actiondata->action_u.session_session_info.meter_idx,
                actiondata->action_u.session_session_info.nexthop_idx,
                actiondata->action_u.session_session_info.tx_rewrite_flags,
                actiondata->action_u.session_session_info.rx_rewrite_flags,
                ipaddr2str(&ip_addr));
        fflush(g_fp);
    }
#endif
}

#define MAX_VPCS        64
#define MAX_LOCAL_EPS   8
#define MAX_REMOTE_EPS  512
#define MAX_EP_PAIRS_PER_VPC (MAX_LOCAL_EPS*MAX_REMOTE_EPS)

typedef struct mapping_s {
    union {
        struct {
            ip_addr_t local_ip;
            ip_addr_t public_ip;
            ip_addr_t service_ip;
        };
        struct {
            ip_addr_t remote_ip;
        };
    };
    ip_addr_t provider_ip;
} mapping_t;

typedef struct vpc_epdb_s {
    uint32_t vpc_id;
    uint32_t valid;
    uint32_t v4_lcount;
    uint32_t v6_lcount;
    uint32_t v4_rcount;
    uint32_t v6_rcount;
    mapping_t v4_locals[MAX_LOCAL_EPS];
    mapping_t v6_locals[MAX_LOCAL_EPS];
    mapping_t v4_remotes[MAX_REMOTE_EPS];
    mapping_t v6_remotes[MAX_REMOTE_EPS];
} vpc_epdb_t;

typedef struct vpc_ep_pair_s {
    uint32_t vpc_id;
    uint32_t valid;
    mapping_t v4_local;
    mapping_t v6_local;
    mapping_t v4_remote;
    mapping_t v6_remote;
} vpc_ep_pair_t;

typedef struct cfg_params_s {
    bool valid;
    bool dual_stack;
    uint16_t sport_hi;
    uint16_t sport_lo;
    uint16_t dport_hi;
    uint16_t dport_lo;
    uint32_t num_tcp;
    uint32_t num_udp;
    uint32_t num_icmp;
} cfg_params_t;

class flow_test {
private:
    ftl_base *v6table;
    ftl_base *v4table;
    vpc_epdb_t epdb[MAX_VPCS+1];
    uint32_t session_index;
    uint32_t epoch;
    uint32_t hash;
    uint16_t sport_base;
    uint16_t sport;
    uint16_t dport_base;
    uint16_t dport;
    sdk_table_api_params_t params;
    sdk_table_factory_params_t factory_params;
    flow_hash_entry_t v6entry;
    ipv4_flow_hash_entry_t v4entry;
    vpc_ep_pair_t ep_pairs[MAX_EP_PAIRS_PER_VPC];
    cfg_params_t cfg_params;
    uint32_t nexthop_idx_start;
    uint32_t num_nexthop_idx_per_vpc;
    uint32_t svc_tep_nh_idx_start;
    uint32_t remote_svc_tep_nh_idx_start;
    uint32_t meter_idx;
    uint32_t num_meter_idx_per_vpc;
    bool with_hash;
    test_params_t *test_params;
    uint32_t ip_offset;

private:

    const char *get_cfg_json_() {
        auto p = api::g_pds_state.scale_profile();
        if (p == PDS_SCALE_PROFILE_DEFAULT) {
            return "scale_cfg.json";
        } else if (p == PDS_SCALE_PROFILE_P1) {
            return "scale_cfg_p1.json";
        } else if (p == PDS_SCALE_PROFILE_P2) {
            return "scale_cfg_p2.json";
        } else {
            assert(0);
        }
    }

    void parse_cfg_json_() {
        const char *pipeline = pipeline_get().c_str();
        static test_params_t test_params;
        memset(&test_params, 0, sizeof(test_params_t));
        parse_test_cfg(&test_params, pipeline);
        test_params.pipeline = pipeline;
        set_cfg_params(&test_params,
                       test_params.dual_stack,
                       test_params.num_tcp,
                       test_params.num_udp,
                       test_params.num_icmp,
                       test_params.sport_lo,
                       test_params.sport_hi,
                       test_params.dport_lo,
                       test_params.dport_hi);
#if defined(ARTEMIS)
        set_session_info_cfg_params(
            test_params.num_vpcs, test_params.num_ip_per_vnic,
            test_params.num_remote_mappings, test_params.meter_scale,
            TESTAPP_METER_NUM_PREFIXES, test_params.num_nh,
            TESTAPP_MAX_SERVICE_TEP, TESTAPP_MAX_REMOTE_SERVICE_TEP);
#endif
    }

    void show_cfg_params_() {
        assert(cfg_params.valid);
        SDK_TRACE_DEBUG("FLOW TEST CONFIG PARAMS");
        SDK_TRACE_DEBUG("- dual_stack : %d", cfg_params.dual_stack);
        SDK_TRACE_DEBUG("- num_tcp : %d", cfg_params.num_tcp);
        SDK_TRACE_DEBUG("- num_udp : %d", cfg_params.num_udp);
        SDK_TRACE_DEBUG("- num_icmp : %d", cfg_params.num_icmp);
        SDK_TRACE_DEBUG("- sport_lo : %d", cfg_params.sport_lo);
        SDK_TRACE_DEBUG("- sport_hi : %d", cfg_params.sport_hi);
        SDK_TRACE_DEBUG("- dport_lo : %d", cfg_params.dport_lo);
        SDK_TRACE_DEBUG("- dport_hi : %d", cfg_params.dport_hi);
    }


    sdk_ret_t insert_(flow_hash_entry_t *v6entry) {
        memset(&params, 0, sizeof(params));
        params.entry = v6entry;
        if (with_hash) {
            params.hash_valid = true;
            params.hash_32b = hash++;
        }
        params.entry_size = flow_hash_entry_t::entry_size();
        return v6table->insert(&params);
    }

    sdk_ret_t insert_(ipv4_flow_hash_entry_t *v4entry) {
        memset(&params, 0, sizeof(params));
        params.entry = v4entry;
        if (with_hash) {
            params.hash_valid = true;
            params.hash_32b = hash++;
        }
        params.entry_size = ipv4_flow_hash_entry_t::entry_size();
        return v4table->insert(&params);
    }

    sdk_ret_t remove_(flow_hash_entry_t *key) {
        sdk_table_api_params_t params = { 0 };
        params.key = key;
        return v6table->remove(&params);
    }

public:
    flow_test(bool w = false) {
        memset(&factory_params, 0, sizeof(factory_params));
        factory_params.entry_trace_en = false;
        v6table = flow_hash::factory(&factory_params);
        assert(v6table);

        memset(&factory_params, 0, sizeof(factory_params));
        factory_params.entry_trace_en = false;
        v4table = ipv4_flow_hash::factory(&factory_params);
        assert(v4table);

        memset(epdb, 0, sizeof(epdb));
        memset(&cfg_params, 0, sizeof(cfg_params_t));
        session_index = 1;
        epoch = 1;
        hash = 0;
        with_hash = w;
        ip_offset = 0;
    }

    void set_port_bases(uint16_t spbase, uint16_t dpbase) {
        sport_base = spbase;
        sport = spbase;
        dport_base = dpbase;
        dport = dpbase;
    }

    uint16_t alloc_sport(uint8_t proto) {
        if (proto == IP_PROTO_ICMP) {
            // Fix ICMP ID = 1
            return 1;
        } else if (proto == IP_PROTO_UDP) {
            return 100;
        } else {
            if (sport_base) {
                sport = sport + 1 ? sport + 1 : sport_base;
            } else {
                sport = 0;
            }
        }
        return sport;
    }

    uint16_t alloc_dport(uint8_t proto) {
        if (proto == 1) {
            // ECHO Request: type = 8, code = 0
            return 0x0800;
        } else if (proto == IP_PROTO_UDP) {
            return 100;
        } else {
            if (dport_base) {
                dport = dport + 1 ? dport + 1 : dport_base;
            } else {
                dport = 0;
            }
        }
        return dport;
    }

    ~flow_test() {
        flow_hash::destroy(v6table);
        ipv4_flow_hash::destroy(v4table);
    }

    void add_local_ep(pds_local_mapping_spec_t *local_spec) {
        uint32_t vpc_id = test::pdsobjkey2int(local_spec->key.vpc);
        ip_addr_t ip_addr = local_spec->key.ip_addr;

        assert(vpc_id);
        if (vpc_id > MAX_VPCS) {
            return;
        }
        epdb[vpc_id].vpc_id = vpc_id;
        if (ip_addr.af == IP_AF_IPV4) {
            if (epdb[vpc_id].v4_lcount >= MAX_LOCAL_EPS) {
                return;
            }
            epdb[vpc_id].valid = 1;
            epdb[vpc_id].v4_locals[epdb[vpc_id].v4_lcount].local_ip = ip_addr;
            if (local_spec->public_ip_valid) {
                epdb[vpc_id].v4_locals[epdb[vpc_id].v4_lcount].public_ip =
                                                        local_spec->public_ip;
            }
            if (local_spec->provider_ip_valid) {
                epdb[vpc_id].v4_locals[epdb[vpc_id].v4_lcount].provider_ip =
                                                        local_spec->provider_ip;
            }
            ip_addr_t service_ip;
            service_ip.af = IP_AF_IPV4;
            service_ip.addr.v4_addr =
                test_params->v4_vip_pfx.addr.addr.v4_addr | ip_offset;
            ip_offset++;
            epdb[vpc_id].v4_locals[epdb[vpc_id].v4_lcount].service_ip = service_ip;
            epdb[vpc_id].v4_lcount++;
        } else {
            if (epdb[vpc_id].v6_lcount >= MAX_LOCAL_EPS) {
                return;
            }
            epdb[vpc_id].valid = 1;
            epdb[vpc_id].v6_locals[epdb[vpc_id].v6_lcount].local_ip = ip_addr;
            if (local_spec->public_ip_valid) {
                epdb[vpc_id].v6_locals[epdb[vpc_id].v6_lcount].public_ip =
                                                        local_spec->public_ip;
            }
            if (local_spec->provider_ip_valid) {
                epdb[vpc_id].v6_locals[epdb[vpc_id].v6_lcount].provider_ip =
                                                        local_spec->provider_ip;
            }
            ip_addr_t service_ip;
            service_ip.af = IP_AF_IPV6;
            service_ip.addr.v6_addr = test_params->v6_vip_pfx.addr.addr.v6_addr;
            CONVERT_TO_V4_MAPPED_V6_ADDRESS(service_ip.addr.v6_addr,
                                            epdb[vpc_id].v4_locals[epdb[vpc_id].v6_lcount].
                                                        service_ip.addr.v4_addr);
            epdb[vpc_id].v6_locals[epdb[vpc_id].v6_lcount].service_ip = service_ip;
            epdb[vpc_id].v6_lcount++;
        }
        //printf("Adding Local EP: Vcn=%d IP=%#x\n", vpc_id, ip_addr);
    }

    void add_remote_ep(pds_remote_mapping_spec_t *remote_spec) {
        uint32_t vpc_id = test::pdsobjkey2int(remote_spec->key.vpc);
        ip_addr_t ip_addr = remote_spec->key.ip_addr;

        assert(vpc_id);
        if (vpc_id > MAX_VPCS) {
            return;
        }
        epdb[vpc_id].vpc_id = vpc_id;
        if (ip_addr.af == IP_AF_IPV4) {
            if (epdb[vpc_id].v4_rcount >= MAX_REMOTE_EPS) {
                return;
            }
            epdb[vpc_id].valid = 1;
            epdb[vpc_id].v4_remotes[epdb[vpc_id].v4_rcount].remote_ip = ip_addr;
            if (remote_spec->provider_ip_valid) {
                epdb[vpc_id].v4_remotes[epdb[vpc_id].v4_rcount].provider_ip =
                                                    remote_spec->provider_ip;
            }
            epdb[vpc_id].v4_rcount++;
        } else {
            if (epdb[vpc_id].v6_rcount >= MAX_REMOTE_EPS) {
                return;
            }
            epdb[vpc_id].valid = 1;
            epdb[vpc_id].v6_remotes[epdb[vpc_id].v6_rcount].remote_ip = ip_addr;
            if (remote_spec->provider_ip_valid) {
                epdb[vpc_id].v6_remotes[epdb[vpc_id].v6_rcount].provider_ip =
                                                    remote_spec->provider_ip;
            }
            epdb[vpc_id].v6_rcount++;
        }
        //printf("Adding Remote EP: Vcn=%d IP=%#x\n", vpc_id, ip_addr);
    }

    void generate_ep_pairs(uint32_t vpc, bool dual_stack) {
        uint32_t rcount = 0;
        if (dual_stack) {
            assert(epdb[vpc].v6_lcount == epdb[vpc].v4_lcount);
            assert(epdb[vpc].v6_rcount == epdb[vpc].v4_rcount);
        }
        uint32_t pid = 0;
        memset(ep_pairs, 0, sizeof(ep_pairs));
        if (epdb[vpc].valid == 0) {
            return;
        }
        // only TESTAPP_MAX_SERVICE_TEP remote service teps are valid
        if ((vpc == TEST_APP_S1_SVC_TUNNEL_IN_OUT) ||
            (vpc == TEST_APP_S1_REMOTE_SVC_TUNNEL_IN_OUT)) {
            rcount = TESTAPP_MAX_SERVICE_TEP;
        } else {
            rcount = epdb[vpc].v4_rcount;
        }
        for (uint32_t lid = 0; lid < epdb[vpc].v4_lcount; lid++) {
            for (uint32_t rid = 0; rid < rcount; rid++) {
                ep_pairs[pid].vpc_id = vpc;
                ep_pairs[pid].v4_local = epdb[vpc].v4_locals[lid];
                ep_pairs[pid].v6_local = epdb[vpc].v6_locals[lid];
                ep_pairs[pid].v4_remote = epdb[vpc].v4_remotes[rid];
                ep_pairs[pid].v6_remote = epdb[vpc].v6_remotes[rid];
                ep_pairs[pid].valid = 1;
                pid++;
            }
        }
    }

    void generate_dummy_epdb() {
        pds_local_mapping_spec_t local_spec;
        pds_remote_mapping_spec_t remote_spec;
        memset(&local_spec, 0, sizeof(local_spec));
        memset(&remote_spec, 0, sizeof(remote_spec));
        for (uint32_t vpc = 1; vpc < MAX_VPCS; vpc++) {
            epdb[vpc].vpc_id = vpc;
            local_spec.key.type = PDS_MAPPING_TYPE_L3;
            local_spec.key.vpc = test::int2pdsobjkey(vpc);
            remote_spec.key.type = PDS_MAPPING_TYPE_L3;
            remote_spec.key.vpc = test::int2pdsobjkey(vpc);
            for (uint32_t lid = 0; lid < MAX_LOCAL_EPS; lid++) {
                local_spec.key.ip_addr.af = IP_AF_IPV4;
                local_spec.key.ip_addr.addr.v4_addr = 0x0a000001 + lid;
                add_local_ep(&local_spec);
                remote_spec.key.ip_addr.af = IP_AF_IPV4;
                for (uint32_t rid = 0; rid < MAX_REMOTE_EPS; rid++) {
                    remote_spec.key.ip_addr.addr.v4_addr = 0x1400001 + rid;
                    add_remote_ep(&remote_spec);
                }
            }
        }
        return;
    }

    sdk_ret_t create_session_info(uint32_t vpc, ip_addr_t ip_addr,
                                  uint32_t nexthop_idx, uint32_t inh_idx,
                                  uint32_t svc_tep_nh_idx,
                                  uint32_t remote_svc_tep_nh_idx) {
        session_actiondata_t actiondata;
        p4pd_error_t p4pd_ret;
        uint32_t tableid = P4TBL_ID_SESSION;
        static uint32_t cur_vpc = 0;

        // reset meter_idx to corresponding start value for the VPC
        if (cur_vpc != vpc) {
            cur_vpc = vpc;
            meter_idx = (vpc - 1) * num_meter_idx_per_vpc + 1;
        }
        memset(&actiondata, 0, sizeof(session_actiondata_t));
        actiondata.action_id = SESSION_SESSION_INFO_ID;
        if (vpc == TEST_APP_S1_REMOTE_SVC_TUNNEL_IN_OUT) {
            // VPC 59 is used for Remote Scenario1-ST in/out traffic
            // Tx path:
            //     SMACi is unchnged
            //     DMACi is unchanged
            //     SIPi xlated to IPv6 (from LOCAL_46_TX table)
            //     DIPi xlated to IPv6 (comes from REMOTE_46_MAPPING table,
            //                          stored in the flow)
            //     L4 ports remain unchanged
            //     Vxlan encap is added (IPv4)
            //     SMACo is local device mac (table constant of EGRESS_VNIC_INFO)
            //     DMACo is from nexthop (comes from REMOTE_46_MAPPING table,
            //                            stored in flow table)
            //     SIPo is public IP of remote service tunnel (DIPo from NEXTHOP
            //     table)
            //     DIPo is last 32 bits of IPv6 address from REMOTE_46_MAPPING
            //     table
            //     vnid associated with the service tunnel (from NEXTHOP table)
            // Rx path:
            //     Use the outermost vnid as lookup key in TEP1_RX table and
            //     decap that header
            //     Use the inner vnid and DIPm (public IP assocaated with remote
            //     ST) as key into TEP2_RX
            //     Remove the vxlan encap after terminating based on received
            //     vnid
            //     IPv6 SIPi is xlated to IPv4 (last 32 bits of received IPv6
            //     packet)
            //     IPv6 DIPi is xlated to IPv4 (last 32 bits of received IPv6
            //     packet)
            //     vlan tag added from EGRESS_VNIC_INFO table
            // Objects to configure (vpc, subnet, vnic, tep are common)
            // 1. vpc should be configured with nat46 prefix
            // 2. local IP mapping (will inherit the nat46 prefix from vpc)
            // 3. tunnel of type SERVICE_TEP
            // 4. route pointing to NH of type (remote) service TEPs
            actiondata.action_u.session_session_info.nexthop_idx =
                remote_svc_tep_nh_idx;
            sdk::lib::memrev(actiondata.action_u.session_session_info.tx_dst_ip,
                             ip_addr.addr.v6_addr.addr8, sizeof(ipv6_addr_t));
            actiondata.action_u.session_session_info.tx_rewrite_flags = 0x66;
            actiondata.action_u.session_session_info.rx_rewrite_flags = 0x4;
        } else if (vpc == TEST_APP_S1_SVC_TUNNEL_IN_OUT) {
            // VPC 60 is used for Scenario1-ST in/out traffic
            // Tx path:
            //     SMACi is unchnged
            //     DMACi is unchanged
            //     SIPi xlated to IPv6 (from LOCAL_46_TX table)
            //     DIPi xlated to IPv6 (comes from REMOTE_46_MAPPING table,
            //                          stored in the flow)
            //     L4 ports remain unchanged
            //     Vxlan encap is added (IPv4)
            //     SMACo is local device mac (table constant of EGRESS_VNIC_INFO)
            //     DMACo is from nexthop (comes from REMOTE_46_MAPPING table,
            //                            stored in flow table)
            //     SIPo is provider IP of SIPi (from NAT table)
            //     DIPo is last 32 bits of IPv6 address from REMOTE_46_MAPPING
            //     table
            //     vnid corresponding to the service tunnel (from NEXTHOP table)
            // Rx path:
            //     Remove the vxlan encap after terminating based on received
            //     vnid
            //     IPv6 SIPi is xlated to IPv4 (last 32 bits of received IPv6
            //     packet)
            //     IPv6 DIPi is xlated to IPv4 (last 32 bits of received IPv6
            //     packet)
            //     vlan tag added from EGRESS_VNIC_INFO table
            // Objects to configure (vpc, subnet, vnic, tep are common)
            // 1. vpc should be configured with nat46 prefix
            // 2. local IP mapping (will inherit the nat46 prefix from vpc)
            // 3. tunnel of type SERVICE_TEP
            // 4. route pointing to NH of type service TEPs
            actiondata.action_u.session_session_info.nexthop_idx =
                svc_tep_nh_idx;
            sdk::lib::memrev(actiondata.action_u.session_session_info.tx_dst_ip,
                             ip_addr.addr.v6_addr.addr8, sizeof(ipv6_addr_t));
            actiondata.action_u.session_session_info.tx_rewrite_flags = 0x26;
            actiondata.action_u.session_session_info.rx_rewrite_flags = 0x4;
        } else if (vpc == TEST_APP_S1_SLB_IN_OUT) {
            // VPC 61 is used for Scenario1-SLB in/out traffic (DSR case)
            // Tx path:
            //     SMAC is rewritten with host MAC (table constant)
            //     DMAC is from NH (from routing table)
            //     (CA) SIP is xlated to VIP
            //     DIP is unchanged
            //     sport is xlated svc port
            //     dport are unchanged
            //     no vxlan encap is added
            // Rx path:
            //     Packet received is vxlan encapped
            //     remove the vxlan encap
            //     SMACi rewritten with VR MAC
            //     DMACi rewritten with VNIC's MAC
            //     SIPi untouched
            //     DIPi rewritten with CA IP of vnic
            //     sport is untouched
            //     dport is rewritten with xlated port from service mapping
            //     packet sent to vnic with appropriate vlan tag
            // Objects to configure (vpc, subnet, vnic, tep are common)
            // 1. local mapping with service mapping (IP, port)
            // 2. route pointing to NH of type IP
            actiondata.action_u.session_session_info.nexthop_idx = inh_idx;
            actiondata.action_u.session_session_info.tx_rewrite_flags = 0x45;
            actiondata.action_u.session_session_info.rx_rewrite_flags = 0x61;
        } else if (vpc == TEST_APP_S2_INTERNET_IN_OUT_VPC_VIP_VPC) {
            // VPC 63 is used for scenario2-Internet in/out traffic
            // Tx path:
            //     SMAC is rewritten with VR_MAC (EGRESS_VNIC_INFO table)
            //     - TBD: ToR expected to do this
            //     DMAC is rewritten with IGW MAC (NEXTHOP table)
            //     (CA-SIP, sport) is xlated from CA IPv4/IPv6 to
            //         (VIP, svc-port) IPv4/IPv6 (SERVICE_MAPPING table)
            //     DIP, dport are untouched
            //     No Vxlan encap is added on the way out
            //     Packet goes out with Internet vlan (i.e. bridge vnic's encap)
            // Rx path:
            //     Packet received is non-vxlan packet with Internet vlan (i.e.
            //     bridge vnic's encap)
            //     SMAC is rewritten with VR_MAC (EGRESS_VNIC_INFO table)
            //     DMAC is rewritten with LOCAL IP mapping's MAC (MAPPING table)
            //     - TBD: ToR expected to do this ?
            //     (SIP, sport) are untouched
            //     (DIP, dport) are xlated from from (VIP, svc-port) to
            //         (CA-IP, DIP-port) (SERVICE_MAPPING table)
            //     No Vxlan encap is added on the way out
            //     Packet goes out with vnic' vlan (EGRESS_VNIC_INFO table)
            // NH indices in this case should be between 1-1022 (NH_TYPE_IP)
            // Objects to configure (vpc, subnet, vnic, tep are common)
            // 1. local mapping with service mapping (IP, port)
            // 2. route pointing to NH of type IP
            actiondata.action_u.session_session_info.nexthop_idx = inh_idx;
            actiondata.action_u.session_session_info.tx_rewrite_flags = 0x5;
            actiondata.action_u.session_session_info.rx_rewrite_flags = 0x21;
        } else if (vpc == TEST_APP_S2_INTERNET_IN_OUT_FLOATING_IP_VPC) {
            // VPC 64 is used for scenario2-Internet in/out traffic
            // Tx path:
            //     SMAC is rewritten with VR_MAC (EGRESS_VNIC_INFO table)
            //     - TBD: ToR expected to do this
            //     DMAC is rewritten with IGW MAC (NEXTHOP table)
            //     SIP is xlated from CA IPv4/IPv6 to floating public IPv4/IPv6
            //     DIP is untouched
            //     L4 ports untouched
            //     No Vxlan encap is added on the way out
            //     Packet goes out with Internet vlan (i.e. bridge vnic's encap)
            // Rx path:
            //     Packet received is non-vxlan packet in Internet vlan (i.e.
            //     bridge vnic's encap)
            //     SMAC is rewritten with VR_MAC (EGRESS_VNIC_INFO table)
            //     DMAC is rewritten with LOCAL IP mapping's MAC (MAPPING table)
            //     - TBD: ToR expected to do this
            //     SIP is untouched
            //     DIP is xlated from floating public IPv4/IPv6 to CA IPv4/IPv6
            //     (LOCAL_IP_MAPPING table)
            //     L4 ports untouched
            //     No Vxlan encap is added on the way out
            //     Packet goes out with vnic' vlan (EGRESS_VNIC_INFO table)
            // NH indices in this case should be between 1-1022 (NH_TYPE_IP)
            // Objects to configure (vpc, subnet, vnic, tep are common)
            // 1. local mapping with public IP
            // 2. route pointing to NH of type IP
            actiondata.action_u.session_session_info.nexthop_idx = inh_idx;
            // do DMACi rewrite and encap in the Tx direction
            actiondata.action_u.session_session_info.tx_rewrite_flags = 0x3;
            // do SMACi rewrite with VR MAC and go with CA-IP always
            actiondata.action_u.session_session_info.rx_rewrite_flags = 0x11;
        } else {
            // VNET in/out case
            // Tx path:
            //     DMAC rewritten with remote IP mapping's MAC
            //     Encap the packet with vxlan
            // Rx path:
            //     Decap the packet
            //     put right vlan (if applicable)
            // Objects to configure (vpc, subnet, vnic, tep are common)
            // 1. local mapping
            // 2. remote mapping
            // NOTE: TEST_APP_S3_VNET_IN_OUT_V6_OUTER also will fall in this
            //       case (in this case local & remote mappings should be
            //       created with IPv6 provider IP)
            actiondata.action_u.session_session_info.nexthop_idx = nexthop_idx;
            // do DMACi rewrite and encap in the Tx direction
            actiondata.action_u.session_session_info.tx_rewrite_flags = 0x21;
            // just decap in the Rx direction, no rewrites
            actiondata.action_u.session_session_info.rx_rewrite_flags = 0x00;
        }
        actiondata.action_u.session_session_info.meter_idx = meter_idx++;
        if (meter_idx == vpc * num_meter_idx_per_vpc) {
            meter_idx = (vpc-1) * num_meter_idx_per_vpc + 1;
        }
        dump_session_info(vpc, ip_addr, &actiondata);
        p4pd_ret = p4pd_global_entry_write(
                            tableid, session_index, NULL, NULL, &actiondata);
        if (p4pd_ret != P4PD_SUCCESS) {
            SDK_TRACE_ERR("Failed to create session info index %u",
                          session_index);
            return SDK_RET_ERR;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t create_flow(uint32_t vpc, uint8_t proto,
                          ip_addr_t flow_sip, ip_addr_t flow_dip,
                          uint16_t flow_sport, uint16_t flow_dport,
                          uint8_t role) {
        sdk_ret_t ret;
        if (flow_sip.af == IP_AF_IPV4) {
            v4entry.clear();
            // Common DATA fields
            v4entry.session_index = session_index;
            v4entry.epoch = 0xFF;
            // Common KEY fields
            v4entry.vnic_metadata_vpc_id = vpc;
            v4entry.key_metadata_proto = proto;
            // Create IFLOW
            v4entry.key_metadata_sport = flow_sport;
            v4entry.key_metadata_dport = flow_dport;
            v4entry.key_metadata_ipv4_src = flow_sip.addr.v4_addr;
            v4entry.key_metadata_ipv4_dst = flow_dip.addr.v4_addr;
            v4entry.flow_role = role;
            ret = insert_(&v4entry);
            SDK_ASSERT(ret == SDK_RET_OK);
            //if (vpc == TEST_APP_S3_VNET_IN_OUT_V6_OUTER) {
                dump_flow_entry(&v4entry, flow_sip, flow_dip);
            //}
        } else if (flow_sip.af == IP_AF_IPV6) {
            v6entry.clear();
            // Common DATA fields
            v6entry.session_index = session_index;
            v6entry.epoch = 0xFF;
            // Common KEY fields
            v6entry.key_metadata_ktype = 2;
            v6entry.vnic_metadata_vpc_id = vpc;
            v6entry.key_metadata_proto = proto;
            v6entry.key_metadata_sport = flow_sport;
            v6entry.key_metadata_dport = flow_dport;
            sdk::lib::memrev(v6entry.key_metadata_src, flow_sip.addr.v6_addr.addr8,
                             sizeof(ipv6_addr_t));
            sdk::lib::memrev(v6entry.key_metadata_dst, flow_dip.addr.v6_addr.addr8,
                             sizeof(ipv6_addr_t));
            v6entry.flow_role = role;
            ret = insert_(&v6entry);
            SDK_ASSERT(ret == SDK_RET_OK);
            //if (vpc == TEST_APP_S3_VNET_IN_OUT_V6_OUTER) {
                dump_flow_entry(&v6entry, flow_sip, flow_dip);
            //}
        }
        return SDK_RET_OK;
    }

    sdk_ret_t create_session(uint32_t vpc, uint8_t proto,
                             ip_addr_t iflow_sip, ip_addr_t iflow_dip,
                             uint16_t iflow_sport, uint16_t iflow_dport,
                             ip_addr_t rflow_sip, ip_addr_t rflow_dip,
                             uint16_t rflow_sport, uint16_t rflow_dport) {
        sdk_ret_t ret;
        if (iflow_sip.af == IP_AF_IPV4) {
            v4entry.clear();

            // Common DATA fields
            v4entry.session_index = session_index;
            v4entry.epoch = 0xFF;

            // Common KEY fields
            v4entry.vnic_metadata_vpc_id = vpc;
            v4entry.key_metadata_proto = proto;

            // Create IFLOW
            v4entry.key_metadata_sport = iflow_sport;
            v4entry.key_metadata_dport = iflow_dport;
            v4entry.key_metadata_ipv4_src = iflow_sip.addr.v4_addr;
            v4entry.key_metadata_ipv4_dst = iflow_dip.addr.v4_addr;
            v4entry.flow_role = TCP_FLOW_INITIATOR;
            ret = insert_(&v4entry);
            SDK_ASSERT(ret == SDK_RET_OK);
            //if (vpc == TEST_APP_S3_VNET_IN_OUT_V6_OUTER) {
                dump_flow_entry(&v4entry, iflow_sip, iflow_dip);
            //}

            // Create RFLOW
            v4entry.key_metadata_sport = rflow_sport;
            v4entry.key_metadata_dport = rflow_dport;
            v4entry.key_metadata_ipv4_src = rflow_sip.addr.v4_addr;
            v4entry.key_metadata_ipv4_dst = rflow_dip.addr.v4_addr;
            v4entry.flow_role = TCP_FLOW_RESPONDER;
            ret = insert_(&v4entry);
            SDK_ASSERT(ret == SDK_RET_OK);
            //if (vpc == TEST_APP_S3_VNET_IN_OUT_V6_OUTER) {
                dump_flow_entry(&v4entry, rflow_sip, rflow_dip);
            //}
        } else {
            v6entry.clear();

            // Common DATA fields
            v6entry.session_index = session_index;
            v6entry.epoch = 0xFF;

            // Common KEY fields
            v6entry.key_metadata_ktype = 2;
            v6entry.vnic_metadata_vpc_id = vpc;
            v6entry.key_metadata_proto = proto;

            // create IFLOW
            v6entry.key_metadata_sport = iflow_sport;
            v6entry.key_metadata_dport = iflow_dport;
            v6entry.flow_role = TCP_FLOW_INITIATOR;
            sdk::lib::memrev(v6entry.key_metadata_src, iflow_sip.addr.v6_addr.addr8,
                             sizeof(ipv6_addr_t));
            sdk::lib::memrev(v6entry.key_metadata_dst, iflow_dip.addr.v6_addr.addr8,
                             sizeof(ipv6_addr_t));
            ret = insert_(&v6entry);
            SDK_ASSERT(ret == SDK_RET_OK);
            //if (vpc == TEST_APP_S3_VNET_IN_OUT_V6_OUTER) {
                dump_flow_entry(&v6entry, iflow_sip, iflow_dip);
            //}

            // create RFLOW
            v6entry.key_metadata_sport = rflow_sport;
            v6entry.key_metadata_dport = rflow_dport;
            sdk::lib::memrev(v6entry.key_metadata_src, rflow_sip.addr.v6_addr.addr8,
                             sizeof(ipv6_addr_t));
            sdk::lib::memrev(v6entry.key_metadata_dst, rflow_dip.addr.v6_addr.addr8,
                             sizeof(ipv6_addr_t));
            v6entry.flow_role = TCP_FLOW_RESPONDER;
            ret = insert_(&v6entry);
            SDK_ASSERT(ret == SDK_RET_OK);
            //if (vpc == TEST_APP_S3_VNET_IN_OUT_V6_OUTER) {
                dump_flow_entry(&v6entry, rflow_sip, rflow_dip);
            //}
        }
        return SDK_RET_OK;
    }

    sdk_ret_t create_flows_one_proto_(uint32_t count,
                                      uint8_t proto, bool dual_stack) {
        uint16_t local_port = 0, remote_port = 0;
        uint32_t i = 0;
        uint16_t fwd_sport = 0, fwd_dport = 0;
        uint16_t rev_sport = 0, rev_dport = 0;
        uint32_t nflows = 0;
        sdk_ret_t ret;
        ip_addr_t ip_addr;
        ip_addr_t rewrite_ip;
        uint32_t nexthop_idx = nexthop_idx_start;
        uint32_t inh_idx = 1;
        uint32_t svc_tep_nh_idx = svc_tep_nh_idx_start;
        uint32_t remote_svc_tep_nh_idx = remote_svc_tep_nh_idx_start;
        uint32_t dport_hi = cfg_params.dport_hi;
        uint32_t sport_hi = cfg_params.sport_hi;
        uint32_t num_sport;
        uint32_t num_dport;
        uint32_t num_flows_per_local;

        for (uint32_t vpc = 1; vpc <= MAX_VPCS; vpc++) {
            generate_ep_pairs(vpc, dual_stack);
            nexthop_idx = nexthop_idx_start +
                             (vpc - 1) * num_nexthop_idx_per_vpc;
            for (i = 0; i < MAX_EP_PAIRS_PER_VPC ; i++) {
                num_sport = cfg_params.sport_hi - cfg_params.sport_lo + 1;
                num_dport = cfg_params.dport_hi - cfg_params.dport_lo + 1;
                switch (vpc) {
                case TEST_APP_S1_SVC_TUNNEL_IN_OUT:
                case TEST_APP_S1_REMOTE_SVC_TUNNEL_IN_OUT:
                    // only TESTAPP_MAX_SERVICE_TEP remotes are installed.
                    // adjust dport range so that total flows per local =
                    // num_remotes * num_sport * num_dport
                    //
                    // locals * remotes * sport * dport
                    // 8      * 64      * 2     * 64
                    num_flows_per_local = test_params->num_remote_mappings *
                                          num_sport * num_dport;
                    dport_hi = cfg_params.dport_lo +
                               num_flows_per_local/(TESTAPP_MAX_SERVICE_TEP * num_sport) - 1;
                    sport_hi = cfg_params.sport_hi;
                    break;
                case TEST_APP_S1_SLB_IN_OUT:
                case TEST_APP_S2_INTERNET_IN_OUT_VPC_VIP_VPC:
                    // since sport is rewritten, adjust dport range so that total
                    // flows are maintained
                    //
                    // locals * remotes * sport * dport
                    // 8      * 512      * 1    * 16
                    dport_hi = cfg_params.dport_lo + num_sport * num_dport - 1;
                    sport_hi = cfg_params.sport_lo;
                    break;
                default:
                    // locals * remotes * sport * dport
                    // 8      * 512      * 2    * 8
                    dport_hi = cfg_params.dport_hi;
                    sport_hi = cfg_params.sport_hi;
                    break;
                }
                for (auto lp = cfg_params.sport_lo; lp <= sport_hi; lp++) {
                    for (auto rp = cfg_params.dport_lo; rp <= dport_hi; rp++) {
                        local_port = lp;
                        remote_port = rp;
                        if (ep_pairs[i].valid == 0) {
                            break;
                        }
                        if (proto == IP_PROTO_ICMP) {
                            fwd_sport = rev_sport = local_port;
                            fwd_dport = rev_dport = remote_port;
                        } else {
                            fwd_sport = rev_dport = local_port;
                            fwd_dport = rev_sport = remote_port;
                        }

                        memset(&ip_addr, 0, sizeof(ip_addr));
                        // create V4 Flows
                        if ((vpc == TEST_APP_S1_SVC_TUNNEL_IN_OUT) ||
                            (vpc == TEST_APP_S1_REMOTE_SVC_TUNNEL_IN_OUT)) {
                            // VPC 60 is used for Scenario1-ST in/out traffic
                            if (vpc == TEST_APP_S1_SVC_TUNNEL_IN_OUT) {
                                ip_addr.addr.v4_addr = TESTAPP_V4ROUTE_VAL(i % TESTAPP_MAX_SERVICE_TEP);
                            } else {
                                ip_addr.addr.v4_addr =
                                    TESTAPP_V4ROUTE_VAL(TESTAPP_MAX_SERVICE_TEP + (i % TESTAPP_MAX_SERVICE_TEP));
                            }

                            // iflow v4 session
                            ret = create_flow(vpc, proto,
                                              ep_pairs[i].v4_local.local_ip,
                                              ip_addr, fwd_sport, fwd_dport,
                                              TCP_FLOW_INITIATOR);
                            if (ret != SDK_RET_OK) {
                                return ret;
                            }
                            rewrite_ip = test_params->svc_tep_pfx.addr;
                            compute_remote46_addr(&rewrite_ip,
                                &test_params->svc_tep_pfx,
                                (vpc == TEST_APP_S1_SVC_TUNNEL_IN_OUT) ?
                                    (i % TESTAPP_MAX_SERVICE_TEP) :
                                    ((i % TESTAPP_MAX_SERVICE_TEP) + TESTAPP_MAX_SERVICE_TEP));
                            rewrite_ip.addr.v6_addr.addr32[IP6_ADDR32_LEN-1] =
                                htonl(ip_addr.addr.v4_addr);
                            ip_addr_t rflow_dip =
                                test_params->nat46_vpc_pfx.addr;
                            rflow_dip.addr.v6_addr.addr32[IP6_ADDR32_LEN-1] =
                                htonl(ep_pairs[i].v4_local.local_ip.addr.v4_addr);

                            // rflow v6 session
                            ret = create_flow(vpc, proto, rewrite_ip, rflow_dip,
                                              fwd_dport, fwd_sport,
                                              TCP_FLOW_RESPONDER);
                        } else if (vpc == TEST_APP_S1_SLB_IN_OUT) {
                            // this VPC is used for Scenario1-SLB in/out traffic (DSR case)
                            ip_addr.addr.v4_addr = TESTAPP_V4ROUTE_VAL(i);
                            ret = create_session(vpc, proto,
                                                 ep_pairs[i].v4_local.local_ip,
                                                 ip_addr,
                                                 TEST_APP_DIP_PORT,
                                                 fwd_dport,
                                                 ip_addr,
                                                 ep_pairs[i].v4_local.service_ip,
                                                 fwd_dport,
                                                 TEST_APP_VIP_PORT);
                        } else if (vpc == TEST_APP_S2_INTERNET_IN_OUT_VPC_VIP_VPC) {
                            // this VPC is for Internet IN/OUT with VIP & svc
                            // port IP flows
                            ip_addr.addr.v4_addr = TESTAPP_V4ROUTE_VAL(i);
                            ret = create_session(vpc, proto,
                                                 ep_pairs[i].v4_local.local_ip,
                                                 ip_addr,
                                                 TEST_APP_DIP_PORT,
                                                 fwd_dport,
                                                 ip_addr,
                                                 ep_pairs[i].v4_local.service_ip, // must be service IP
                                                 fwd_dport,
                                                 TEST_APP_VIP_PORT);
                        } else if (vpc == TEST_APP_S2_INTERNET_IN_OUT_FLOATING_IP_VPC) {
                            // this VPC is for Internet IN/OUT with floating
                            // IP flows
                            ip_addr.addr.v4_addr = TESTAPP_V4ROUTE_VAL(i);
                            ret = create_session(vpc, proto,
                                                 ep_pairs[i].v4_local.local_ip,
                                                 ip_addr,
                                                 fwd_sport, fwd_dport,
                                                 ip_addr,
                                                 ep_pairs[i].v4_local.public_ip, /* must be public IP */
                                                 fwd_dport, fwd_sport);
                        } else {
                            // vnet in/out with vxlan encap
                            ret = create_session(vpc, proto,
                                                 ep_pairs[i].v4_local.local_ip,
                                                 ep_pairs[i].v4_remote.remote_ip,
                                                 fwd_sport, fwd_dport,
                                                 ep_pairs[i].v4_remote.remote_ip,
                                                 ep_pairs[i].v4_local.local_ip,
                                                 fwd_dport, fwd_sport);
                        }
                        if (ret != SDK_RET_OK) {
                            return ret;
                        }
                        // create V4 session
                        ret = create_session_info(vpc, rewrite_ip, nexthop_idx,
                                                  inh_idx, svc_tep_nh_idx,
                                                  remote_svc_tep_nh_idx);
                        if (ret != SDK_RET_OK) {
                            return ret;
                        }
                        session_index++;

                        if (dual_stack &&
                            ((vpc != TEST_APP_S1_SVC_TUNNEL_IN_OUT) &&
                             (vpc != TEST_APP_S1_REMOTE_SVC_TUNNEL_IN_OUT))) {
                            memset(&ip_addr, 0, sizeof(ip_addr));
                            // create V6 Flows
                            switch (vpc) {
                            case TEST_APP_S1_SLB_IN_OUT:
                            case TEST_APP_S2_INTERNET_IN_OUT_VPC_VIP_VPC:
                                ip_addr = test_params->v6_route_pfx.addr;
                                ip_addr.addr.v6_addr.addr32[IP6_ADDR32_LEN-2]
                                    = htonl(0xF1D0D1D0);
                                ip_addr.addr.v6_addr.addr32[IP6_ADDR32_LEN-1] =
                                            htonl(TESTAPP_V4ROUTE_VAL(i));
                                ret = create_session(vpc, proto,
                                          ep_pairs[i].v6_local.local_ip,
                                          ip_addr,
                                          TEST_APP_DIP_PORT,
                                          fwd_dport,
                                          ip_addr,
                                          ep_pairs[i].v6_local.service_ip,
                                          fwd_dport,
                                          TEST_APP_VIP_PORT);
                                break;

                            case TEST_APP_S2_INTERNET_IN_OUT_FLOATING_IP_VPC:
                                // this VPC is for Internet IN/OUT with
                                // floating IP address
                                ip_addr = test_params->v6_route_pfx.addr;
                                ip_addr.addr.v6_addr.addr32[IP6_ADDR32_LEN-2]
                                    = htonl(0xF1D0D1D0);
                                ip_addr.addr.v6_addr.addr32[IP6_ADDR32_LEN-1] =
                                            htonl(TESTAPP_V4ROUTE_VAL(i));
                                ret = create_session(vpc, proto,
                                          ep_pairs[i].v6_local.local_ip,
                                          ip_addr,
                                          fwd_sport, fwd_dport,
                                          ip_addr,
                                          ep_pairs[i].v6_local.public_ip,
                                          fwd_dport, fwd_sport);
                                break;

                            default:
                                // vnet in/out with vxlan encap
                                ret = create_session(vpc, proto,
                                                     ep_pairs[i].v6_local.local_ip,
                                                     ep_pairs[i].v6_remote.remote_ip,
                                                     fwd_sport, fwd_dport,
                                                     ep_pairs[i].v6_remote.remote_ip,
                                                     ep_pairs[i].v6_local.local_ip,
                                                     fwd_dport, fwd_sport);
                                break;
                            }
                            if (ret != SDK_RET_OK) {
                                return ret;
                            }
                            // create V6 session
                            ret = create_session_info(vpc, rewrite_ip,
                                                      nexthop_idx+1, inh_idx,
                                                      svc_tep_nh_idx,
                                                      remote_svc_tep_nh_idx);
                            if (ret != SDK_RET_OK) {
                                return ret;
                            }
                            session_index++;
                        }

                        nflows+=2;
                        if (nflows >= count-1) {
                            return SDK_RET_OK;
                        }
                    }
                }
                // update nexthop_idx only when dst changes
                switch (vpc) {
                    case TEST_APP_S1_SLB_IN_OUT:
                    case TEST_APP_S2_INTERNET_IN_OUT_VPC_VIP_VPC:
                    case TEST_APP_S2_INTERNET_IN_OUT_FLOATING_IP_VPC:
                        if (i % (0x1 << (32 - TESTAPP_V4ROUTE_PREFIX_LEN)) == 0) {
                            inh_idx++;
                            if (inh_idx > this->test_params->num_nh) {
                                inh_idx = 1;
                            }
                        }
                        break;
                    case TEST_APP_S1_SVC_TUNNEL_IN_OUT:
                        svc_tep_nh_idx++;
                        if (svc_tep_nh_idx ==
                                svc_tep_nh_idx_start + TESTAPP_MAX_SERVICE_TEP) {
                            svc_tep_nh_idx = svc_tep_nh_idx_start;
                        }
                        break;
                    case TEST_APP_S1_REMOTE_SVC_TUNNEL_IN_OUT:
                        remote_svc_tep_nh_idx++;
                        if (remote_svc_tep_nh_idx ==
                                remote_svc_tep_nh_idx_start + TESTAPP_MAX_REMOTE_SERVICE_TEP) {
                            remote_svc_tep_nh_idx =
                                remote_svc_tep_nh_idx_start;
                        }
                        break;
                    default:
                        nexthop_idx += 2 /* for both v4 and v6 */;
                        if (nexthop_idx ==
                                (nexthop_idx_start + (vpc * num_nexthop_idx_per_vpc))) {
                            nexthop_idx = nexthop_idx_start +
                                                       (vpc - 1) * num_nexthop_idx_per_vpc;
                        }
                        break;
                }
            }
        }
        return SDK_RET_OK;
    }

    sdk_ret_t get_v4flow_stats(sdk_table_api_stats_t *api_stats,
                               sdk_table_stats_t *table_stats) {
        return v4table->stats_get(api_stats, table_stats);
    }

    sdk_ret_t get_v6flow_stats(sdk_table_api_stats_t *api_stats,
                               sdk_table_stats_t *table_stats) {
        return v6table->stats_get(api_stats, table_stats);
    }

    void print_flow_stats(sdk_table_api_stats_t *api_stats,
                          sdk_table_stats_t *table_stats) {
        if (g_fp) {
            fprintf(g_fp,
                    "insert %lu, insert_duplicate %lu, insert_fail %lu, "
                    "remove %lu, remove_not_found %lu, remove_fail %lu, "
                    "update %lu, update_fail %lu, "
                    "get %lu, get_fail %lu, "
                    "reserve %lu, reserver_fail %lu, "
                    "release %lu, release_fail %lu, "
                    "entries %lu, collisions %lu\n",
                    api_stats->insert,
                    api_stats->insert_duplicate,
                    api_stats->insert_fail,
                    api_stats->remove,
                    api_stats->remove_not_found,
                    api_stats->remove_fail,
                    api_stats->update,
                    api_stats->update_fail,
                    api_stats->get,
                    api_stats->get_fail,
                    api_stats->reserve,
                    api_stats->reserve_fail,
                    api_stats->release,
                    api_stats->release_fail,
                    table_stats->entries,
                    table_stats->collisions);
            fflush(g_fp);
        }
    }

    sdk_ret_t dump_flow_stats(void) {
        sdk_table_api_stats_t api_stats;
        sdk_table_stats_t table_stats;

        memset(&api_stats, 0, sizeof(sdk_table_api_stats_t));
        memset(&table_stats, 0, sizeof(sdk_table_stats_t));
        get_v4flow_stats(&api_stats, &table_stats);
        print_flow_stats(&api_stats, &table_stats);

        memset(&api_stats, 0, sizeof(sdk_table_api_stats_t));
        memset(&table_stats, 0, sizeof(sdk_table_stats_t));
        get_v6flow_stats(&api_stats, &table_stats);
        print_flow_stats(&api_stats, &table_stats);
        return SDK_RET_OK;
    }

    sdk_ret_t create_flows_all_protos_(bool dual_stack) {
        if (cfg_params.num_tcp) {
            auto ret = create_flows_one_proto_(cfg_params.num_tcp, IP_PROTO_TCP, dual_stack);
            if (ret != SDK_RET_OK) {
                return ret;
            }
        }

        if (cfg_params.num_udp) {
            auto ret = create_flows_one_proto_(cfg_params.num_udp, IP_PROTO_UDP, dual_stack);
            if (ret != SDK_RET_OK) {
                return ret;
            }
        }

        if (cfg_params.num_icmp) {
            auto ret = create_flows_one_proto_(cfg_params.num_icmp, IP_PROTO_ICMP, dual_stack);
            if (ret != SDK_RET_OK) {
                return ret;
            }
        }
        return SDK_RET_OK;
    }

    void set_session_info_cfg_params(
                            uint32_t num_vpcs, uint32_t num_ip_per_vnic,
                            uint32_t num_remote_mappings, uint32_t meter_scale,
                            uint32_t meter_pfx_per_rule, uint32_t num_nh,
                            uint32_t num_svc_teps,
                            uint32_t num_remote_svc_teps) {
        nexthop_idx_start = num_nh                             /* NH for routes */
                            + (2 * TESTAPP_MAX_SERVICE_TEP)    /* NH for SVC mappings + Remote SVC mappings */
                            + (num_ip_per_vnic * num_vpcs * 2) /* NH for IPv4/v6 local IP mappings */
                            + 1;
        num_nexthop_idx_per_vpc = num_remote_mappings * 2;
        num_meter_idx_per_vpc = (meter_scale/meter_pfx_per_rule) +
                                (meter_scale % meter_pfx_per_rule);
        svc_tep_nh_idx_start = num_nh + 1;
        remote_svc_tep_nh_idx_start =
            svc_tep_nh_idx_start + num_remote_svc_teps;
    }

    void set_cfg_params(test_params_t *test_params,
                        bool dual_stack, uint32_t num_tcp,
                        uint32_t num_udp, uint32_t num_icmp,
                        uint16_t sport_lo, uint16_t sport_hi,
                        uint16_t dport_lo, uint16_t dport_hi) {
        this->test_params = test_params;
        cfg_params.dual_stack = dual_stack;
        cfg_params.num_tcp = num_tcp;
        cfg_params.num_udp = num_udp;
        cfg_params.sport_lo = sport_lo;
        cfg_params.sport_hi = sport_hi;
        cfg_params.dport_lo = dport_lo;
        cfg_params.dport_hi = dport_hi;
        cfg_params.valid = true;
        show_cfg_params_();
    }

    sdk_ret_t create_flows() {
        g_fp = fopen("/sw/nic/flow_log.log", "w+");
        if (cfg_params.valid == false) {
            parse_cfg_json_();
        }

        auto ret = create_flows_all_protos_(cfg_params.dual_stack);
        if (ret != SDK_RET_OK) {
            return ret;
        }

        dump_flow_stats();
        return SDK_RET_OK;
    }

    sdk_ret_t delete_flows(void) {
        // Not implemented
        SDK_ASSERT(0);
        return SDK_RET_OK;
    }

    sdk_ret_t iterate_flows(sdk::table::iterate_t table_entry_iterate) {
        sdk_table_api_params_t params = { 0 };

        params.itercb = table_entry_iterate;

        params.entry_size = ipv4_flow_hash_entry_t::entry_size();
        v4table->iterate(&params);

        params.entry_size = flow_hash_entry_t::entry_size();
        v6table->iterate(&params);
        return SDK_RET_OK;
    }

    sdk_ret_t clear_flows() {
       sdk_table_api_params_t params = {0};
       params.entry_size = ipv4_flow_hash_entry_t::entry_size();
       v4table->clear(true, true, &params);

       params.entry_size = flow_hash_entry_t::entry_size();
       v6table->clear(true, true, &params);
       return SDK_RET_OK;
    }
};

#endif // __APOLLO_SCALE_FLOW_TEST_HPP__

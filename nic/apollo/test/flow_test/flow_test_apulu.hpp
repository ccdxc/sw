//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __FLOW_TEST_APOLLO_HPP__
#define __FLOW_TEST_APOLLO_HPP__
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
#include "nic/utils/ftl/ftlv4.hpp"
#include "nic/utils/ftl/ftlv6.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"
#include "nic/apollo/p4/include/apulu_defines.h"

using sdk::table::ftlv6;
using sdk::table::ftlv4;
using sdk::table::sdk_table_api_params_t;
using sdk::table::sdk_table_api_stats_t;
using sdk::table::sdk_table_stats_t;
using sdk::table::sdk_table_factory_params_t;

namespace pt = boost::property_tree;
#define FLOW_TEST_CHECK_RETURN(_exp, _ret) if (!(_exp)) return (_ret)
#define MAX_NEXTHOP_GROUP_INDEX 1024

FILE *g_fp;

static char *
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
            k->key_metadata_proto, k->key_metadata_lkp_id);
#else
#endif
    return str;
}

static char *
flow_appdata2str(void *appdata) {
    static char str[512];
    flow_appdata_t *d = (flow_appdata_t *)appdata;
    sprintf(str, "I:%d R:%d",
            d->session_id, d->flow_role);
    return str;
}

static void
dump_flow_entry(ftlv6_entry_t *entry, ipv6_addr_t v6_addr_sip,
                ipv6_addr_t v6_addr_dip) {
    // return;
    char *src_ip_str = ipv6addr2str(v6_addr_sip);
    char *dst_ip_str = ipv6addr2str(v6_addr_dip);
    if (g_fp) {
        fprintf(g_fp, "proto %d, session_id %d, sip %s, dip %s, sport %d, dport %d, "
                "nexthop_group_index %d, flow_role %d, ktype %d, bd_id %d\n",
                entry->proto,
                entry->session_id,
                src_ip_str,
                dst_ip_str,
                entry->sport,
                entry->dport,
                entry->nexthop_id,
                entry->flow_role,
                entry->ktype,
                entry->bd_id);
        fflush(g_fp);
    }
}

static void
dump_flow_entry(ftlv4_entry_t *entry, ipv4_addr_t v4_addr_sip,
                ipv4_addr_t v4_addr_dip) {
    // return;
    char *src_ip_str = ipv4addr2str(v4_addr_sip);
    char *dst_ip_str = ipv4addr2str(v4_addr_dip);
    if (g_fp) {
        fprintf(g_fp, "proto %d, session_id %d, sip %s, dip %s, sport %d, dport %d, "
                "nexthop_group_index %d, flow_role %d, bd_id %d\n",
                entry->proto,
                entry->session_id,
                src_ip_str,
                dst_ip_str,
                entry->sport,
                entry->dport,
                entry->nexthop_id,
                entry->flow_role,
                entry->bd_id);
        fflush(g_fp);
    }
}

static void
dump_session_info(uint32_t vpc,
                  session_actiondata_t *actiondata)
{
#ifdef SIM
    if (g_fp) {
        fprintf(g_fp, "vpc %u, tx rewrite flags 0x%x, "
                "rx rewrite flags 0x%x\n", vpc,
                actiondata->action_u.session_session_info.tx_rewrite_flags,
                actiondata->action_u.session_session_info.rx_rewrite_flags);
        fflush(g_fp);
    }
#endif
}

#define MAX_VPCS        512
#define MAX_LOCAL_EPS   32
#define MAX_REMOTE_EPS  1024
#define MAX_EP_PAIRS_PER_VPC (MAX_LOCAL_EPS*MAX_REMOTE_EPS)

typedef struct vpc_epdb_s {
    uint32_t vpc_id;
    uint32_t valid;
    uint32_t v4_lcount;
    uint32_t v6_lcount;
    uint32_t v4_rcount;
    uint32_t v6_rcount;
    uint32_t lips[MAX_LOCAL_EPS];
    ipv6_addr_t lip6s[MAX_LOCAL_EPS];
    uint32_t rips[MAX_REMOTE_EPS];
    ipv6_addr_t rip6s[MAX_REMOTE_EPS];
} vpc_epdb_t;

typedef struct vpc_ep_pair_s {
    uint32_t vpc_id;
    uint32_t lip;
    ipv6_addr_t lip6;
    uint32_t rip;
    ipv6_addr_t rip6;
    uint32_t valid;
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
    ftlv6 *v6table;
    ftlv4 *v4table;
    vpc_epdb_t epdb[MAX_VPCS+1];
    uint32_t session_id;
    uint32_t nexthop_group_index;
    uint32_t hash;
    uint16_t sport_base;
    uint16_t sport;
    uint16_t dport_base;
    uint16_t dport;
    sdk_table_api_params_t params;
    sdk_table_factory_params_t factory_params;
    ftlv6_entry_t v6entry;
    ftlv4_entry_t v4entry;
    vpc_ep_pair_t ep_pairs[MAX_EP_PAIRS_PER_VPC];
    cfg_params_t cfg_params;
    bool with_hash;
    uint32_t session_index;

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
        pt::ptree json_pt;
        char cfgfile[256];

        assert(getenv("CONFIG_PATH"));
#ifdef SIM
        snprintf(cfgfile, 256, "%s/../apollo/test/scale/%s", getenv("CONFIG_PATH"), get_cfg_json_());
#else
        snprintf(cfgfile, 256, "%s/%s", getenv("CONFIG_PATH"), get_cfg_json_());
#endif

        // parse the config and create objects
        std::ifstream json_cfg(cfgfile);
        read_json(json_cfg, json_pt);
        try {
            BOOST_FOREACH (pt::ptree::value_type &obj,
                           json_pt.get_child("objects")) {
                std::string kind = obj.second.get<std::string>("kind");
                if (kind == "device") {
                    cfg_params.dual_stack = false;
                    if (!obj.second.get<std::string>("dual-stack").compare("true")) {
                        cfg_params.dual_stack = true;
                    }
                } else if (kind == "flows") {
                    cfg_params.num_tcp = std::stol(obj.second.get<std::string>("num_tcp"));
                    cfg_params.num_udp = std::stol(obj.second.get<std::string>("num_udp"));
                    cfg_params.num_icmp = std::stol(obj.second.get<std::string>("num_icmp"));
                    cfg_params.sport_lo = std::stol(obj.second.get<std::string>("sport_lo"));
                    cfg_params.sport_hi = std::stol(obj.second.get<std::string>("sport_hi"));
                    cfg_params.dport_lo = std::stol(obj.second.get<std::string>("dport_lo"));
                    cfg_params.dport_hi = std::stol(obj.second.get<std::string>("dport_hi"));
                }
            }

            cfg_params.valid = true;
            show_cfg_params_();
        } catch (std::exception const &e) {
            std::cerr << e.what() << std::endl;
            assert(0);
        }
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


    sdk_ret_t insert_(ftlv6_entry_t *v6entry) {
        memset(&params, 0, sizeof(params));
        params.entry = v6entry;
        if (with_hash) {
            params.hash_valid = true;
            params.hash_32b = hash++;
        }
        return v6table->insert(&params);
    }

    sdk_ret_t insert_(ftlv4_entry_t *v4entry) {
        memset(&params, 0, sizeof(params));
        params.entry = v4entry;
        if (with_hash) {
            params.hash_valid = true;
            params.hash_32b = hash++;
        }
        return v4table->insert(&params);
    }

    sdk_ret_t remove_(ftlv6_entry_t *key) {
        sdk_table_api_params_t params = { 0 };
        params.key = key;
        return v6table->remove(&params);
    }

public:
    flow_test(bool w = false) {
        memset(&factory_params, 0, sizeof(factory_params));
        factory_params.table_id = P4TBL_ID_FLOW;
        factory_params.num_hints = 4;
        factory_params.max_recircs = 8;
        factory_params.key2str = flow_key2str;
        factory_params.appdata2str = flow_appdata2str;
        factory_params.entry_trace_en = true;
        v6table = ftlv6::factory(&factory_params);
        assert(v6table);

        memset(&factory_params, 0, sizeof(factory_params));
        factory_params.table_id = P4TBL_ID_IPV4_FLOW;
        factory_params.num_hints = 2;
        factory_params.max_recircs = 8;
        factory_params.key2str = NULL;
        factory_params.appdata2str = NULL;
        factory_params.entry_trace_en = true;
        v4table = ftlv4::factory(&factory_params);
        assert(v4table);

        memset(epdb, 0, sizeof(epdb));
        memset(&cfg_params, 0, sizeof(cfg_params_t));
        session_id = 1;
        nexthop_group_index = 1;
        hash = 0;
        with_hash = w;
        session_index = 1;
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
        ftlv6::destroy(v6table);
        ftlv4::destroy(v4table);
    }

    void add_local_ep(pds_local_mapping_spec_t *local_spec) {
        uint32_t vpc_id = local_spec->key.vpc.id;
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
            epdb[vpc_id].lips[epdb[vpc_id].v4_lcount] = ip_addr.addr.v4_addr;
            epdb[vpc_id].v4_lcount++;
        } else {
            if (epdb[vpc_id].v6_lcount >= MAX_LOCAL_EPS) {
                return;
            }
            epdb[vpc_id].valid = 1;
            epdb[vpc_id].lip6s[epdb[vpc_id].v6_lcount] = ip_addr.addr.v6_addr;
            epdb[vpc_id].v6_lcount++;
        }
        //printf("Adding Local EP: Vcn=%d IP=%#x\n", vpc_id, ip_addr);
    }

    void add_remote_ep(pds_remote_mapping_spec_t *remote_spec) {
        uint32_t vpc_id = remote_spec->key.vpc.id;
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
            epdb[vpc_id].rips[epdb[vpc_id].v4_rcount] = ip_addr.addr.v4_addr;
            epdb[vpc_id].v4_rcount++;
        } else {
            if (epdb[vpc_id].v6_rcount >= MAX_REMOTE_EPS) {
                return;
            }
            epdb[vpc_id].valid = 1;
            epdb[vpc_id].rip6s[epdb[vpc_id].v6_rcount] = ip_addr.addr.v6_addr;
            epdb[vpc_id].v6_rcount++;
        }
        //printf("Adding Remote EP: Vcn=%d IP=%#x\n", vpc_id, ip_addr);
    }

    void generate_ep_pairs(uint32_t vpc, bool ipv6) {
        auto lcount = ipv6 ? epdb[vpc].v6_lcount : epdb[vpc].v4_lcount;
        auto rcount = ipv6 ? epdb[vpc].v6_rcount : epdb[vpc].v4_rcount;
        uint32_t pid = 0;
        memset(ep_pairs, 0, sizeof(ep_pairs));
        if (epdb[vpc].valid == 0) {
            return;
        }
        for (uint32_t lid = 0; lid < lcount; lid++) {
            for (uint32_t rid = 0; rid < rcount; rid++) {
                ep_pairs[pid].vpc_id = vpc;
                ep_pairs[pid].lip = epdb[vpc].lips[lid];
                ep_pairs[pid].lip6 = epdb[vpc].lip6s[lid];
                ep_pairs[pid].rip = epdb[vpc].rips[rid];
                ep_pairs[pid].rip6 = epdb[vpc].rip6s[rid];
                ep_pairs[pid].valid = 1;
                //printf("Appending EP pair: Vcn=%d LIP=%#x RIP=%#x\n", vpc,
                //       epdb[vpc].lips[lid], epdb[vpc].rips[rid]);
                pid++;
            }
        }
    }

    void generate_dummy_epdb() {
        ipv6_addr_t sip6 = {0};
        ipv6_addr_t dip6 = {0};
        pds_local_mapping_spec_t local_spec;
        pds_remote_mapping_spec_t remote_spec;
        memset(&local_spec, 0, sizeof(local_spec));
        memset(&remote_spec, 0, sizeof(remote_spec));
        for (uint32_t vpc = 1; vpc < MAX_VPCS; vpc++) {
            epdb[vpc].vpc_id = vpc;
            local_spec.key.type = PDS_MAPPING_TYPE_L3;
            local_spec.key.vpc.id = vpc;
            remote_spec.key.type = PDS_MAPPING_TYPE_L3;
            remote_spec.key.vpc.id = vpc;
            for (uint32_t lid = 0; lid < MAX_LOCAL_EPS; lid++) {
                local_spec.key.ip_addr.af = IP_AF_IPV4;
                local_spec.key.ip_addr.addr.v4_addr = 0x0a000001 + lid;
                add_local_ep(&local_spec);

                sip6.addr32[0] = 0x22;
                sip6.addr32[3] = lid;
                local_spec.key.ip_addr.af = IP_AF_IPV6;
                local_spec.key.ip_addr.addr.v6_addr = sip6;
                add_local_ep(&local_spec);

                for (uint32_t rid = 0; rid < MAX_REMOTE_EPS; rid++) {
                    remote_spec.key.ip_addr.af = IP_AF_IPV4;
                    remote_spec.key.ip_addr.addr.v4_addr = 0x1400001 + rid;
                    add_remote_ep(&remote_spec);
                    dip6.addr32[0] = 0x33;
                    dip6.addr32[3] = rid;
                    remote_spec.key.ip_addr.af = IP_AF_IPV6;
                    remote_spec.key.ip_addr.addr.v6_addr = dip6;
                    add_remote_ep(&remote_spec);
                }
            }
        }
        return;
    }

    sdk_ret_t create_session_info(uint32_t vpc, uint32_t session_index) {
        p4pd_error_t p4pd_ret;
        uint32_t tableid = P4TBL_ID_SESSION;
        session_actiondata_t actiondata;

        memset(&actiondata, 0, sizeof(session_actiondata_t));
        actiondata.action_id = SESSION_SESSION_INFO_ID;

        actiondata.action_u.session_session_info.tx_rewrite_flags =
                 ((TX_REWRITE_SMAC_FROM_VRMAC << TX_REWRITE_SMAC_START) |
                  (TX_REWRITE_DMAC_FROM_MAPPING << TX_REWRITE_DMAC_START) |
                  (TX_REWRITE_ENCAP_VXLAN << TX_REWRITE_ENCAP_START));

        actiondata.action_u.session_session_info.rx_rewrite_flags =
               ((RX_REWRITE_DMAC_FROM_MAPPING << RX_REWRITE_DMAC_START) |
                (RX_REWRITE_ENCAP_VLAN << RX_REWRITE_ENCAP_START));

        dump_session_info(vpc, &actiondata);

        p4pd_ret = p4pd_global_entry_write(
                            tableid, session_index, NULL, NULL, &actiondata);
        if (p4pd_ret != P4PD_SUCCESS) {
            SDK_TRACE_ERR("Failed to create session info index %u",
                          session_index);
            return SDK_RET_ERR;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t create_flow(uint32_t vpc, ipv6_addr_t v6_addr_sip,
                          ipv6_addr_t v6_addr_dip, uint8_t proto,
                          uint16_t sport, uint16_t dport) {
        memset(&v6entry, 0, sizeof(ftlv6_entry_t));
        v6entry.ktype = 2;
        v6entry.bd_id = vpc - 1;
        v6entry.sport = sport;
        v6entry.dport = dport;
        v6entry.proto = proto;
        sdk::lib::memrev(v6entry.src, v6_addr_sip.addr8, sizeof(ipv6_addr_t));
        sdk::lib::memrev(v6entry.dst, v6_addr_dip.addr8, sizeof(ipv6_addr_t));
        v6entry.session_id = session_id++;
        v6entry.set_nhgroup_index(nexthop_group_index++);

        // reset nexthop_group_index if it reaches max
        if (nexthop_group_index == MAX_NEXTHOP_GROUP_INDEX) {
            nexthop_group_index = 1;
        }
        auto ret = insert_(&v6entry);
        if (ret != SDK_RET_OK) {
            return ret;
        }
        // print entry info
        dump_flow_entry(&v6entry, v6_addr_sip, v6_addr_dip);
        return SDK_RET_OK;
    }

    sdk_ret_t create_flow(uint32_t vpc, ipv4_addr_t v4_addr_sip,
                          ipv4_addr_t v4_addr_dip, uint8_t proto,
                          uint16_t sport, uint16_t dport) {
        memset(&v4entry, 0, sizeof(ftlv4_entry_t));
        v4entry.bd_id = vpc - 1;
        v4entry.sport = sport;
        v4entry.dport = dport;
        v4entry.proto = proto;
        v4entry.src = v4_addr_sip;
        v4entry.dst = v4_addr_dip;
        v4entry.session_id = session_id++;
        v4entry.set_nhgroup_index(nexthop_group_index++);

        // reset nexthop_group_index if it reaches max
        if (nexthop_group_index == MAX_NEXTHOP_GROUP_INDEX) {
            nexthop_group_index = 1;
        }
        auto ret = insert_(&v4entry);
        if (ret != SDK_RET_OK) {
            return ret;
        }
        // print entry info
        dump_flow_entry(&v4entry, v4_addr_sip, v4_addr_dip);
        return SDK_RET_OK;
    }

    sdk_ret_t create_flows_one_proto_(uint32_t count, uint8_t proto, bool ipv6) {
        uint16_t local_port = 0, remote_port = 0;
        uint32_t i = 0;
        uint16_t fwd_sport = 0, fwd_dport = 0;
        uint16_t rev_sport = 0, rev_dport = 0;
        uint32_t nflows = 0;

        for (uint32_t vpc = 1; vpc < MAX_VPCS; vpc++) {
            generate_ep_pairs(vpc, ipv6);
            for (i = 0; i < MAX_EP_PAIRS_PER_VPC ; i++) {
                for (auto lp = cfg_params.sport_lo; lp <= cfg_params.sport_hi; lp++) {
                    for (auto rp = cfg_params.dport_lo; rp <= cfg_params.dport_hi; rp++) {
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

                        if (ipv6) {
                            auto ret = create_flow(vpc, ep_pairs[i].lip6,
                                                   ep_pairs[i].rip6, proto,
                                                   fwd_sport, fwd_dport);
                            if (ret != SDK_RET_OK) {
                                return ret;
                            }
                        } else {
                            auto ret = create_flow(vpc, ep_pairs[i].lip, ep_pairs[i].rip,
                                                   proto, fwd_sport, fwd_dport);
                            if (ret != SDK_RET_OK) {
                                return ret;
                            }
                        }
                        create_session_info(vpc, session_index++);
                        nflows++;
                        if (nflows >= count) {
                            return SDK_RET_OK;
                        }
                    }
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
        SDK_TRACE_DEBUG(
                "insert %u, insert_duplicate %u, insert_fail %u, "
                "remove %u, remove_not_found %u, remove_fail %u, "
                "update %u, update_fail %u, "
                "get %u, get_fail %u, "
                "reserve %u, reserver_fail %u, "
                "release %u, release_fail %u, "
                "entries %u, collisions %u",
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

    sdk_ret_t create_flows_all_protos_(bool ipv6) {
        if (cfg_params.num_tcp) {
            auto ret = create_flows_one_proto_(cfg_params.num_tcp, IP_PROTO_TCP, ipv6);
            if (ret != SDK_RET_OK) {
                return ret;
            }
        }

        if (cfg_params.num_udp) {
            auto ret = create_flows_one_proto_(cfg_params.num_udp, IP_PROTO_UDP, ipv6);
            if (ret != SDK_RET_OK) {
                return ret;
            }
        }

        if (cfg_params.num_icmp) {
            auto ret = create_flows_one_proto_(cfg_params.num_icmp, IP_PROTO_ICMP, ipv6);
            if (ret != SDK_RET_OK) {
                return ret;
            }
        }
        return SDK_RET_OK;
    }

    void set_cfg_params(test_params_t *test_params,
                        bool dual_stack, uint32_t num_tcp,
                        uint32_t num_udp, uint32_t num_icmp,
                        uint16_t sport_lo, uint16_t sport_hi,
                        uint16_t dport_lo, uint16_t dport_hi) {
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
        g_fp = fopen("/data/flow_log.log", "w+");

        if (cfg_params.valid == false) {
            parse_cfg_json_();
        }

        // Create V4 Flows
        auto ret = create_flows_all_protos_(false);
        if (ret != SDK_RET_OK) {
            return ret;
        }

        if (cfg_params.dual_stack) {
            ret = create_flows_all_protos_(true);
            if (ret != SDK_RET_OK) {
                return ret;
            }
        }
        dump_flow_stats();
        return SDK_RET_OK;
    }

    sdk_ret_t clear_flows() {
       v4table->clear(true, true);
       v6table->clear(true, true);
       return SDK_RET_OK;
    }
};

#endif    // __FLOW_TEST_APOLLO_HPP__

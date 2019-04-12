//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __APOLLO_SCALE_FLOW_TEST_HPP__
#define __APOLLO_SCALE_FLOW_TEST_HPP__
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
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/sdk/lib/utils/utils.hpp"

#include "gen/p4gen/apollo/include/p4pd.h"

using sdk::table::mem_hash;
using sdk::table::sdk_table_api_params_t;
using sdk::table::sdk_table_api_stats_t;
using sdk::table::sdk_table_stats_t;
using sdk::table::sdk_table_factory_params_t;

namespace pt = boost::property_tree;
#define FLOW_TEST_CHECK_RETURN(_exp, _ret) if (!(_exp)) return (_ret)

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
    sprintf(str, "T:%d SA:%s DA:%s DP:%d SP:%d P:%d VN:%d",
            k->key_metadata_ktype, srcstr, dststr,
            k->key_metadata_dport, k->key_metadata_sport,
            k->key_metadata_proto, k->vnic_metadata_local_vnic_tag);
    return str;
}

static char *
flow_appdata2str(void *appdata) {
    static char str[512];
    flow_appdata_t *d = (flow_appdata_t *)appdata;
    sprintf(str, "I:%d R:%d",
            d->flow_index, d->flow_role);
    return str;
}

#define MAX_VCNS        512
#define MAX_LOCAL_EPS   32
#define MAX_REMOTE_EPS  1024
#define MAX_EP_PAIRS_PER_VCN (MAX_LOCAL_EPS*MAX_REMOTE_EPS)

typedef struct vcn_epdb_s {
    uint32_t vcn_id;
    uint32_t valid;
    uint32_t v4_lcount;
    uint32_t v6_lcount;
    uint32_t v4_rcount;
    uint32_t v6_rcount;
    uint32_t lips[MAX_LOCAL_EPS];
    ipv6_addr_t lip6s[MAX_LOCAL_EPS];
    uint32_t rips[MAX_REMOTE_EPS];
    ipv6_addr_t rip6s[MAX_REMOTE_EPS];
} vcn_epdb_t;

typedef struct vcn_ep_pair_s {
    uint32_t vcn_id;
    uint32_t lip;
    ipv6_addr_t lip6;
    uint32_t rip;
    ipv6_addr_t rip6;
    uint32_t valid;
} vcn_ep_pair_t;

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
    mem_hash *table;
    vcn_epdb_t epdb[MAX_VCNS];
    uint32_t flow_index;
    uint32_t hash;
    uint16_t sport_base;
    uint16_t sport;
    uint16_t dport_base;
    uint16_t dport;
    sdk_table_api_params_t params;
    sdk_table_factory_params_t factory_params;
    flow_swkey_t swkey;
    flow_appdata_t swappdata;
    vcn_ep_pair_t ep_pairs[MAX_EP_PAIRS_PER_VCN];
    cfg_params_t cfg_params;
    bool with_hash;

private:

    const char *get_cfg_json_() {
        auto p = api::g_pds_state.scale_profile();
        if (p == PDS_SCALE_PROFILE_DEFAULT) {
            return "scale_cfg.json";
        } else if (p == PDS_SCALE_PROFILE_P1) {
            return "scale_cfg_p1.json";
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


    sdk_ret_t insert_(flow_swkey_t *key,
                      flow_appdata_t *appdata) {
        memset(&params, 0, sizeof(params));
        params.key = key;
        params.appdata = appdata;
        if (with_hash) {
            params.hash_valid = true;
            params.hash_32b = hash++;
        }
        return table->insert(&params);
    }

    sdk_ret_t remove_(flow_swkey_t *key) {
        sdk_table_api_params_t params = { 0 };
        params.key = key;
        return table->remove(&params);
    }

    void add_local_ep_(uint32_t vcn_id, uint32_t ipaddr) {
        assert(vcn_id);
        if (vcn_id > MAX_VCNS) {
            return;
        }
        if (epdb[vcn_id].v4_lcount >= MAX_LOCAL_EPS) {
            return;
        }
        epdb[vcn_id].valid = 1;
        epdb[vcn_id].vcn_id = vcn_id;
        epdb[vcn_id].lips[epdb[vcn_id].v4_lcount] = ipaddr;
        epdb[vcn_id].v4_lcount++;
        //printf("Adding Local EP: Vcn=%d IP=%#x\n", vcn_id, ipaddr);
    }

    void add_local_ep_(uint32_t vcn_id, ipv6_addr_t ip6addr) {
        assert(vcn_id);
        if (vcn_id > MAX_VCNS) {
            return;
        }
        if (epdb[vcn_id].v6_lcount >= MAX_LOCAL_EPS) {
            return;
        }
        epdb[vcn_id].valid = 1;
        epdb[vcn_id].vcn_id = vcn_id;
        epdb[vcn_id].lip6s[epdb[vcn_id].v6_lcount] = ip6addr;
        epdb[vcn_id].v6_lcount++;
        //printf("Adding Local EP: Vcn=%d IP=%#x\n", vcn_id, ipaddr);
    }

    void add_remote_ep_(uint32_t vcn_id, uint32_t ipaddr) {
        assert(vcn_id);
        if (vcn_id > MAX_VCNS) {
            return;
        }
        if (epdb[vcn_id].v4_rcount >= MAX_REMOTE_EPS) {
            return;
        }
        epdb[vcn_id].valid = 1;
        epdb[vcn_id].vcn_id = vcn_id;
        epdb[vcn_id].rips[epdb[vcn_id].v4_rcount] = ipaddr;
        epdb[vcn_id].v4_rcount++;
        //printf("Adding Remote EP: Vcn=%d IP=%#x\n", vcn_id, ipaddr);
    }

    void add_remote_ep_(uint32_t vcn_id, ipv6_addr_t ip6addr) {
        assert(vcn_id);
        if (vcn_id > MAX_VCNS) {
            return;
        }
        if (epdb[vcn_id].v6_rcount >= MAX_REMOTE_EPS) {
            return;
        }
        epdb[vcn_id].valid = 1;
        epdb[vcn_id].vcn_id = vcn_id;
        epdb[vcn_id].rip6s[epdb[vcn_id].v6_rcount] = ip6addr;
        epdb[vcn_id].v6_rcount++;
        //printf("Adding Remote EP: Vcn=%d IP=%#x\n", vcn_id, ipaddr);
    }


public:
    flow_test(bool w = false) {
        memset(&factory_params, 0, sizeof(factory_params));

        factory_params.table_id = P4TBL_ID_FLOW;
        factory_params.num_hints = 5;
        factory_params.max_recircs = 8;
        factory_params.key2str = flow_key2str;
        factory_params.appdata2str = flow_appdata2str;
        table = mem_hash::factory(&factory_params);
        assert(table);

        memset(epdb, 0, sizeof(epdb));
        memset(&cfg_params, 0, sizeof(cfg_params_t));
        flow_index = 0;
        hash = 0;
        with_hash = w;

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
        mem_hash::destroy(table);
    }

    void add_local_ep(uint32_t vcn_id, ip_addr_t ipaddr) {
        if (ipaddr.af == IP_AF_IPV4) {
            return add_local_ep_(vcn_id, ipaddr.addr.v4_addr);
        } else {
            return add_local_ep_(vcn_id, ipaddr.addr.v6_addr);
        }
    }

    void add_remote_ep(uint32_t vcn_id, ip_addr_t ipaddr) {
        if (ipaddr.af == IP_AF_IPV4) {
            return add_remote_ep_(vcn_id, ipaddr.addr.v4_addr);
        } else {
            return add_remote_ep_(vcn_id, ipaddr.addr.v6_addr);
        }
    }

    void generate_ep_pairs(uint32_t vcn, bool ipv6) {
        auto lcount = ipv6 ? epdb[vcn].v6_lcount : epdb[vcn].v4_lcount;
        auto rcount = ipv6 ? epdb[vcn].v6_rcount : epdb[vcn].v4_rcount;
        uint32_t pid = 0;
        memset(ep_pairs, 0, sizeof(ep_pairs));
        if (epdb[vcn].valid == 0) {
            return;
        }
        for (uint32_t lid = 0; lid < lcount; lid++) {
            for (uint32_t rid = 0; rid < rcount; rid++) {
                ep_pairs[pid].vcn_id = vcn;
                ep_pairs[pid].lip = epdb[vcn].lips[lid];
                ep_pairs[pid].lip6 = epdb[vcn].lip6s[lid];
                ep_pairs[pid].rip = epdb[vcn].rips[rid];
                ep_pairs[pid].rip6 = epdb[vcn].rip6s[rid];
                ep_pairs[pid].valid = 1;
                //printf("Appending EP pair: Vcn=%d LIP=%#x RIP=%#x\n", vcn,
                //       epdb[vcn].lips[lid], epdb[vcn].rips[rid]);
                pid++;
            }
        }
    }

    void generate_dummy_epdb() {
        for (uint32_t vcn = 1; vcn < MAX_VCNS; vcn++) {
            epdb[vcn].vcn_id = vcn;
            for (uint32_t lid = 0; lid < MAX_LOCAL_EPS; lid++) {
                add_local_ep_(vcn, 0x0a000001 + lid);
                for (uint32_t rid = 0; rid < MAX_REMOTE_EPS; rid++) {
                    add_remote_ep_(vcn, 0x1400001 + rid);
                }
            }
        }
        return;
    }

    sdk_ret_t create_flows_one_proto_(uint32_t count, uint8_t proto, bool ipv6) {
        uint16_t local_port = 0, remote_port = 0;
        sdk_ret_t ret = SDK_RET_OK;
        uint32_t i = 0;
        uint16_t fwd_sport = 0, fwd_dport = 0;
        uint16_t rev_sport = 0, rev_dport = 0;
        uint32_t nflows = 0;

        memset(&swkey, 0, sizeof(swkey));
        memset(&swappdata, 0, sizeof(swappdata));
        if (ipv6) {
            swkey.key_metadata_ktype = 2;
        } else {
            swkey.key_metadata_ktype = 1;
        }

        for (uint32_t vcn = 1; vcn < MAX_VCNS; vcn++) {
            generate_ep_pairs(vcn, ipv6);
            for (i = 0; i < MAX_EP_PAIRS_PER_VCN ; i++) {
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

                        // Local to Remote Flow
                        swkey.vnic_metadata_local_vnic_tag = vcn - 1;
                        swkey.key_metadata_sport = fwd_sport;
                        swkey.key_metadata_dport = fwd_dport;
                        swkey.key_metadata_proto = proto;
                        if (ipv6) {
                            sdk::lib::memrev(swkey.key_metadata_src, ep_pairs[i].lip6.addr8, sizeof(ipv6_addr_t));
                            sdk::lib::memrev(swkey.key_metadata_dst, ep_pairs[i].rip6.addr8, sizeof(ipv6_addr_t));
                        } else {
                            memcpy(&(swkey.key_metadata_src), &ep_pairs[i].lip, sizeof(uint32_t));
                            memcpy(&(swkey.key_metadata_dst), &ep_pairs[i].rip, sizeof(uint32_t));
                        }
                        swappdata.flow_index = flow_index++;
                        ret = insert_(&swkey, &swappdata);
                        if (ret != SDK_RET_OK) {
                            return ret;
                        }

                        nflows++;
                        if (nflows >= count) {
                            return SDK_RET_OK;
                        }
#if 0 // Only create local to remote flows for now.
                        // Remote to Local Flow
                        swkey.vnic_metadata_local_vnic_tag = vcn - 1;
                        swkey.key_metadata_sport = rev_sport;
                        swkey.key_metadata_dport = rev_dport;
                        swkey.key_metadata_proto = proto;
                        if (ipv6) {
                            memcpy(&(swkey.key_metadata_src), &ep_pairs[i].rip6, sizeof(ipv6_addr_t));
                            memcpy(&(swkey.key_metadata_dst), &ep_pairs[i].lip6, sizeof(ipv6_addr_t));
                        } else {
                            memcpy(&(swkey.key_metadata_src), &ep_pairs[i].rip, sizeof(uint32_t));
                            memcpy(&(swkey.key_metadata_dst), &ep_pairs[i].lip, sizeof(uint32_t));
                        }
                        swappdata.flow_index = flow_index++;
                        ret = insert_(&swkey, &swappdata);
                        if (ret != SDK_RET_OK) {
                            return ret;
                        }

                        nflows++;
                        if (nflows >= count) {
                            return SDK_RET_OK;
                        }
#endif
                    }
                }
            }
        }
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

    void set_cfg_params(bool dual_stack, uint32_t num_tcp,
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
        return SDK_RET_OK;
    }
};

#endif // __APOLLO_SCALE_FLOW_TEST_HPP__

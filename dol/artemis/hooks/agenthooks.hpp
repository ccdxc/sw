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
#include "nic/utils/ftl/ftlv4.hpp"
#include "nic/utils/ftl/ftlv6.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "gen/p4gen/artemis/include/p4pd.h"

//#define DUMP_FLOW_SESSION_INFO 1

using sdk::table::ftlv6;
using sdk::table::ftlv4;
using sdk::table::sdk_table_api_params_t;
using sdk::table::sdk_table_api_stats_t;
using sdk::table::sdk_table_stats_t;
using sdk::table::sdk_table_factory_params_t;

namespace pt = boost::property_tree;

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
    return str;
}

static char *
flow_appdata2str(void *appdata) {
    static char str[512];
    flow_appdata_t *d = (flow_appdata_t *)appdata;
    sprintf(str, "I:%d R:%d",
            d->session_index, d->flow_role);
    return str;
}

#ifdef DUMP_FLOW_SESSION_INFO
static void
dump_flow_entry(ftlv6_entry_t *entry, ipv6_addr_t v6_addr_sip,
                ipv6_addr_t v6_addr_dip) {
    static FILE *d_fp = fopen("/tmp/flow_log.log", "a+");
    char *src_ip_str = ipv6addr2str(v6_addr_sip);
    char *dst_ip_str = ipv6addr2str(v6_addr_dip);

    if (d_fp) {
        fprintf(d_fp, "vpc %u, proto %u, session_index %u, sip %s, dip %s, "
                "sport %u, dport %u, epoch %u, flow %u, ktype %u\n",
                entry->vpc_id, entry->proto, entry->session_index,
                src_ip_str, dst_ip_str, entry->sport, entry->dport,
                entry->epoch, entry->flow_role, entry->ktype);
        fflush(d_fp);
    }
}

static void
dump_flow_entry(ftlv4_entry_t *entry, ipv4_addr_t v4_addr_sip,
                ipv4_addr_t v4_addr_dip) {
    static FILE *d_fp = fopen("/tmp/flow_log.log", "a+");
    char *src_ip_str = ipv4addr2str(v4_addr_sip);
    char *dst_ip_str = ipv4addr2str(v4_addr_dip);

    if (d_fp) {
        fprintf(d_fp, "vpc %u, proto %u, session_index %u, sip %s, dip %s, "
                "sport %u, dport %u, epoch %u, role %u\n",
                entry->vpc_id, entry->proto, entry->session_index,
                src_ip_str, dst_ip_str, entry->sport, entry->dport,
                entry->epoch, entry->flow_role);
        fflush(d_fp);
    }
}

static void
dump_session_info(uint32_t vpc, session_actiondata_t *actiondata)
{
    static FILE *d_fp = fopen("/tmp/flow_log.log", "a+");

    if (d_fp) {
        fprintf(d_fp, "vpc %u, meter_idx %u, nh_idx %u, tx rewrite flags 0x%x, "
                "rx rewrite flags 0x%x\n\n", vpc - 1,
                actiondata->action_u.session_session_info.meter_idx,
                actiondata->action_u.session_session_info.nexthop_idx,
                actiondata->action_u.session_session_info.tx_rewrite_flags,
                actiondata->action_u.session_session_info.rx_rewrite_flags);
        fflush(d_fp);
    }
}

static void
dump_hash_value(uint32_t hash)
{
    static FILE *d_fp = fopen("/tmp/flow_log.log", "a+");

    if (d_fp) {
        fprintf(d_fp, "hash %x\n", hash);
        fflush(d_fp);
    }
}

#else
static void
dump_flow_entry(ftlv6_entry_t *entry, ipv6_addr_t v6_addr_sip,
                ipv6_addr_t v6_addr_dip)
{
    return;
}
static void
dump_flow_entry(ftlv4_entry_t *entry, ipv4_addr_t v4_addr_sip,
                ipv4_addr_t v4_addr_dip)
{
    return;
}
static void
dump_session_info(uint32_t vpc, session_actiondata_t *actiondata)
{
    return;
}
static void
dump_hash_value(uint32_t hash)
{
    return;
}
#endif

#define MAX_VPCS        64
#define MAX_LOCAL_EPS   512
#define MAX_REMOTE_EPS  512
#define MAX_EP_PAIRS_PER_VPC (MAX_LOCAL_EPS*MAX_REMOTE_EPS)

typedef struct mapping_s {
    union {
        struct {
            ip_addr_t local_ip;
            ip_addr_t public_ip;
            ip_addr_t service_ip;
            uint16_t  service_port; // Front service port
            uint16_t  lb_port; // Local backend port
        };
        struct {
            ip_addr_t remote_ip;
        };
    };
    ip_addr_t provider_ip;
    uint32_t nh_idx;
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
    // session related
    uint32_t num_nexthop_idx_per_vpc;
    uint32_t vpc_count;
    uint32_t num_meter_idx_per_vpc;
} cfg_params_t;


#define MAX_ROUTE_TBLS              128
#define MAX_SUBNETS                 64
#define MAX_ROUTES_PER_ROUTE_TBL    64
#define MAX_REMOTE_GW_HOSTS         4
typedef struct route_s {
    pds_route_t route;
    uint32_t    nh_id;
    union {
        uint32_t v4_host_count;
        uint32_t v6_host_count;
    };
    union {
        ipv4_addr_t v4_addr[MAX_REMOTE_GW_HOSTS];
        ipv6_addr_t v6_addr[MAX_REMOTE_GW_HOSTS];
    };
} route_t;

typedef struct vpc_local_gw_mapping_s {
    uint32_t vpc_id;
    uint32_t valid;
    mapping_t v4_local;
    mapping_t v6_local;
    route_t remotes[MAX_ROUTE_TBLS * MAX_ROUTES_PER_ROUTE_TBL];
    uint32_t remote_count;
} vpc_local_gw_mapping_t;

typedef struct route_db_s {
    uint32_t num_routes;
    pds_route_t routes[MAX_ROUTES_PER_ROUTE_TBL];
} route_db_t;

typedef struct subnet_db_s {
    uint32_t v4_routetbl_id;
    uint32_t v6_routetbl_id;
} subnet_db_t;

typedef enum session_type_e {
    VNET_TO_VNET = 0,
    VNET_TO_INTERNET_SLB
} session_type_t;

class flow_test {
private:
    ftlv6 *v6table;
    ftlv4 *v4table;
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
    ftlv6_entry_t v6entry;
    ftlv4_entry_t v4entry;
    vpc_ep_pair_t ep_pairs[MAX_EP_PAIRS_PER_VPC];
    cfg_params_t cfg_params;
    uint32_t meter_idx;
    bool with_hash;
    uint32_t remote_mapping_nexthop_idx;
    route_db_t routedb[2][MAX_ROUTE_TBLS];
    subnet_db_t subnetdb[MAX_VPCS][MAX_SUBNETS];
    vpc_local_gw_mapping_t local_gw_mapping[MAX_LOCAL_EPS];
    uint32_t v6_route_table_sid;
    uint32_t v4_route_table_sid;

private:

    void parse_cfg_json_() {
        pt::ptree json_pt;
        char cfgfile[256];

        assert(getenv("CONFIG_PATH"));
        snprintf(cfgfile, sizeof(cfgfile), "%s/gen/dol_agentcfg.json",
                 getenv("CONFIG_PATH"));

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
                } else if (kind == "flow") {
                    cfg_params.num_tcp = std::stol(obj.second.get<std::string>("num_tcp"));
                    cfg_params.num_udp = std::stol(obj.second.get<std::string>("num_udp"));
                    cfg_params.num_icmp = std::stol(obj.second.get<std::string>("num_icmp"));
                    cfg_params.sport_lo = std::stol(obj.second.get<std::string>("sport_lo"));
                    cfg_params.sport_hi = std::stol(obj.second.get<std::string>("sport_hi"));
                    cfg_params.dport_lo = std::stol(obj.second.get<std::string>("dport_lo"));
                    cfg_params.dport_hi = std::stol(obj.second.get<std::string>("dport_hi"));
                } else if (kind == "session") {
                    cfg_params.num_nexthop_idx_per_vpc = std::stol(obj.second.get<std::string>("num_nh_per_vpc"));
                    cfg_params.vpc_count = std::stol(obj.second.get<std::string>("vpc_count"));
                    cfg_params.num_meter_idx_per_vpc = cfg_params.num_nexthop_idx_per_vpc; //TODO
                    // Remote Mapping next hop starts immediately after the vpc reserved ones
                    remote_mapping_nexthop_idx = cfg_params.num_nexthop_idx_per_vpc * cfg_params.vpc_count;
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
        SDK_TRACE_DEBUG("- num_nh_per_vpc : %d", cfg_params.num_nexthop_idx_per_vpc);
        SDK_TRACE_DEBUG("- vpc_count : %d", cfg_params.vpc_count);
    }


    sdk_ret_t insert_(ftlv6_entry_t *v6entry) {
        sdk_ret_t rv;

        memset(&params, 0, sizeof(params));
        params.entry = v6entry;
        if (with_hash) {
            params.hash_valid = true;
            params.hash_32b = hash++;
        }
        rv = v6table->insert(&params);
        dump_hash_value(params.hash_32b);
        return rv;
    }

    sdk_ret_t insert_(ftlv4_entry_t *v4entry) {
        sdk_ret_t rv;

        memset(&params, 0, sizeof(params));
        params.entry = v4entry;
        if (with_hash) {
            params.hash_valid = true;
            params.hash_32b = hash++;
        }
        rv = v4table->insert(&params);
        dump_hash_value(params.hash_32b);
        return rv;
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
        factory_params.entry_trace_en = false;
        v6table = ftlv6::factory(&factory_params);
        assert(v6table);

        memset(&factory_params, 0, sizeof(factory_params));
        factory_params.table_id = P4TBL_ID_IPV4_FLOW;
        factory_params.num_hints = 2;
        factory_params.max_recircs = 8;
        factory_params.key2str = NULL;
        factory_params.appdata2str = NULL;
        factory_params.entry_trace_en = false;
        v4table = ftlv4::factory(&factory_params);
        assert(v4table);

        memset(epdb, 0, sizeof(epdb));
        memset(&cfg_params, 0, sizeof(cfg_params_t));
        session_index = 1;
        epoch = 1;
        hash = 0;
        with_hash = w;
        v6_route_table_sid = 0;
        v4_route_table_sid = 0;
        memset(routedb, 0, sizeof(routedb));
        memset(subnetdb, 0 , sizeof(subnetdb));
    }

    void read_config() {
        parse_cfg_json_();
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
            epdb[vpc_id].v4_locals[epdb[vpc_id].v4_lcount].local_ip = ip_addr;
            if (local_spec->public_ip_valid) {
                epdb[vpc_id].v4_locals[epdb[vpc_id].v4_lcount].public_ip =
                                                        local_spec->public_ip;
            }
            if (local_spec->provider_ip_valid) {
                epdb[vpc_id].v4_locals[epdb[vpc_id].v4_lcount].provider_ip =
                                                        local_spec->provider_ip;
            }
            // Even Local one consume one next hop for local to local remote.
            // Local entries are added in remote table
            epdb[vpc_id].v4_locals[epdb[vpc_id].v4_lcount].nh_idx = ++remote_mapping_nexthop_idx;
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
            // Even Local one consume one next hop for local to local remote.
            // Local entries are added in remote table
            epdb[vpc_id].v6_locals[epdb[vpc_id].v6_lcount].nh_idx = ++remote_mapping_nexthop_idx;
            epdb[vpc_id].v6_lcount++;
        }
        //fprintf(stderr, "Adding Local EP Vpc %d, IP %s, PIP %s\n",
        //        vpc_id, ipaddr2str(&ip_addr), ipaddr2str(&local_spec->provider_ip));
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
            epdb[vpc_id].v4_remotes[epdb[vpc_id].v4_rcount].remote_ip = ip_addr;
            if (remote_spec->provider_ip_valid) {
                epdb[vpc_id].v4_remotes[epdb[vpc_id].v4_rcount].provider_ip =
                                                    remote_spec->provider_ip;
            }
            epdb[vpc_id].v4_remotes[epdb[vpc_id].v4_rcount].nh_idx = ++remote_mapping_nexthop_idx;
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
            epdb[vpc_id].v6_remotes[epdb[vpc_id].v6_rcount].nh_idx = ++remote_mapping_nexthop_idx;
            epdb[vpc_id].v6_rcount++;
        }
        //fprintf(stderr, "Adding Remote EP Vpc %d, IP %s, PIP %s\n",
        //        vpc_id, ipaddr2str(&ip_addr), ipaddr2str(&remote_spec->provider_ip));
    }

    void add_route_table(pds_route_table_spec_t *route_spec) {
        uint32_t rt_dbid = route_spec->af == IP_AF_IPV4;
        uint32_t tblid = 0;

        if (rt_dbid == 1) {
            if (!v4_route_table_sid)
                v4_route_table_sid = route_spec->key.id;
            else
                tblid = route_spec->key.id - v4_route_table_sid;
        } else if(rt_dbid == 0) {
            if (!v6_route_table_sid)
                v6_route_table_sid = route_spec->key.id;
            else
                 tblid = route_spec->key.id - v6_route_table_sid;
        }
        //fprintf(stderr, "Route tblid %u:%u type %u\n", route_spec->key.id, tblid, rt_dbid);
        assert(tblid < MAX_ROUTE_TBLS);
        assert(route_spec->num_routes < MAX_ROUTES_PER_ROUTE_TBL);
        for (uint32_t i = 0; i < route_spec->num_routes; i++) {
            memcpy(&routedb[rt_dbid][tblid].routes[i],
                   &route_spec->routes[i], sizeof(pds_route_t));
        }
        routedb[rt_dbid][tblid].num_routes = route_spec->num_routes;
    }

    void add_subnet(pds_subnet_spec_t *subnet_spec) {
        assert(subnet_spec->key.id < MAX_SUBNETS);
        assert(subnet_spec->vpc.id < MAX_VPCS);
        //fprintf(stderr, "Subnet tblid %u, rtid %u:%u\n", subnet_spec->key.id,
        //        subnet_spec->v4_route_table.id, subnet_spec->v6_route_table.id);
        subnetdb[subnet_spec->vpc.id][subnet_spec->key.id].v4_routetbl_id = subnet_spec->v4_route_table.id + 1;
        subnetdb[subnet_spec->vpc.id][subnet_spec->key.id].v6_routetbl_id = subnet_spec->v6_route_table.id + 1;
    }

    void add_svc_mapping(pds_svc_mapping_spec_t *svc_spec) {
        uint32_t vpc = svc_spec->vpc.id;
        uint32_t af = svc_spec->key.vip.af;
        uint32_t lid;

        if (af == IP_AF_IPV4) {
            for (lid = 0; lid < epdb[vpc].v4_lcount; lid++) {
                if (epdb[vpc].v4_locals[lid].provider_ip.addr.v4_addr ==
                    svc_spec->backend_provider_ip.addr.v4_addr) {
                    epdb[vpc].v4_locals[lid].service_ip = svc_spec->key.vip;
                    epdb[vpc].v4_locals[lid].service_port = svc_spec->key.svc_port;
                    epdb[vpc].v4_locals[lid].lb_port = svc_spec->svc_port;
                    break;
                }
            }
            assert(lid < epdb[vpc].v4_lcount);
        } else {
            for (lid = 0; lid < epdb[vpc].v6_lcount; lid++) {
                if ((epdb[vpc].v6_locals[lid].provider_ip.addr.v6_addr.addr64[0] ==
                    svc_spec->backend_provider_ip.addr.v6_addr.addr64[0]) && (
                    epdb[vpc].v6_locals[lid].provider_ip.addr.v6_addr.addr64[1] ==
                    svc_spec->backend_provider_ip.addr.v6_addr.addr64[1])) {
                    epdb[vpc].v6_locals[lid].service_ip = svc_spec->key.vip;
                    epdb[vpc].v4_locals[lid].service_port = svc_spec->key.svc_port;
                    epdb[vpc].v4_locals[lid].lb_port = svc_spec->svc_port;
                    break;
                }
                assert(lid < epdb[vpc].v6_lcount);
            }
        }
    }

    uint32_t generate_local_gw_mapping(uint32_t vpc, bool v4) {
        pds_route_t *routes;
        uint32_t nroutes;
        uint32_t rt_dbid = v4 ? 1 : 0;
        uint32_t lid;

        memset(local_gw_mapping, 0, sizeof(local_gw_mapping));
        // fprintf(stderr, "%s in vpc %d\n", __func__, vpc);
        // We will configure the first and last ip of each prefix
        assert(epdb[vpc].v4_lcount < MAX_LOCAL_EPS);
        for (lid = 0; lid < epdb[vpc].v4_lcount; lid++) {
            local_gw_mapping[lid].v4_local = epdb[vpc].v4_locals[lid];
            for (int sid = 0; sid < MAX_SUBNETS; sid++) {
                if (subnetdb[vpc][sid].v4_routetbl_id) {
                    uint32_t tblid = subnetdb[vpc][sid].v4_routetbl_id - 1 - v4_route_table_sid;
                    uint32_t rid = 0, rc;

                    routes = routedb[rt_dbid][tblid].routes;
                    nroutes = routedb[rt_dbid][tblid].num_routes;
                    rc = local_gw_mapping[lid].remote_count;

                    for (rid = 0; rid < nroutes; rid++, rc++) {
                        assert(rc < (sizeof(local_gw_mapping[lid].remotes) / sizeof(route_t)));
                        local_gw_mapping[lid].remotes[rc].route = routes[rid];
                        // Create flows for the last addresses, we are saving the first one also
                        local_gw_mapping[lid].remotes[rc].v4_addr[0] = routes[rid].prefix.addr.addr.v4_addr;
                        local_gw_mapping[lid].remotes[rc].v4_host_count++;
                        if (routes[rid].prefix.len != 32) {
                            local_gw_mapping[lid].remotes[rc].v4_addr[1] =
                                routes[rid].prefix.addr.addr.v4_addr | ((1 << (32 - routes[rid].prefix.len)) - 1);
                            local_gw_mapping[lid].remotes[rc].v4_host_count++;
                        } else {
                            local_gw_mapping[lid].remotes[rc].v4_addr[1]  = local_gw_mapping[lid].remotes[rc].v4_addr[0];
                        }
                       /*  fprintf(stderr, "local gw mapping vpc %u, routes %s, count %u addr %x:%x\n", vpc,
                                ippfx2str(&local_gw_mapping[lid].remotes[rc].route.prefix),
                                local_gw_mapping[lid].remotes[rc].v4_host_count,
                                local_gw_mapping[lid].remotes[rc].v4_addr[0],
                                local_gw_mapping[lid].remotes[rc].v4_addr[1]); */
                    }
                    local_gw_mapping[lid].remote_count += rid;
                }
            }
        }
        //fprintf(stderr, "%s out vpc %d\n", __func__, vpc);
        return lid;
    }

    void generate_ep_pairs(uint32_t vpc, bool dual_stack) {
        if (dual_stack) {
            assert(epdb[vpc].v6_lcount == epdb[vpc].v4_lcount);
            assert(epdb[vpc].v6_rcount == epdb[vpc].v4_rcount);
        }
        uint32_t pid = 0;
        memset(ep_pairs, 0, sizeof(ep_pairs));
        if (epdb[vpc].valid == 0) {
            return;
        }
        for (uint32_t lid = 0; lid < epdb[vpc].v4_lcount; lid++) {
            for (uint32_t rid = 0; rid < epdb[vpc].v4_rcount; rid++) {
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

    sdk_ret_t create_session_info(uint32_t vpc, uint32_t nexthop_idx, session_type_t type) {
        session_actiondata_t actiondata;
        p4pd_error_t p4pd_ret;
        uint32_t tableid = P4TBL_ID_SESSION;
        static uint32_t cur_vpc = 0;

        // reset meter_idx to corresponding start value for the VPC
        if (cur_vpc != vpc) {
            cur_vpc = vpc;
            meter_idx = (vpc - 1) * cfg_params.num_meter_idx_per_vpc + 1;
        }
        memset(&actiondata, 0, sizeof(session_actiondata_t));
        actiondata.action_id = SESSION_SESSION_INFO_ID;
        actiondata.action_u.session_session_info.nexthop_idx = nexthop_idx;
        if (type == VNET_TO_VNET) {
            // do DMACi rewrite and encap in the Tx direction
            actiondata.action_u.session_session_info.tx_rewrite_flags = 0x21;
            // No rewrites in the RX direction.
            actiondata.action_u.session_session_info.rx_rewrite_flags = 0x11;
        } else if (type == VNET_TO_INTERNET_SLB) {
            actiondata.action_u.session_session_info.tx_rewrite_flags = 0x45;
            actiondata.action_u.session_session_info.rx_rewrite_flags = 0x61;
        }
        actiondata.action_u.session_session_info.meter_idx = meter_idx++;
        if (meter_idx == vpc * cfg_params.num_meter_idx_per_vpc) {
            meter_idx = (vpc-1) * cfg_params.num_meter_idx_per_vpc + 1;
        }
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

    sdk_ret_t create_flow(uint32_t vpc, ipv6_addr_t iflow_sip,
                             ipv6_addr_t iflow_dip, uint8_t proto,
                             uint16_t iflow_sport, uint16_t iflow_dport) {
        memset(&v6entry, 0, sizeof(ftlv6_entry_t));
        // Common DATA fields
        v6entry.session_index = session_index;
        v6entry.epoch = 0xFF;
        // Common KEY fields
        v6entry.ktype = 2;
        v6entry.vpc_id = vpc;
        v6entry.proto = proto;
        // Create IFLOW
        v6entry.sport = iflow_sport;
        v6entry.dport = iflow_dport;
        sdk::lib::memrev(v6entry.src, iflow_sip.addr8, sizeof(ipv6_addr_t));
        sdk::lib::memrev(v6entry.dst, iflow_dip.addr8, sizeof(ipv6_addr_t));
        auto ret = insert_(&v6entry);
        SDK_ASSERT(ret == SDK_RET_OK || ret == SDK_RET_ENTRY_EXISTS);
        dump_flow_entry(&v6entry, iflow_sip, iflow_dip);
        // Create RFLOW
        v6entry.sport = iflow_dport;
        v6entry.dport = iflow_sport;
        sdk::lib::memrev(v6entry.src, iflow_dip.addr8, sizeof(ipv6_addr_t));
        sdk::lib::memrev(v6entry.dst, iflow_sip.addr8, sizeof(ipv6_addr_t));
        ret = insert_(&v6entry);
        SDK_ASSERT(ret == SDK_RET_OK || ret == SDK_RET_ENTRY_EXISTS);
        dump_flow_entry(&v6entry, iflow_dip, iflow_sip);

        return SDK_RET_OK;
    }

    sdk_ret_t create_flow(uint32_t vpc, uint8_t proto,
                             ipv4_addr_t iflow_sip, ipv4_addr_t iflow_dip,
                             uint16_t iflow_sport, uint16_t iflow_dport,
                             ipv4_addr_t rflow_sip, ipv4_addr_t rflow_dip,
                             uint16_t rflow_sport, uint16_t rflow_dport) {
        memset(&v4entry, 0, sizeof(ftlv4_entry_t));
        // Common DATA fields
        v4entry.session_index = session_index;
        v4entry.epoch = 0xFF;
        // Common KEY fields
        v4entry.vpc_id = vpc;
        v4entry.proto = proto;
        // Create IFLOW
        v4entry.sport = iflow_sport;
        v4entry.dport = iflow_dport;
        v4entry.src = iflow_sip;
        v4entry.dst = iflow_dip;
        auto ret = insert_(&v4entry);
        SDK_ASSERT(ret == SDK_RET_OK || ret == SDK_RET_ENTRY_EXISTS);
        dump_flow_entry(&v4entry, iflow_sip, iflow_dip);
        // Create RFLOW
        v4entry.sport = rflow_sport;
        v4entry.dport = rflow_dport;
        v4entry.src = rflow_sip;
        v4entry.dst = rflow_dip;
        ret = insert_(&v4entry);
        SDK_ASSERT(ret == SDK_RET_OK || ret == SDK_RET_ENTRY_EXISTS);
        dump_flow_entry(&v4entry, iflow_dip, iflow_sip);

        return SDK_RET_OK;
    }

    sdk_ret_t create_vnet_flows_one_proto_(uint32_t count, uint8_t proto, bool dual_stack) {
        uint16_t local_port = 0, remote_port = 0;
        uint32_t i = 0;
        uint16_t fwd_sport = 0, fwd_dport = 0;
        uint16_t rev_sport = 0, rev_dport = 0;
        uint32_t nflows = 0;
        sdk_ret_t ret;
        ip_addr_t ip_addr;

        for (uint32_t vpc = 1; vpc <= MAX_VPCS; vpc++) {
            generate_ep_pairs(vpc, dual_stack);
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

                        memset(&ip_addr, 0, sizeof(ip_addr));
                        // create V4 Flows
                        // vnet in/out with vxlan encap
                        ret = create_flow(
                            vpc, proto,
                            ep_pairs[i].v4_local.local_ip.addr.v4_addr,
                            ep_pairs[i].v4_remote.remote_ip.addr.v4_addr,
                            fwd_sport, fwd_dport,
                            ep_pairs[i].v4_remote.remote_ip.addr.v4_addr,
                            ep_pairs[i].v4_local.local_ip.addr.v4_addr,
                            fwd_dport, fwd_sport);
                        if (ret != SDK_RET_OK) {
                            return ret;
                        }
                        // create V4 session
                        ret = create_session_info(vpc,  ep_pairs[i].v4_remote.nh_idx, VNET_TO_VNET);
                        if (ret != SDK_RET_OK) {
                            return ret;
                        }
                        session_index++;

                        if (dual_stack) {
                            memset(&ip_addr, 0, sizeof(ip_addr));
                            // create V6 Flows
                            // vnet in/out with vxlan encap
                            ret = create_flow(
                                vpc, ep_pairs[i].v6_local.local_ip.addr.v6_addr,
                                ep_pairs[i].v6_remote.remote_ip.addr.v6_addr,
                                proto, fwd_sport, fwd_dport);
                            if (ret != SDK_RET_OK) {
                                return ret;
                            }
                            // create V6 session
                            ret = create_session_info(vpc, ep_pairs[i].v6_remote.nh_idx, VNET_TO_VNET);
                            if (ret != SDK_RET_OK) {
                                return ret;
                            }
                            session_index++;
                        }

                        nflows+=2;
                        if (nflows >= count) {
                            return SDK_RET_OK;
                        }
                    }
                }
            }
        }
        return SDK_RET_OK;
    }

    sdk_ret_t create_vnet_vip_flows_one_proto_(uint32_t count, uint8_t proto, bool v4) {
        uint16_t local_port = 0, remote_port = 0;
        uint32_t i = 0;
        uint16_t fwd_sport = 0, fwd_dport = 0;
        uint16_t rev_sport = 0, rev_dport = 0;
        uint32_t nflows = 0;
        sdk_ret_t ret;
        ip_addr_t ip_addr;

        for (uint32_t vpc = 1; vpc <= MAX_VPCS; vpc++) {
            generate_local_gw_mapping(vpc, v4);
            for (i = 0; i < MAX_LOCAL_EPS ; i++) {
                if (local_gw_mapping[i].remote_count == 0) {
                    break;
                }
                local_port = local_gw_mapping[i].v4_local.lb_port;
                remote_port = cfg_params.dport_lo;
                if (proto == IP_PROTO_ICMP) {
                    fwd_sport = rev_sport = local_port;
                    fwd_dport = rev_dport = remote_port;
                } else {
                    fwd_sport = rev_dport = local_port;
                    fwd_dport = rev_sport = remote_port;
                }

                memset(&ip_addr, 0, sizeof(ip_addr));
                if (v4) {
                    for (uint32_t rc = 0; rc < local_gw_mapping[i].remote_count; rc++) {
                        // create V4 Flows
                        ret = create_flow(
                            vpc, proto,
                            local_gw_mapping[i].v4_local.local_ip.addr.v4_addr,
                            local_gw_mapping[i].remotes[rc].v4_addr[1],
                            fwd_sport, fwd_dport,
                            local_gw_mapping[i].remotes[rc].v4_addr[1],
                            local_gw_mapping[i].v4_local.service_ip.addr.v4_addr,
                            fwd_dport, local_gw_mapping[i].v4_local.service_port);
                        if (ret != SDK_RET_OK) {
                            return ret;
                        }
                        // create V4 session
                        ret = create_session_info(
                            vpc, local_gw_mapping[i].remotes[rc].route.nh,
                            VNET_TO_INTERNET_SLB);
                        if (ret != SDK_RET_OK) {
                            return ret;
                        }
                        session_index++;
                        nflows+=2;
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

    sdk_ret_t create_flows_all_protos_(bool dual_stack) {
        if (cfg_params.num_tcp) {
            auto ret = create_vnet_flows_one_proto_(cfg_params.num_tcp, IP_PROTO_TCP, dual_stack);
            if (ret != SDK_RET_OK) {
                return ret;
            }
            ret = create_vnet_vip_flows_one_proto_(cfg_params.num_tcp, IP_PROTO_TCP, true);
            if (ret != SDK_RET_OK) {
                return ret;
            }
        }

        if (cfg_params.num_udp) {
            auto ret = create_vnet_flows_one_proto_(cfg_params.num_udp, IP_PROTO_UDP, dual_stack);
            if (ret != SDK_RET_OK) {
                return ret;
            }
            ret = create_vnet_vip_flows_one_proto_(cfg_params.num_udp, IP_PROTO_UDP, true);
            if (ret != SDK_RET_OK) {
                return ret;
            }

        }

        if (cfg_params.num_icmp) {
            auto ret = create_vnet_flows_one_proto_(cfg_params.num_icmp, IP_PROTO_ICMP, dual_stack);
            if (ret != SDK_RET_OK) {
                return ret;
            }
            ret = create_vnet_vip_flows_one_proto_(cfg_params.num_icmp, IP_PROTO_ICMP, true);
            if (ret != SDK_RET_OK) {
                return ret;
            }
        }
        return SDK_RET_OK;
    }

    sdk_ret_t create_flows() {
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
};

#endif // __APOLLO_SCALE_FLOW_TEST_HPP__

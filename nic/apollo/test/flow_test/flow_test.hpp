//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __APOLLO_SCALE_FLOW_TEST_HPP__
#define __APOLLO_SCALE_FLOW_TEST_HPP__
#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "include/sdk/base.hpp"
#include "include/sdk/table.hpp"
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/sdk/include/sdk/ip.hpp"

#include "gen/p4gen/apollo/include/p4pd.h"

using sdk::table::mem_hash;
using sdk::table::sdk_table_api_params_t;
using sdk::table::sdk_table_api_stats_t;
using sdk::table::sdk_table_stats_t;
using sdk::table::sdk_table_factory_params_t;

#define FLOW_TEST_CHECK_RETURN(_exp, _ret) if (!(_exp)) return (_ret)

static char *
flow_key2str(void *key) {
    static char str[256];
    flow_swkey_t *k = (flow_swkey_t *)key;
    char srcstr[INET_ADDRSTRLEN];
    char dststr[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, k->key_metadata_src, srcstr, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, k->key_metadata_dst, dststr, INET_ADDRSTRLEN);
    sprintf(str, "Type:%d Src:%s Dst:%s Dport:%d Sport:%d Proto:%d Tag:%d",
            k->key_metadata_ktype, srcstr, dststr,
            k->key_metadata_dport, k->key_metadata_sport,
            k->key_metadata_proto, k->vnic_metadata_local_vnic_tag);
    return str;
}

static char *
flow_appdata2str(void *appdata) {
    static char str[512];
    flow_appdata_t *d = (flow_appdata_t *)appdata;
    sprintf(str, "flow_index:%d flow_role:%d",
            d->flow_index, d->flow_role);
    return str;
}

#define MAX_VCNS        1024+1
#define MAX_LOCAL_EPS   32
#define MAX_REMOTE_EPS  1024
#define MAX_EP_PAIRS_PER_VCN (MAX_LOCAL_EPS*MAX_REMOTE_EPS)

typedef struct vcn_epdb_s {
    uint32_t vcn_id;
    uint32_t valid;
    uint32_t lcount;
    uint32_t rcount;
    uint32_t lips[MAX_LOCAL_EPS];
    uint32_t rips[MAX_REMOTE_EPS];
} vcn_epdb_t;

typedef struct vcn_ep_pair_s {
    uint32_t vcn_id;
    uint32_t lip;
    uint32_t rip;
    uint32_t valid;
} vcn_ep_pair_t;

vcn_ep_pair_t g_ep_pairs[MAX_EP_PAIRS_PER_VCN];

class flow_test {
private:
    mem_hash *table;
    vcn_epdb_t epdb[MAX_VCNS];
    uint32_t nflows;
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

private:
    sdk_ret_t insert_(flow_swkey_t *key,
                      flow_appdata_t *appdata) {
        memset(&params, 0, sizeof(params));
        params.key = key;
        params.appdata = appdata;
        //params.hash_valid = true;
        //params.hash_32b = hash++;
        return table->insert(&params);
    }

    sdk_ret_t remove_(flow_swkey_t *key) {
        sdk_table_api_params_t params = { 0 };
        params.key = key;
        return table->remove(&params);
    }
 
public:
    flow_test() {
        memset(&factory_params, 0, sizeof(factory_params));
        
        factory_params.table_id = P4TBL_ID_FLOW;
        factory_params.num_hints = 5;
        factory_params.max_recircs = 8;
        factory_params.key2str = flow_key2str;
        factory_params.appdata2str = flow_appdata2str;
        table = mem_hash::factory(&factory_params);
        assert(table);

        memset(epdb, 0, sizeof(epdb));
        flow_index = 0;
        hash = 0;
        nflows = 0;
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

    void add_local_ep(uint32_t vcn_id, uint32_t ipaddr) {
        assert(vcn_id && vcn_id < MAX_VCNS);
        assert(epdb[vcn_id].lcount < MAX_LOCAL_EPS);
        epdb[vcn_id].valid = 1;
        epdb[vcn_id].vcn_id = vcn_id;
        epdb[vcn_id].lips[epdb[vcn_id].lcount] = ipaddr;
        epdb[vcn_id].lcount++;
        printf("Adding Local EP: Vcn=%d IP=%#x\n", vcn_id, ipaddr);
    }

    void add_remote_ep(uint32_t vcn_id, uint32_t ipaddr) {
        assert(vcn_id && vcn_id < MAX_VCNS);
        assert(epdb[vcn_id].rcount < MAX_REMOTE_EPS);
        epdb[vcn_id].valid = 1;
        epdb[vcn_id].vcn_id = vcn_id;
        epdb[vcn_id].rips[epdb[vcn_id].rcount] = ipaddr;
        epdb[vcn_id].rcount++;
        printf("Adding Remote EP: Vcn=%d IP=%#x\n", vcn_id, ipaddr);
    }

    void generate_ep_pairs(uint32_t vcn) {
        uint32_t pid = 0;
        memset(g_ep_pairs, 0, sizeof(g_ep_pairs));
        if (epdb[vcn].valid == 0) {
            return;
        }
        for (uint32_t lid = 0; lid < epdb[vcn].lcount; lid++) {
            for (uint32_t rid = 0; rid < epdb[vcn].rcount; rid++) {
                g_ep_pairs[pid].vcn_id = vcn;
                g_ep_pairs[pid].lip = epdb[vcn].lips[lid];
                g_ep_pairs[pid].rip = epdb[vcn].rips[rid];
                g_ep_pairs[pid].valid = 1;
                printf("Appending EP pair: Vcn=%d LIP=%#x RIP=%#x\n", vcn,
                       epdb[vcn].lips[lid], epdb[vcn].rips[rid]);
                pid++;
            }
        }
    }

    void generate_dummy_epdb() {
        for (uint32_t vcn = 1; vcn < MAX_VCNS; vcn++) {
            epdb[vcn].vcn_id = vcn;
            for (uint32_t lid = 0; lid < MAX_LOCAL_EPS; lid++) {
                add_local_ep(vcn, 0x0a000001 + lid);
                for (uint32_t rid = 0; rid < MAX_REMOTE_EPS; rid++) {
                    add_remote_ep(vcn, 0x1400001 + rid);
                }
            }
        }
        return;
    }

    sdk_ret_t create_flows(uint32_t count, uint8_t proto,
                           uint16_t spbase, uint16_t dpbase) {
        uint16_t local_port = 0, remote_port = 0;
        sdk_ret_t ret = SDK_RET_OK;
        uint32_t i = 0;
        uint16_t fwd_sport = 0, fwd_dport = 0;
        uint16_t rev_sport = 0, rev_dport = 0;

        set_port_bases(spbase, dpbase);
        memset(&swkey, 0, sizeof(swkey));
        memset(&swappdata, 0, sizeof(swappdata));
        swkey.key_metadata_ktype = 1;
        for (uint32_t vcn = 1; vcn < MAX_VCNS; vcn++) {
            generate_ep_pairs(vcn);
            for (i = 0; i < MAX_EP_PAIRS_PER_VCN ; i+=2) {
                local_port = alloc_sport(proto);
                remote_port = alloc_dport(proto);
                if (g_ep_pairs[i].valid == 0) {
                    break;
                }

                if (proto == 1) {
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
                memcpy(&(swkey.key_metadata_src), &g_ep_pairs[i].lip, sizeof(uint32_t));
                memcpy(&(swkey.key_metadata_dst), &g_ep_pairs[i].rip, sizeof(uint32_t));
                swappdata.flow_index = flow_index++;
                ret = insert_(&swkey, &swappdata);
                if (ret != SDK_RET_OK) {
                    return ret;
                }

                nflows++;
                if (nflows >= count) {
                    return SDK_RET_OK;
                }
                
                // Remote to Local Flow
                swkey.vnic_metadata_local_vnic_tag = vcn - 1;
                swkey.key_metadata_sport = rev_sport;
                swkey.key_metadata_dport = rev_dport;
                swkey.key_metadata_proto = proto;
                memcpy(&(swkey.key_metadata_src), &g_ep_pairs[i].rip, sizeof(uint32_t));
                memcpy(&(swkey.key_metadata_dst), &g_ep_pairs[i].lip, sizeof(uint32_t));
                swappdata.flow_index = flow_index++;
                ret = insert_(&swkey, &swappdata);
                if (ret != SDK_RET_OK) {
                    return ret;
                }

                nflows++;
                if (nflows >= count) {
                    return SDK_RET_OK;
                }
            }
        }
        return SDK_RET_OK;
    }
};

#endif // __APOLLO_SCALE_FLOW_TEST_HPP__

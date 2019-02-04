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

typedef struct flow_endpoint_s {
    uint32_t vcn_id;
    uint32_t ipaddr;
} flow_endpoint_t;

#define MAX_EPS 64*1024

class flow_test {
private:
    mem_hash *table;
    uint32_t num_local_eps;
    flow_endpoint_t local_eps[MAX_EPS];
    uint32_t num_remote_eps;
    flow_endpoint_t remote_eps[MAX_EPS];
    uint32_t local_ep_iter;
    uint32_t remote_ep_iter;
    uint32_t flow_index;
    uint32_t hash;
    uint16_t port;
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

        num_local_eps = 0;
        num_remote_eps = 0;
        local_ep_iter = 0;
        remote_ep_iter = 0;
        flow_index = 0;
        hash = 0;
        port = 1;
    }

    ~flow_test() {
        mem_hash::destroy(table);
    }

    void add_local_ep(uint32_t vcn_id, uint32_t ipaddr) {
        if (num_local_eps >= MAX_EPS) {
            return;
        }
        local_eps[num_local_eps].vcn_id = vcn_id;
        local_eps[num_local_eps].ipaddr = ipaddr;
        num_local_eps++;
    }

    void add_remote_ep(uint32_t vcn_id, uint32_t ipaddr) {
        if (num_remote_eps >= MAX_EPS) {
            return;
        }
        remote_eps[num_remote_eps].vcn_id = vcn_id;
        remote_eps[num_remote_eps].ipaddr = ipaddr;
        num_remote_eps++;
    }

    void get_next_ep_pair(uint32_t &vcn_id,
                          uint32_t &local_ipaddr,
                          uint32_t &remote_ipaddr) {
        vcn_id = local_eps[local_ep_iter].vcn_id;
        local_ipaddr = local_eps[local_ep_iter].ipaddr;
        remote_ipaddr = remote_eps[remote_ep_iter].ipaddr;
        local_ep_iter = (local_ep_iter + 1) % num_local_eps;
        remote_ep_iter = (remote_ep_iter + 1) % num_remote_eps;
    }

    void generate_dummy_ep_pairs() {
        for (uint32_t i = 0; i < 1024; i++) {
            local_eps[i].ipaddr = 0x0a000001 + i;
            local_eps[i].vcn_id = i;
            remote_eps[i].ipaddr = 0x1400001 + i;
            remote_eps[i].vcn_id = i;
        }
        num_local_eps = 1024;
        num_remote_eps = 1024;
        return;
    }

    sdk_ret_t create_flows(uint32_t count, uint8_t proto) {
        uint32_t vcn_id = 0, localip = 0, remoteip = 0;
        uint32_t local_port = 0, remote_port = 0;
        sdk_ret_t ret = SDK_RET_OK;
        uint32_t i = 0;

        memset(&swkey, 0, sizeof(swkey));
        memset(&swappdata, 0, sizeof(swappdata));

        swkey.key_metadata_ktype = 1;
        for (i = 0; i < count; i+=2) {
            local_port = port++;
            remote_port = port++;
            get_next_ep_pair(vcn_id, localip, remoteip);

            // Local to Remote Flow
            swkey.vnic_metadata_local_vnic_tag = vcn_id;
            swkey.key_metadata_sport = local_port;
            swkey.key_metadata_dport = remote_port;
            swkey.key_metadata_proto = proto;
            memcpy(&(swkey.key_metadata_src), &localip, sizeof(localip));
            memcpy(&(swkey.key_metadata_dst), &remoteip, sizeof(remoteip));
            swappdata.flow_index = flow_index++;
            ret = insert_(&swkey, &swappdata);
            if (ret != SDK_RET_OK) {
                return ret;
            }
            
            // Remote to Local Flow
            swkey.vnic_metadata_local_vnic_tag = vcn_id;
            swkey.key_metadata_sport = remote_port;;
            swkey.key_metadata_dport = local_port;
            swkey.key_metadata_proto = proto;
            memcpy(&(swkey.key_metadata_src), &remoteip, sizeof(remoteip));
            memcpy(&(swkey.key_metadata_dst), &localip, sizeof(localip));
            swappdata.flow_index = flow_index++;
            ret = insert_(&swkey, &swappdata);
            if (ret != SDK_RET_OK) {
                return ret;
            }
        }
        return SDK_RET_OK;
    }
};

#endif // __APOLLO_SCALE_FLOW_TEST_HPP__

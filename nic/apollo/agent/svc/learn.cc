//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// server side implementation for Learn service
///
//----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/learn/ep_mac_entry.hpp"
#include "nic/apollo/learn/ep_ip_entry.hpp"
#include "nic/apollo/learn/ep_mac_state.hpp"
#include "nic/apollo/learn/ep_ip_state.hpp"
#include "nic/apollo/learn/ep_utils.hpp"
#include "nic/apollo/learn/learn.hpp"

using learn::ep_mac_key_t;
using learn::ep_ip_key_t;
using learn::ep_state_t;

static bool
ep_mac_walk_ip_list_cb (void *entry, void *user_data)
{
    pds::LearnMACEntry *proto_entry = (pds::LearnMACEntry *)user_data;
    ep_ip_entry *ip_entry = (ep_ip_entry *)entry;

    pds::LearnIPKey *ip_key = proto_entry->add_ipinfo();
    ipaddr_api_spec_to_proto_spec(
                ip_key->mutable_ipaddr(), &ip_entry->key()->ip_addr);
    ip_key->set_vpcid(ip_entry->key()->vpc.id, PDS_MAX_KEY_LEN);
    return false; // returning false to continue iterating
}

static void
ep_mac_entry_to_proto (ep_mac_entry *mac_entry, pds::LearnMACEntry *proto_entry)
{
    pds_obj_key_t vnic_key;

    vnic_key = learn::ep_vnic_key(mac_entry);
    proto_entry->set_state(pds_learn_state_to_proto(mac_entry->state()));
    proto_entry->set_vnicid(vnic_key.id, PDS_MAX_KEY_LEN);
    proto_entry->mutable_key()->set_macaddr(
                                MAC_TO_UINT64(mac_entry->key()->mac_addr));
    proto_entry->mutable_key()->set_subnetid(mac_entry->key()->subnet.id,
                                             PDS_MAX_KEY_LEN);
}

static sdk_ret_t
ep_mac_get (ep_mac_key_t *key, pds::LearnMACGetResponse *proto_rsp)
{
    ep_mac_entry *mac_entry;

    mac_entry = (ep_mac_entry *)learn_db()->ep_mac_db()->find(key);
    if (!mac_entry) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    pds::LearnMACEntry *proto_entry = proto_rsp->add_response();
    ep_mac_entry_to_proto(mac_entry, proto_entry);
    mac_entry->walk_ip_list(ep_mac_walk_ip_list_cb, proto_entry);
    return SDK_RET_OK;
}

static bool
ep_mac_entry_walk_cb (void *entry, void *rsp)
{
    pds::LearnMACGetResponse *proto_rsp = (pds::LearnMACGetResponse *)rsp;
    ep_mac_entry *mac_entry = (ep_mac_entry *)entry;

    pds::LearnMACEntry *proto_entry = proto_rsp->add_response();
    ep_mac_entry_to_proto(mac_entry, proto_entry);
    mac_entry->walk_ip_list(ep_mac_walk_ip_list_cb, proto_entry);
    return false;
}

static sdk_ret_t
ep_mac_get_all (pds::LearnMACGetResponse *proto_rsp)
{
    learn_db()->ep_mac_db()->walk(ep_mac_entry_walk_cb, proto_rsp);
    return SDK_RET_OK;
}

static bool
ep_ip_entry_to_proto (void *entry, void *rsp)
{
    pds::LearnIPGetResponse *proto_rsp = (pds::LearnIPGetResponse *)rsp;
    pds::LearnIPEntry *proto_entry = proto_rsp->add_response();
    ep_ip_entry *ip_entry = (ep_ip_entry *)entry;
    ep_mac_entry *mac_entry = (ep_mac_entry *)ip_entry->mac_entry();

    proto_entry->set_state(pds_learn_state_to_proto(ip_entry->state()));
    ipaddr_api_spec_to_proto_spec(
                          proto_entry->mutable_key()->mutable_ipaddr(),
                          &ip_entry->key()->ip_addr);
    proto_entry->mutable_key()->set_vpcid(ip_entry->key()->vpc.id,
                                          PDS_MAX_KEY_LEN);
    if (mac_entry) {
        proto_entry->mutable_macinfo()->set_macaddr(
                          MAC_TO_UINT64(mac_entry->key()->mac_addr));
        proto_entry->mutable_macinfo()->set_subnetid(mac_entry->key()->subnet.id,
                                                    PDS_MAX_KEY_LEN);
    }
    return false;
}

static sdk_ret_t
ep_ip_get (ep_ip_key_t *key, pds::LearnIPGetResponse *proto_rsp)
{
    ep_ip_entry *ip_entry;

    ip_entry = (ep_ip_entry *)learn_db()->ep_ip_db()->find(key);
    if (!ip_entry) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    ep_ip_entry_to_proto(ip_entry, proto_rsp);
    return SDK_RET_OK;
}

static sdk_ret_t
ep_ip_get_all (pds::LearnIPGetResponse *proto_rsp)
{
    learn_db()->ep_ip_db()->walk(ep_ip_entry_to_proto, proto_rsp);
    return SDK_RET_OK;
}

static sdk_ret_t
ep_learn_stats_fill (pds::LearnStatsGetResponse *proto_rsp)
{
    learn::learn_counters_t *counters = learn_db()->counters();

    proto_rsp->mutable_stats()->set_numpktsrcvd(counters->rx_pkts);
    proto_rsp->mutable_stats()->set_numpktssent(counters->tx_pkts);
    proto_rsp->mutable_stats()->set_numvnics(counters->vnics);
    proto_rsp->mutable_stats()->set_numl3mappings(counters->l3_mappings);
    proto_rsp->mutable_stats()->set_numapicalls(counters->api_calls);
    proto_rsp->mutable_stats()->set_numapifailure(counters->api_failure);

    for (uint8_t i = (uint8_t)learn::PKT_DROP_REASON_NONE;
         i < (uint8_t)learn::PKT_DROP_REASON_MAX; i++) {
        if (counters->drop_reason[i]) {
            pds::LearnPktDropStats *drop_stats
                            = proto_rsp->mutable_stats()->add_dropstats();
            drop_stats->set_reason(pds_learn_pkt_drop_reason_to_proto(i));
            drop_stats->set_numdrops(counters->drop_reason[i]);
        }
    }
    return SDK_RET_OK;
}

Status
LearnSvcImpl::LearnMACGet(ServerContext *context,
                          const ::pds::LearnMACRequest* proto_req,
                          pds::LearnMACGetResponse *proto_rsp) {
    sdk_ret_t ret;
    ep_mac_key_t mac_key;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    if (proto_req->key_size() == 0) {
        // return all MACs learnt
        ret = ep_mac_get_all(proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    } else {
        for (int i = 0; i < proto_req->key_size(); i++) {
            pds_learn_mackey_proto_to_api(&mac_key, proto_req->key(i));
            ret = ep_mac_get(&mac_key, proto_rsp);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            if (ret != SDK_RET_OK) {
                break;
            }
        }
    }
    return Status::OK;
}

Status
LearnSvcImpl::LearnIPGet(ServerContext* context,
                         const ::pds::LearnIPRequest* proto_req,
                         pds::LearnIPGetResponse* proto_rsp) {
    sdk_ret_t ret;
    ep_ip_key_t ip_key;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    if (proto_req->key_size() == 0) {
        // return all IPs learnt
        ret = ep_ip_get_all(proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    } else {
        for (int i = 0; i < proto_req->key_size(); i++) {
            pds_learn_ipkey_proto_to_api(&ip_key, proto_req->key(i));
            ret = ep_ip_get(&ip_key, proto_rsp);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            if (ret != SDK_RET_OK) {
                break;
            }
        }
    }
    return Status::OK;
}

Status
LearnSvcImpl::LearnStatsGet(ServerContext* context,
                            const types::Empty* proto_req,
                            pds::LearnStatsGetResponse* proto_rsp) {
    sdk_ret_t ret;

    ret = ep_learn_stats_fill(proto_rsp);
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::OK;
}

Status
LearnSvcImpl::LearnMACClear(ServerContext *context,
                            const ::pds::LearnMACRequest* proto_req,
                            pds::LearnClearResponse *proto_rsp) {
    sdk_ret_t ret;
    ep_mac_key_t mac_key;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    if (proto_req->key_size() == 0) {
        // clear all MACs learnt
        learn_mac_clear(NULL);
    } else {
        for (int i = 0; i < proto_req->key_size(); i++) {
            pds_learn_mackey_proto_to_api(&mac_key, proto_req->key(i));
            ret = learn_mac_clear(&mac_key);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            if (ret != SDK_RET_OK) {
                break;
            }
        }
    }
    return Status::OK;
}

Status
LearnSvcImpl::LearnIPClear(ServerContext* context,
                           const ::pds::LearnIPRequest* proto_req,
                           pds::LearnClearResponse* proto_rsp) {
    sdk_ret_t ret;
    ep_ip_key_t ip_key;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    if (proto_req->key_size() == 0) {
        // clear all IPs learnt
        learn_ip_clear(NULL);
    } else {
        for (int i = 0; i < proto_req->key_size(); i++) {
            pds_learn_ipkey_proto_to_api(&ip_key, proto_req->key(i));
            ret = learn_ip_clear(&ip_key);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            if (ret != SDK_RET_OK) {
                break;
            }
        }
    }
    return Status::OK;
}

Status
LearnSvcImpl::LearnStatsClear(ServerContext* context,
                              const types::Empty* proto_req,
                              types::Empty* proto_rsp) {
    //TODO: make it an ipc later, since it is just about counters,
    // leaving it as a direct function call
    learn_db()->reset_counters();
    return Status::OK;
}

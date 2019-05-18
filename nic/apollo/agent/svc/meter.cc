//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_meter.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/meter.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/meter.hpp"

// build Meter api spec from proto buf spec
static inline sdk_ret_t
pds_meter_proto_spec_to_api_spec (pds_meter_spec_t *api_spec,
                                  const pds::MeterSpec &proto_spec)
{
    sdk_ret_t ret;

    api_spec->key.id = proto_spec.id();
    ret = pds_af_proto_spec_to_api_spec(&api_spec->af, proto_spec.af());
    if (ret != SDK_RET_OK) {
        return SDK_RET_INVALID_ARG;
    }

    api_spec->num_rules = proto_spec.rules_size();
    api_spec->rules =
        (pds_meter_rule_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_METER,
                        api_spec->num_rules * sizeof(pds_meter_rule_t));
    for (uint32_t rule = 0; rule < api_spec->num_rules; rule++) {
        const pds::MeterRuleSpec &proto_rule_spec = proto_spec.rules(rule);
        pds_meter_rule_t *api_rule_spec = &api_spec->rules[rule];
        if (proto_rule_spec.has_ppspolicer()) {
            api_rule_spec->type = PDS_METER_TYPE_PPS_POLICER;
            api_rule_spec->pps =
                            proto_rule_spec.ppspolicer().packetspersecond();
            api_rule_spec->pkt_burst = proto_rule_spec.ppspolicer().burst();
        } else if (proto_rule_spec.has_bpspolicer()) {
            api_rule_spec->type = PDS_METER_TYPE_BPS_POLICER;
            api_rule_spec->bps =
                            proto_rule_spec.bpspolicer().bytespersecond();
            api_rule_spec->byte_burst = proto_rule_spec.bpspolicer().burst();
        } else {
            api_rule_spec->type = PDS_METER_TYPE_ACCOUNTING;
        }

        api_rule_spec->num_prefixes = proto_rule_spec.prefix_size();
        api_rule_spec->prefixes =
                 (ip_prefix_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_METER,
                            api_rule_spec->num_prefixes * sizeof(ip_prefix_t));
        for (int pfx = 0; pfx < proto_rule_spec.prefix_size(); pfx++) {
            ippfx_proto_spec_to_api_spec(
                    &api_rule_spec->prefixes[pfx], proto_rule_spec.prefix(pfx));
        }
    }
    return SDK_RET_OK;
}

Status
MeterSvcImpl::MeterCreate(ServerContext *context,
                          const pds::MeterRequest *proto_req,
                          pds::MeterResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_meter_key_t key = {0};
    pds_meter_spec_t *api_spec = NULL;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_meter_spec_t *)
                    core::agent_state::state()->meter_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.id = request.id();
        ret = pds_meter_proto_spec_to_api_spec(api_spec, request);
        if (ret != SDK_RET_OK) {
            core::agent_state::state()->meter_slab()->free(api_spec);
            break;
        }
        ret = core::meter_create(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
MeterSvcImpl::MeterUpdate(ServerContext *context,
                          const pds::MeterRequest *proto_req,
                          pds::MeterResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_meter_key_t key = {0};
    pds_meter_spec_t *api_spec = NULL;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_meter_spec_t *)
                    core::agent_state::state()->meter_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.id = request.id();
        ret = pds_meter_proto_spec_to_api_spec(api_spec, request);
        if (ret != SDK_RET_OK) {
            core::agent_state::state()->meter_slab()->free(api_spec);
            break;
        }
        ret = core::meter_update(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
MeterSvcImpl::MeterDelete(ServerContext *context,
                          const pds::MeterDeleteRequest *proto_req,
                          pds::MeterDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_meter_key_t key = {0};

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::meter_delete(&key);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

// Populate proto buf status from meter API status
static inline void
meter_api_status_to_proto_status (const pds_meter_status_t *api_status,
                                  pds::MeterStatus *proto_status)
{
}

// Populate proto buf stats from meter API stats
static inline void
meter_api_stats_to_proto_stats (const pds_meter_stats_t *api_stats,
                                pds::MeterStats *proto_stats)
{
}

// Populate proto buf from meter API info
static inline void
meter_api_info_to_proto (const pds_meter_info_t *api_info, void *ctxt)
{
    pds::MeterGetResponse *proto_rsp = (pds::MeterGetResponse *)ctxt;
    auto meter = proto_rsp->add_response();
    pds::MeterSpec *proto_spec = meter->mutable_spec();
    pds::MeterStatus *proto_status = meter->mutable_status();
    pds::MeterStats *proto_stats = meter->mutable_stats();

    meter_api_spec_to_proto_spec(&api_info->spec, proto_spec);
    meter_api_status_to_proto_status(&api_info->status, proto_status);
    meter_api_stats_to_proto_stats(&api_info->stats, proto_stats);
}

Status
MeterSvcImpl::MeterGet(ServerContext *context,
                       const pds::MeterGetRequest *proto_req,
                     pds::MeterGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_meter_key_t key = {0};
    pds_meter_info_t info;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::meter_get(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        meter_api_info_to_proto(&info, proto_rsp);
    }

    if (proto_req->id_size() == 0) {
        ret = core::meter_get_all(meter_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

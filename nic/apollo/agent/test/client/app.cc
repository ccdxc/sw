//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------

#define __STDC_FORMAT_MACROS

#include <grpc++/grpc++.h>
#include "gen/proto/batch.grpc.pb.h"
#include "gen/proto/gogo.grpc.pb.h"
#include "gen/proto/mapping.grpc.pb.h"
#include "gen/proto/vpc.grpc.pb.h"
#include "gen/proto/route.grpc.pb.h"
#include "gen/proto/policy.grpc.pb.h"
#include "gen/proto/subnet.grpc.pb.h"
#include "gen/proto/device.grpc.pb.h"
#include "gen/proto/tunnel.grpc.pb.h"
#include "gen/proto/vnic.grpc.pb.h"
#include "gen/proto/mirror.grpc.pb.h"
#include "gen/proto/meter.grpc.pb.h"
#include "gen/proto/tags.grpc.pb.h"
#include "gen/proto/nh.grpc.pb.h"
#include "gen/proto/policer.grpc.pb.h"
#include "gen/proto/types.grpc.pb.h"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/api/include/pds_policer.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_meter.hpp"
#include "nic/apollo/api/include/pds_tag.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/agent/svc/specs.hpp"

using std::string;
using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using pds::VPCDeleteResponse;
using pds::VPCGetResponse;
using pds::VPCPeerResponse;
using pds::PolicerRequest;
using pds::PolicerResponse;
using pds::PolicerSpec;
using pds::NexthopResponse;
using pds::NexthopSpec;
using pds::NhGroupSpec;
using pds::NhGroupResponse;
using pds::BatchSpec;
using pds::BatchStatus;
using types::BatchCtxt;

std::string g_svc_endpoint_;

std::unique_ptr<pds::RouteSvc::Stub>             g_route_table_stub_;
std::unique_ptr<pds::MappingSvc::Stub>           g_mapping_stub_;
std::unique_ptr<pds::VnicSvc::Stub>              g_vnic_stub_;
std::unique_ptr<pds::SubnetSvc::Stub>            g_subnet_stub_;
std::unique_ptr<pds::VPCSvc::Stub>               g_vpc_stub_;
std::unique_ptr<pds::NatSvc::Stub>               g_nat_stub_;
std::unique_ptr<pds::TunnelSvc::Stub>            g_tunnel_stub_;
std::unique_ptr<pds::DeviceSvc::Stub>            g_device_stub_;
std::unique_ptr<pds::BatchSvc::Stub>             g_batch_stub_;
std::unique_ptr<pds::SecurityPolicySvc::Stub>    g_policy_stub_;
std::unique_ptr<pds::MirrorSvc::Stub>            g_mirror_stub_;
std::unique_ptr<pds::MeterSvc::Stub>             g_meter_stub_;
std::unique_ptr<pds::TagSvc::Stub>               g_tag_stub_;
std::unique_ptr<pds::NhSvc::Stub>                g_nexthop_stub_;
std::unique_ptr<pds::PolicerSvc::Stub>           g_policer_stub_;
std::unique_ptr<pds::IfSvc::Stub>                g_if_stub_;
std::unique_ptr<pds::Svc::Stub>                  g_svc_mapping_stub_;
std::unique_ptr<pds::DHCPSvc::Stub>              g_dhcp_stub_;

pds::RouteTableRequest        g_route_table_req;
pds::SecurityPolicyRequest    g_policy_req;
pds::MappingRequest           g_mapping_req;
pds::VnicRequest              g_vnic_req;
pds::SubnetRequest            g_subnet_req;
pds::VPCRequest               g_vpc_req;
pds::VPCPeerRequest           g_vpc_peer_req;
pds::TunnelRequest            g_tunnel_req;
pds::MirrorSessionRequest     g_mirror_session_req;
pds::MeterRequest             g_meter_req;
pds::TagRequest               g_tag_req;
pds::PolicerRequest           g_policer_req;
pds::NexthopRequest           g_nexthop_req;
pds::NhGroupRequest           g_nexthop_group_req;
pds::SvcMappingRequest        g_svc_mapping_req;
pds::VPCDeleteRequest         g_vpc_req_del;
pds::VPCGetRequest            g_vpc_req_get;
pds::NatPortBlockRequest      g_nat_port_block_req;
pds::InterfaceRequest         g_if_req;

#define APP_GRPC_BATCH_COUNT    5000

sdk_ret_t
create_route_table_grpc (pds_route_table_spec_t *spec)
{
    ClientContext       context;
    RouteTableResponse  response;
    Status              ret_status;

    if (spec) {
        pds_route_table_api_spec_to_proto(g_route_table_req.add_request(), spec);
    }
    if ((g_route_table_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
        ret_status = g_route_table_stub_->RouteTableCreate(&context, g_route_table_req, &response);
        if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
        g_route_table_req.clear_request();
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_policy_grpc (pds_policy_spec_t *spec)
{
    ClientContext             context;
    SecurityPolicyResponse    response;
    Status                    ret_status;

    if (spec != NULL) {
        pds::SecurityPolicySpec *proto_spec = g_policy_req.add_request();
        pds_policy_api_spec_to_proto(proto_spec, spec);
    }
    if ((g_policy_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
        ret_status = g_policy_stub_->SecurityPolicyCreate(&context,
                                                          g_policy_req,
                                                          &response);
        if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
        g_policy_req.clear_request();
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_local_mapping_grpc (pds_local_mapping_spec_t *spec)
{
    ClientContext   context;
    MappingResponse response;
    Status          ret_status;

    if (spec) {
        pds_local_mapping_api_spec_to_proto(g_mapping_req.add_request(), spec);
    }
    if ((g_mapping_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
        ret_status = g_mapping_stub_->MappingCreate(&context, g_mapping_req, &response);
        if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
        g_mapping_req.clear_request();
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_remote_mapping_grpc (pds_remote_mapping_spec_t *spec)
{
    ClientContext   context;
    MappingResponse response;
    Status          ret_status;

    if (spec) {
        pds_remote_mapping_api_spec_to_proto(g_mapping_req.add_request(), spec);
    }
    if ((g_mapping_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
        ret_status = g_mapping_stub_->MappingCreate(&context, g_mapping_req, &response);
        if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
        g_mapping_req.clear_request();
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_vnic_grpc (pds_vnic_spec_t *spec)
{
    ClientContext   context;
    VnicResponse    response;
    Status          ret_status;

    if (spec != NULL) {
        pds::VnicSpec *proto_spec = g_vnic_req.add_request();
        pds_vnic_api_spec_to_proto(proto_spec, spec);
    }
    if ((g_vnic_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
        ret_status = g_vnic_stub_->VnicCreate(&context, g_vnic_req, &response);
        if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
        g_vnic_req.clear_request();
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_subnet_grpc (pds_subnet_spec_t *spec)
{
    ClientContext   context;
    SubnetResponse  response;
    Status          ret_status;

    if (spec) {
        pds_subnet_api_spec_to_proto(g_subnet_req.add_request(), spec);
    }
    if ((g_subnet_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
        ret_status = g_subnet_stub_->SubnetCreate(&context, g_subnet_req, &response);
        if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
        g_subnet_req.clear_request();
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_nat_port_block_grpc (pds_nat_port_block_spec_t *spec)
{
    ClientContext           context;
    NatPortBlockResponse    response;
    Status                  status;

    if (spec) {
        pds_nat_port_block_api_spec_to_proto(g_nat_port_block_req.add_request(), spec);
    }
    if ((g_nat_port_block_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
        status = g_nat_stub_->NatPortBlockCreate(&context, g_nat_port_block_req, &response);
        if (!status.ok() ||
            (response.apistatus() != types::API_STATUS_OK)) {
            printf("create nat port block %s failed, status %u response %u "
                   "err %u\n", spec ? spec->key.str() : "", status.ok(),
                   response.apistatus(), status.error_code());
            return SDK_RET_ERR;
        }
        g_nat_port_block_req.clear_request();
    }
    return SDK_RET_OK;
}

sdk_ret_t
create_vpc_grpc (pds_vpc_spec_t *spec)
{
    ClientContext   context;
    VPCResponse     response;
    Status          ret_status;

    if (spec) {
        pds_vpc_api_spec_to_proto(g_vpc_req.add_request(), spec);
    }
    if ((g_vpc_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
        ret_status = g_vpc_stub_->VPCCreate(&context, g_vpc_req, &response);
        if (!ret_status.ok() ||
            (response.apistatus() != types::API_STATUS_OK)) {
            printf("create vpc %s failed, status %u, response %u, err %u\n",
                   spec ? spec->key.str() : "", ret_status.ok(),
                   response.apistatus(), ret_status.error_code());
            return SDK_RET_ERR;
        }
        g_vpc_req.clear_request();
    }

    return SDK_RET_OK;
}

sdk_ret_t
update_vpc_grpc (pds_vpc_spec_t *vpc)
{
    ClientContext   context;
    VPCResponse     response;
    Status          ret_status;

    if (vpc) {
        pds_vpc_api_spec_to_proto(g_vpc_req.add_request(), vpc);
    }
    if ((g_vpc_req.request_size() >= APP_GRPC_BATCH_COUNT) || !vpc) {
        ret_status = g_vpc_stub_->VPCUpdate(&context, g_vpc_req, &response);
        if (!ret_status.ok() ||
            (response.apistatus() != types::API_STATUS_OK)) {
            printf("update vpc %s failed, status %u, response %u, err %u\n",
                   vpc ? vpc->key.str() : "", ret_status.ok(),
                   response.apistatus(), ret_status.error_code());
            return SDK_RET_ERR;
        }
        g_vpc_req.clear_request();
    }
    return SDK_RET_OK;
}

sdk_ret_t
delete_vpc_grpc (pds_obj_key_t *key)
{
    ClientContext   context;
    VPCDeleteResponse     response;
    Status          ret_status;

    g_vpc_req_del.add_id(key->id);

    if ((g_vpc_req.request_size() >= APP_GRPC_BATCH_COUNT) || !key) {
        ret_status = g_vpc_stub_->VPCDelete(&context, g_vpc_req_del, &response);
        if (!ret_status.ok()) {
            printf("delete vpc failed ret_status:%u err:%u\n", ret_status.ok(),
                   ret_status.error_code());
            return SDK_RET_ERR;
        }
        for (int i = 0; i < response.apistatus_size(); i++) {
            int status = response.apistatus(i);
            if (status != types::API_STATUS_OK) {
                printf("%s failed for request i:%d, status:%d\n",
                       __FUNCTION__, i, status);
                return SDK_RET_ERR;
            }
        }
        g_vpc_req_del.clear_id();
    }

    return SDK_RET_OK;
}

sdk_ret_t
read_vpc_grpc (pds_obj_key_t *key, pds_vpc_info_t *info)
{
    ClientContext   context;
    VPCGetResponse  response;
    Status          ret_status;
    pds::VPCSpec vpcspec;

    // TODO - check if batching is needed for read APIs, for now dont batch
    g_vpc_req_get.add_id(key->id);
    if ((g_vpc_req_get.id_size() >= APP_GRPC_BATCH_COUNT) || !key || 1) {
        ret_status = g_vpc_stub_->VPCGet(&context, g_vpc_req_get, &response);
        if (!ret_status.ok()) {
            printf("get vpc %s failed,status %u, response %u, err %u\n",
                   key ? key->str() : "", ret_status.ok(),
                   response.apistatus(), ret_status.error_code());
            return SDK_RET_ERR;
        }
        g_vpc_req_get.clear_id();
        if (response.apistatus() == types::API_STATUS_NOT_FOUND) {
            // ApiStatus::NOTFOUND is 5, SDK_RET_ENTRY_NOT_FOUND is 4
            return SDK_RET_ENTRY_NOT_FOUND;
        } else if (response.apistatus() == types::API_STATUS_OK) {
            auto getresponse = response.response(0);  // get first info
            vpcspec = getresponse.spec();
            pds_vpc_proto_to_api_spec(&info->spec, vpcspec);
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_l3_intf_grpc (pds_if_spec_t *spec)
{
    ClientContext     context;
    InterfaceResponse response;
    Status            ret_status;

    if (spec != NULL) {
        pds::InterfaceSpec *proto_spec = g_if_req.add_request();
        pds_if_api_spec_to_proto(proto_spec, spec);
    }
    if ((g_if_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
        ret_status = g_if_stub_->InterfaceCreate(&context, g_if_req, &response);
        if (!ret_status.ok() ||
            (response.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
        g_if_req.clear_request();
    }
    return SDK_RET_OK;
}

sdk_ret_t
create_vpc_peer_grpc (pds_vpc_peer_spec_t *spec)
{
    ClientContext   context;
    VPCPeerResponse response;
    Status          ret_status;

    if (spec != NULL) {
        pds::VPCPeerSpec *proto_spec = g_vpc_peer_req.add_request();
        pds_vpc_peer_api_spec_to_proto(proto_spec, spec);
    }
    if ((g_vpc_peer_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
        ret_status = g_vpc_stub_->VPCPeerCreate(&context, g_vpc_peer_req, &response);
        if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
        g_vpc_peer_req.clear_request();
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_tag_grpc (pds_tag_spec_t *spec)
{
    ClientContext context;
    TagResponse   response;
    Status        ret_status;

    if (spec != NULL) {
        pds::TagSpec *proto_spec = g_tag_req.add_request();
        pds_tag_api_spec_to_proto(proto_spec, spec);
    }
    if ((g_tag_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
        ret_status = g_tag_stub_->TagCreate(&context, g_tag_req, &response);
        if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
        g_tag_req.clear_request();
    }
    return SDK_RET_OK;
}

sdk_ret_t
create_meter_grpc (pds_meter_spec_t *spec)
{
    ClientContext context;
    MeterResponse response;
    Status        ret_status;

    if (spec != NULL) {
        MeterSpec *proto_spec = g_meter_req.add_request();
        pds_meter_api_spec_to_proto(proto_spec, spec);
    }
    if ((g_meter_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
        ret_status = g_meter_stub_->MeterCreate(&context, g_meter_req, &response);
        if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
        g_meter_req.clear_request();
    }
    return SDK_RET_OK;
}

sdk_ret_t
create_policer_grpc (pds_policer_spec_t *spec)
{
    ClientContext   context;
    PolicerResponse response;
    Status          ret_status;

    if (spec != NULL) {
        PolicerSpec *proto_spec = g_policer_req.add_request();
        pds_policer_api_spec_to_proto(proto_spec, spec);
    }
    if ((g_policer_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
       ret_status = g_policer_stub_->PolicerCreate(
                                        &context, g_policer_req, &response);
        if (!ret_status.ok() ||
                        (response.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
        g_policer_req.clear_request();
    }
    return SDK_RET_OK;
}

sdk_ret_t
create_nexthop_grpc (pds_nexthop_spec_t *spec)
{
    ClientContext   context;
    NexthopResponse response;
    Status          ret_status;

    if (spec != NULL) {
        NexthopSpec *proto_spec = g_nexthop_req.add_request();
        pds_nh_api_spec_to_proto(proto_spec, spec);
    }
    if ((g_nexthop_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
       ret_status = g_nexthop_stub_->NexthopCreate(
                                        &context, g_nexthop_req, &response);
        if (!ret_status.ok() ||
                        (response.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
        g_nexthop_req.clear_request();
    }
    return SDK_RET_OK;
}

sdk_ret_t
create_nexthop_group_grpc (pds_nexthop_group_spec_t *spec)
{
    ClientContext   context;
    NhGroupResponse response;
    Status          ret_status;

    if (spec != NULL) {
        NhGroupSpec *proto_spec = g_nexthop_group_req.add_request();
        pds_nh_group_api_spec_to_proto(proto_spec, spec);
    }
    if ((g_nexthop_group_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
       ret_status = g_nexthop_stub_->NhGroupCreate(&context,
                                         g_nexthop_group_req, &response);
        if (!ret_status.ok() ||
                        (response.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
        g_nexthop_group_req.clear_request();
    }
    return SDK_RET_OK;
}

sdk_ret_t
create_tunnel_grpc (uint32_t id, pds_tep_spec_t *spec)
{
    ClientContext   context;
    TunnelResponse  response;
    Status          ret_status;

    if (spec) {
        pds_tep_api_spec_to_proto(g_tunnel_req.add_request(), spec);
    }
    if ((g_tunnel_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
        ret_status =
            g_tunnel_stub_->TunnelCreate(&context, g_tunnel_req, &response);
        if (!ret_status.ok() ||
            (response.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
        g_tunnel_req.clear_request();
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_device_grpc (pds_device_spec_t *spec)
{
    DeviceRequest   request;
    ClientContext   context;
    DeviceResponse  response;
    Status          ret_status;

    if (spec) {
        pds_device_api_spec_to_proto(request.mutable_request(), spec);
    }
    ret_status = g_device_stub_->DeviceCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed!\n", __FUNCTION__);
        return SDK_RET_ERR;
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_mirror_session_grpc (pds_mirror_session_spec_t *spec)
{
    ClientContext            context;
    MirrorSessionResponse    response;
    Status                   status;

    if (spec) {
        pds_mirror_session_api_spec_to_proto(g_mirror_session_req.add_request(), spec);
    }
    if ((g_mirror_session_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
        status = g_mirror_stub_->MirrorSessionCreate(&context,
                                                     g_mirror_session_req,
                                                     &response);
        if (!status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
        g_mirror_session_req.clear_request();
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_svc_mapping_grpc (pds_svc_mapping_spec_t *spec)
{
    // avinash/srikanth .. please check this path
    ClientContext         context;
    SvcMappingResponse    response;
    Status                status;

    if (spec) {
        pds_service_api_spec_to_proto(g_svc_mapping_req.add_request(), spec);
    }
    if ((g_svc_mapping_req.request_size() >= APP_GRPC_BATCH_COUNT) || !spec) {
        status = g_svc_mapping_stub_->SvcMappingCreate(&context,
                                                       g_svc_mapping_req,
                                                       &response);
        if (!status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
            printf("%s failed!\n", __FUNCTION__);
            return SDK_RET_ERR;
        }
        g_svc_mapping_req.clear_request();
    }

    return SDK_RET_OK;
}

pds_batch_ctxt_t
batch_start_grpc (int epoch)
{
    BatchSpec           spec;
    ClientContext       start_context;
    Status              ret_status;
    pds_batch_params_t  params = { 0 };
    BatchStatus         status;

    params.epoch = epoch;
    spec.set_epoch(params.epoch);

    // batch start
    ret_status = g_batch_stub_->BatchStart(&start_context, spec, &status);
    if (!ret_status.ok()) {
        printf("%s failed!\n", __FUNCTION__);
        return PDS_BATCH_CTXT_INVALID;
    }
    return status.batchcontext().batchcookie();
}

sdk_ret_t
batch_commit_grpc (pds_batch_ctxt_t bctxt)
{
    ClientContext       commit_context;
    BatchCtxt           batch_context;
    types::Empty        empty_spec;
    types::Empty        response;
    Status              ret_status;

    batch_context.set_batchcookie(bctxt);
    ret_status = g_batch_stub_->BatchCommit(&commit_context,
                                            batch_context, &response);
    if (!ret_status.ok()) {
        printf("%s: Batch commit failed!\n", __FUNCTION__);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

void
test_app_init (void)
{
    grpc_init();
    if (g_svc_endpoint_.empty()) {
        g_svc_endpoint_ = std::string("localhost:50054");
    }
    std::shared_ptr<Channel> channel =
        grpc::CreateChannel(g_svc_endpoint_,
                            grpc::InsecureChannelCredentials());
    g_route_table_stub_ = pds::RouteSvc::NewStub(channel);
    g_policy_stub_ = pds::SecurityPolicySvc::NewStub(channel);
    g_mapping_stub_ = pds::MappingSvc::NewStub(channel);
    g_vnic_stub_ = pds::VnicSvc::NewStub(channel);
    g_subnet_stub_ = pds::SubnetSvc::NewStub(channel);
    g_vpc_stub_ = pds::VPCSvc::NewStub(channel);
    g_tunnel_stub_ = pds::TunnelSvc::NewStub(channel);
    g_device_stub_ = pds::DeviceSvc::NewStub(channel);
    g_batch_stub_ = pds::BatchSvc::NewStub(channel);
    g_mirror_stub_ = pds::MirrorSvc::NewStub(channel);
    g_meter_stub_ = pds::MeterSvc::NewStub(channel);
    g_tag_stub_ = pds::TagSvc::NewStub(channel);
    g_policer_stub_ = pds::PolicerSvc::NewStub(channel);
    g_nexthop_stub_ = pds::NhSvc::NewStub(channel);
    g_if_stub_ = pds::IfSvc::NewStub(channel);
    g_nat_stub_ = pds::NatSvc::NewStub(channel);
    g_dhcp_stub_ = pds::DHCPSvc::NewStub(channel);
    g_svc_mapping_stub_ = pds::Svc::NewStub(channel);

    return;
}

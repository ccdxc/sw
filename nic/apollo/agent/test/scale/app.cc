//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <thread>
#include <iostream>
#include <fstream>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <google/protobuf/util/json_util.h>
#include <stdio.h>
#include <getopt.h>
#include <gtest/gtest.h>
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/agent/test/scale/utils.hpp"
#include "nic/apollo/agent/test/scale/app.hpp"
#include "nic/apollo/test/scale/test.hpp"

using std::string;

std::string  svc_endpoint_  = "localhost:9999";

std::unique_ptr<pds::RouteSvc::Stub>     g_route_table_stub_;
std::unique_ptr<pds::MappingSvc::Stub>   g_mapping_stub_;
std::unique_ptr<pds::VnicSvc::Stub>      g_vnic_stub_;
std::unique_ptr<pds::SubnetSvc::Stub>    g_subnet_stub_;
std::unique_ptr<pds::PCNSvc::Stub>       g_pcn_stub_;
std::unique_ptr<pds::TunnelSvc::Stub>    g_tunnel_stub_;
std::unique_ptr<pds::DeviceSvc::Stub>    g_device_stub_;
std::unique_ptr<pds::BatchSvc::Stub>     g_batch_stub_;

sdk_ret_t
create_route_table_grpc (pds_route_table_spec_t *rt)
{
    RouteTableRequest   request;
    ClientContext       context;
    RouteTableResponse  response;
    Status              ret_status;

    populate_route_table_request(&request, rt);
    ret_status = g_route_table_stub_->RouteTableCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s: failed!\n", __FUNCTION__);
        return SDK_RET_ERR;
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_mapping_grpc (pds_mapping_spec_t *mapping)
{
    MappingRequest  request;
    ClientContext   context;
    MappingResponse response;
    Status          ret_status;

    populate_mapping_request(&request, mapping);
    ret_status = g_mapping_stub_->MappingCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s: failed!\n", __FUNCTION__);
        return SDK_RET_ERR;
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_vnic_grpc (pds_vnic_spec_t *vnic)
{
    VnicRequest     request;
    ClientContext   context;
    VnicResponse    response;
    Status          ret_status;

    populate_vnic_request(&request, vnic);
    ret_status = g_vnic_stub_->VnicCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s: failed!\n", __FUNCTION__);
        return SDK_RET_ERR;
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_subnet_grpc (pds_subnet_spec_t *subnet)
{
    SubnetRequest   request;
    ClientContext   context;
    SubnetResponse  response;
    Status          ret_status;

    populate_subnet_request(&request, subnet);
    ret_status = g_subnet_stub_->SubnetCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s: failed!\n", __FUNCTION__);
        return SDK_RET_ERR;
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_vcn_grpc (pds_vcn_spec_t *vcn)
{
    PCNRequest      request;
    ClientContext   context;
    PCNResponse     response;
    Status          ret_status;

    populate_pcn_request(&request, vcn);
    ret_status = g_pcn_stub_->PCNCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s: failed!\n", __FUNCTION__);
        return SDK_RET_ERR;
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_tunnel_grpc (uint32_t id, pds_tep_spec_t *tep)
{
    TunnelRequest   request;
    ClientContext   context;
    TunnelResponse  response;
    Status          ret_status;

    populate_tunnel_request(&request, id, tep);
    ret_status = g_tunnel_stub_->TunnelCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s: failed!\n", __FUNCTION__);
        return SDK_RET_ERR;
    }

    return SDK_RET_OK;
}

sdk_ret_t
create_device_grpc (pds_device_spec_t *device)
{
    DeviceRequest   request;
    ClientContext   context;
    DeviceResponse  response;
    Status          ret_status;

    populate_device_request(&request, device);
    ret_status = g_device_stub_->DeviceCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s: failed!\n", __FUNCTION__);
        return SDK_RET_ERR;
    }
    
    return SDK_RET_OK;
}

sdk_ret_t
test_app_push_configs (void)
{
    BatchSpec           spec;
    ClientContext       start_context;
    ClientContext       commit_context;
    BatchStatus         status;
    Status              ret_status;
    pds_batch_params_t  params;
    types::Empty        empty_spec;
    types::Empty        response;

    params.epoch = 1;
    populate_batch_spec(&spec, &params);
    
    /* Batch start */
    printf("%s: Batch start\n", __FUNCTION__);
    ret_status = g_batch_stub_->BatchStart(&start_context, spec, &status);
    if (!ret_status.ok()) {
        printf("%s: failed!\n", __FUNCTION__);
        return SDK_RET_ERR;
    }
    
    printf("%s: Pushing objects...\n", __FUNCTION__);
    /* Create objects */
    create_objects();
    
#if 0
    // TODO: Adding this here since there is no proto defs for
    // flows. This needs to be cleaned up
    sdk_ret_t ret = SDK_RET_OK;

    ret = g_flow_test_obj->create_flows(1024*1024, 17, 100, 100, false);
    if (ret != sdk::SDK_RET_OK) {
        return Status::CANCELLED;
    }

    ret = g_flow_test_obj->create_flows(1024*1024, 17, 100, 100, true);
    if (ret != sdk::SDK_RET_OK) {
        return Status::CANCELLED;
    }
#endif
    
    printf("%s: Batch commit START\n", __FUNCTION__);
    /* Batch commit */
    ret_status = g_batch_stub_->BatchCommit(&commit_context, empty_spec, &response);
    if (!ret_status.ok()) {
        printf("%s: Batch commit failed!\n", __FUNCTION__);
        return SDK_RET_ERR;
    }
    printf("%s: Batch commit DONE!\n", __FUNCTION__);

    return SDK_RET_OK;
}

void
test_app_init (void)
{
    grpc_init();
    std::shared_ptr<Channel> channel = grpc::CreateChannel(svc_endpoint_,
                               grpc::InsecureChannelCredentials());
    g_route_table_stub_ = pds::RouteSvc::NewStub(channel);
    g_mapping_stub_ = pds::MappingSvc::NewStub(channel);
    g_vnic_stub_ = pds::VnicSvc::NewStub(channel);
    g_subnet_stub_ = pds::SubnetSvc::NewStub(channel);
    g_pcn_stub_ = pds::PCNSvc::NewStub(channel);
    g_tunnel_stub_ = pds::TunnelSvc::NewStub(channel);
    g_device_stub_ = pds::DeviceSvc::NewStub(channel);
    g_batch_stub_ = pds::BatchSvc::NewStub(channel);
    
    return;
}

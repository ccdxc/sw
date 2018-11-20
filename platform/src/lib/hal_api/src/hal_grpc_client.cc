
#include <string>
#include <memory>
#include <iostream>
#include <grpc++/grpc++.h>

#include "hal_grpc_client.hpp"

using namespace std;

#if 0
shared_ptr<HalCommonClient>
HalGRPCClient::GetInstance()
{
    if (!instance) {
        NIC_LOG_ERR("HalGRPCClient instance is not created!");
    }

    return instance;
}

shared_ptr<HalCommonClient>
HalGRPCClient::GetInstance(enum HalForwardingMode fwd_mode)
{
    if (!instance) {
        instance = shared_ptr<HalGRPCClient>(new HalGRPCClient(fwd_mode));
    }

    return instance;
}
#endif

HalCommonClient *
HalGRPCClient::Factory(enum HalForwardingMode mode)
{
    if (!instance) {
        instance = new HalGRPCClient(mode);
    }

    return instance;
}

void
HalGRPCClient::Destroy(HalCommonClient *hal_client)
{
    delete hal_client;

    instance = NULL;
}

HalGRPCClient::HalGRPCClient(enum HalForwardingMode mode) : HalCommonClient(mode)
{
    std::string svc_url;

    if (getenv("HAL_SOCK_PATH")) {
        svc_url = std::string("unix:") + std::getenv("HAL_SOCK_PATH") + "halsock";
    } else if (getenv("HAL_GRPC_PORT")) {
        svc_url = std::string("localhost:") + getenv("HAL_GRPC_PORT");
    } else {
        svc_url = std::string("localhost:50054");
    }

    channel = grpc::CreateChannel(svc_url, grpc::InsecureChannelCredentials());

    // cout << "[INFO] Waiting for HAL to be ready ..." << endl;
    NIC_LOG_INFO("Waiting for HAL to be ready at: {}", svc_url);
    auto state = channel->GetState(true);
    while (state != GRPC_CHANNEL_READY) {
        // Wait for State change or deadline
        channel->WaitForStateChange(state, gpr_time_from_seconds(1, GPR_TIMESPAN));
        state = channel->GetState(true);
        // cout << "[INFO] Connecting to HAL, channel status = " << channel->GetState(true) << endl;
    }

    vrf_stub_ = vrf::Vrf::NewStub(channel);
    interface_stub_ = intf::Interface::NewStub(channel);
    endpoint_stub_ = endpoint::Endpoint::NewStub(channel);
    l2segment_stub_ = l2segment::L2Segment::NewStub(channel);
    multicast_stub_ = multicast::Multicast::NewStub(channel);
    rdma_stub_ = rdma::Rdma::NewStub(channel);
    qos_stub_ = qos::QOS::NewStub(channel);
}

#define HAL_CREATE_API(obj_api, obj_class, pkg)                             \
    Status                                                                  \
    HalGRPCClient::obj_api ## _create (obj_class ## RequestMsg& req_msg,    \
                                       obj_class ## ResponseMsg& rsp_msg)   \
    {                                                                       \
        grpc::ClientContext         context;                                \
        grpc::Status                status;                                 \
        status = pkg ## _stub_->obj_class ## Create(&context, req_msg, &rsp_msg);   \
        return status;                                                      \
    }

#define HAL_UPDATE_API(obj_api, obj_class, pkg)                             \
    Status                                                                  \
    HalGRPCClient::obj_api ## _update (obj_class ## RequestMsg& req_msg,    \
                                       obj_class ## ResponseMsg& rsp_msg)   \
    {                                                                       \
        grpc::ClientContext         context;                                \
        grpc::Status                status;                                 \
        status = pkg ## _stub_->obj_class ## Update(&context, req_msg, &rsp_msg);   \
        return status;                                                      \
    }

#define HAL_DELETE_API(obj_api, obj_class, pkg)                             \
    Status                                                                  \
    HalGRPCClient::obj_api ## _delete (obj_class ## DeleteRequestMsg& req_msg,    \
                                       obj_class ## DeleteResponseMsg& rsp_msg)   \
    {                                                                       \
        grpc::ClientContext         context;                                \
        grpc::Status                status;                                 \
        status = pkg ## _stub_->obj_class ## Delete(&context, req_msg, &rsp_msg);   \
        return status;                                                      \
    }

#define HAL_GET_API(obj_api, obj_class, pkg)                                \
    Status                                                                  \
    HalGRPCClient::obj_api ## _get (obj_class ## GetRequestMsg& req_msg,    \
                                    obj_class ## GetResponseMsg& rsp_msg)   \
    {                                                                       \
        grpc::ClientContext         context;                                \
        grpc::Status                status;                                 \
        status = pkg ## _stub_->obj_class ## Get(&context, req_msg, &rsp_msg);   \
        return status;                                                      \
    }

//-----------------------------------------------------------------------------
// Vrf Create to HAL
//-----------------------------------------------------------------------------
Status
HalGRPCClient::vrf_create (VrfRequestMsg& req_msg, VrfResponseMsg& rsp_msg)
{
    grpc::ClientContext         context;
    grpc::Status                status;

    status = vrf_stub_->VrfCreate(&context, req_msg, &rsp_msg);

    return status;
}

//-----------------------------------------------------------------------------
// Vrf Delete to HAL
//-----------------------------------------------------------------------------
Status
HalGRPCClient::vrf_delete (VrfDeleteRequestMsg& req_msg,
                           VrfDeleteResponseMsg& rsp_msg)
{
    grpc::ClientContext         context;
    grpc::Status                status;

    status = vrf_stub_->VrfDelete(&context, req_msg, &rsp_msg);

    return status;
}

//-----------------------------------------------------------------------------
// Vrf Update to HAL
//-----------------------------------------------------------------------------
Status
HalGRPCClient::vrf_update (VrfRequestMsg& req_msg, VrfResponseMsg& rsp_msg)
{
    grpc::ClientContext         context;
    grpc::Status                status;

    status = vrf_stub_->VrfUpdate(&context, req_msg, &rsp_msg);

    return status;
}

//-----------------------------------------------------------------------------
// Vrf Get to HAL
//-----------------------------------------------------------------------------
Status
HalGRPCClient::vrf_get (VrfGetRequestMsg& req_msg,
                        VrfGetResponseMsg& rsp_msg)
{
    grpc::ClientContext         context;
    grpc::Status                status;

    status = vrf_stub_->VrfGet(&context, req_msg, &rsp_msg);

    return status;
}

// HalL2Segment Calls
HAL_CREATE_API(l2segment, L2Segment, l2segment);
HAL_UPDATE_API(l2segment, L2Segment, l2segment);
HAL_DELETE_API(l2segment, L2Segment, l2segment);
HAL_GET_API(l2segment, L2Segment, l2segment);

// HalEndpoint Calls
HAL_CREATE_API(endpoint, Endpoint, endpoint);
HAL_DELETE_API(endpoint, Endpoint, endpoint);
HAL_GET_API(endpoint, Endpoint, endpoint);

// TODO: Cleanup once EP's Update request change
// HAL_DELETE_API(endpoint, HalEndpoint);
//-----------------------------------------------------------------------------
// HalEndpoint Update to HAL
//-----------------------------------------------------------------------------
Status
HalGRPCClient::endpoint_update (EndpointUpdateRequestMsg& req_msg,
                                EndpointUpdateResponseMsg& rsp_msg)
{
    grpc::ClientContext         context;
    grpc::Status                status;

    status = endpoint_stub_->EndpointUpdate(&context, req_msg, &rsp_msg);

    return status;
}

// Interface Calls
HAL_CREATE_API(interface, Interface, interface);
HAL_UPDATE_API(interface, Interface, interface);
HAL_DELETE_API(interface, Interface, interface);
HAL_GET_API(interface, Interface, interface);

// Lif Calls
HAL_CREATE_API(lif, Lif, interface);
HAL_UPDATE_API(lif, Lif, interface);
HAL_DELETE_API(lif, Lif, interface);
HAL_GET_API(lif, Lif, interface);

// HalMulticast Calls
HAL_CREATE_API(multicast, MulticastEntry, multicast);
HAL_UPDATE_API(multicast, MulticastEntry, multicast);
HAL_DELETE_API(multicast, MulticastEntry, multicast);
HAL_GET_API(multicast, MulticastEntry, multicast);

// Filter Calls
HAL_CREATE_API(filter, Filter, endpoint);
HAL_DELETE_API(filter, Filter, endpoint);
HAL_GET_API(filter, Filter, endpoint);

// Qos Calls
HAL_GET_API(qos_class, QosClass, qos);

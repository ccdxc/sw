#include "hal_grpc_client_mock.hpp"
HalCommonClient *HalCommonClient::instance = NULL;

HalCommonClient *
HalCommonClient::GetInstance()
{
    return instance;
}

HalCommonClient::HalCommonClient(){}
HalCommonClient::HalCommonClient(enum HalForwardingMode mode)
{
    this->mode = mode;
}

enum HalForwardingMode
HalCommonClient::GetMode()
{
    return mode;
}

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
#if 0
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
    NIC_LOG_DEBUG("Waiting for HAL to be ready at: {}", svc_url);
    auto state = channel->GetState(true);
    while (state != GRPC_CHANNEL_READY) {
        // Wait for State change or deadline
        channel->WaitForStateChange(state, gpr_time_from_seconds(1, GPR_TIMESPAN));
        state = channel->GetState(true);
        // cout << "[INFO] Connecting to HAL, channel status = " << channel->GetState(true) << endl;
    }

#if 0
    vrf_stub_ = vrf::Vrf::NewStub(channel);
    interface_stub_ = intf::Interface::NewStub(channel);
    endpoint_stub_ = endpoint::Endpoint::NewStub(channel);
    l2segment_stub_ = l2segment::L2Segment::NewStub(channel);
    multicast_stub_ = multicast::Multicast::NewStub(channel);
    rdma_stub_ = rdma::Rdma::NewStub(channel);
    qos_stub_ = qos::QOS::NewStub(channel);
#endif
#endif

    vrf_stub_ = NULL;
    interface_stub_ = NULL;
    endpoint_stub_ = NULL;
    l2segment_stub_ = NULL;
    multicast_stub_ = NULL;
    rdma_stub_ = NULL;
    qos_stub_ = NULL;
}

#define HAL_CREATE_API(obj_api, obj_class, pkg)                             \
    Status                                                                  \
    HalGRPCClient::obj_api ## _create (obj_class ## RequestMsg& req_msg,    \
                                       obj_class ## ResponseMsg& rsp_msg)   \
    {                                                                       \
        printf("Hal Mock: " #obj_api " create: Should not come here\n"); \
        return Status::OK;                                                  \
    }

#define HAL_UPDATE_API(obj_api, obj_class, pkg)                             \
    Status                                                                  \
    HalGRPCClient::obj_api ## _update (obj_class ## RequestMsg& req_msg,    \
                                       obj_class ## ResponseMsg& rsp_msg)   \
    {                                                                       \
        printf("Hal Mock: " #obj_api " update: Should not come here\n"); \
        return Status::OK; \
    }

#define HAL_DELETE_API(obj_api, obj_class, pkg)                             \
    Status                                                                  \
    HalGRPCClient::obj_api ## _delete (obj_class ## DeleteRequestMsg& req_msg,    \
                                       obj_class ## DeleteResponseMsg& rsp_msg)   \
    {                                                                       \
        printf("Hal Mock: " #obj_api " delete: Should not come here\n"); \
        return Status::OK; \
    }

#define HAL_GET_API(obj_api, obj_class, pkg)                                \
    Status                                                                  \
    HalGRPCClient::obj_api ## _get (obj_class ## GetRequestMsg& req_msg,    \
                                    obj_class ## GetResponseMsg& rsp_msg)   \
    {                                                                       \
        printf("Hal Mock: " #obj_api " get: Should not come here\n"); \
        return Status::OK; \
    }

//-----------------------------------------------------------------------------
// Vrf Create to HAL
//-----------------------------------------------------------------------------
Status
HalGRPCClient::vrf_create (VrfRequestMsg& req_msg, VrfResponseMsg& rsp_msg)
{
    printf("Hal Mock: Vrf Create\n");
    auto response = rsp_msg.add_response();
    response->set_api_status(types::API_STATUS_OK);
    return Status::OK;
}

//-----------------------------------------------------------------------------
// Vrf Delete to HAL
//-----------------------------------------------------------------------------
Status
HalGRPCClient::vrf_delete (VrfDeleteRequestMsg& req_msg,
                           VrfDeleteResponseMsg& rsp_msg)
{
    printf("Hal Mock: Vrf Delete\n");
    return Status::OK;
#if 0
    grpc::ClientContext         context;
    grpc::Status                status;

    status = vrf_stub_->VrfDelete(&context, req_msg, &rsp_msg);

    return status;
#endif
}

//-----------------------------------------------------------------------------
// Vrf Update to HAL
//-----------------------------------------------------------------------------
Status
HalGRPCClient::vrf_update (VrfRequestMsg& req_msg, VrfResponseMsg& rsp_msg)
{
    printf("Hal Mock: Vrf Update\n");
    return Status::OK;
#if 0
    grpc::ClientContext         context;
    grpc::Status                status;

    status = vrf_stub_->VrfUpdate(&context, req_msg, &rsp_msg);

    return status;
#endif
}

//-----------------------------------------------------------------------------
// Vrf Get to HAL
//-----------------------------------------------------------------------------
Status
HalGRPCClient::vrf_get (VrfGetRequestMsg& req_msg,
                        VrfGetResponseMsg& rsp_msg)
{
    printf("Hal Mock: Vrf Get\n");
    return Status::OK;
#if 0
    grpc::ClientContext         context;
    grpc::Status                status;

    status = vrf_stub_->VrfGet(&context, req_msg, &rsp_msg);

    return status;
#endif
}

// HalL2Segment Calls
Status
HalGRPCClient::l2segment_create(L2SegmentRequestMsg& req_msg,
                                L2SegmentResponseMsg& rsp_msg)
{
    auto request = req_msg.request(0);
    printf("Hal Mock: L2Segment Create for 0x%lx\n",
           request.key_or_handle().segment_id());
    auto response = rsp_msg.add_response();
    response->set_api_status(types::API_STATUS_OK);
    return Status::OK;
}
Status
HalGRPCClient::l2segment_delete(L2SegmentDeleteRequestMsg& req_msg,
                               L2SegmentDeleteResponseMsg& rsp_msg)
{
    auto response = rsp_msg.add_response();
    auto request = req_msg.request(0);
    printf("Hal Mock: L2Segment Delete for 0x%lx\n",
           request.key_or_handle().segment_id());
    response->set_api_status(types::API_STATUS_OK);
    return Status::OK;
}
// HAL_CREATE_API(l2segment, L2Segment, l2segment);
HAL_UPDATE_API(l2segment, L2Segment, l2segment);
// HAL_DELETE_API(l2segment, L2Segment, l2segment);
HAL_GET_API(l2segment, L2Segment, l2segment);

// HalEndpoint Calls
Status
HalGRPCClient::endpoint_create(EndpointRequestMsg& req_msg,
                                EndpointResponseMsg& rsp_msg)
{
    auto response = rsp_msg.add_response();
    auto request = req_msg.request(0);
    uint64_t seg_id = request.key_or_handle().endpoint_key().l2_key().l2segment_key_handle().segment_id();
    uint64_t mac = request.key_or_handle().endpoint_key().l2_key().mac_address();
    printf("Hal Mock: EP Create for segid: 0x%lx and mac: 0x%lx\n", seg_id, mac);
    if ((seg_id == 4 && mac == 0x000102030407) ||
        (seg_id == 1 && mac == 0x20203040a) ||
        (seg_id == 1 && mac == 0x1015e010106)) {
        printf("Hal Mock: Returning error for segid: 0x%lx and mac: 0x%lx\n",
               seg_id, mac);
        response->set_api_status(types::API_STATUS_ERR);
        goto end;
    }
    response->set_api_status(types::API_STATUS_OK);
end:
    return Status::OK;
}
Status
HalGRPCClient::endpoint_delete(EndpointDeleteRequestMsg& req_msg,
                               EndpointDeleteResponseMsg& rsp_msg)
{
    auto response = rsp_msg.add_response();
    auto request = req_msg.request(0);
    printf("Hal Mock: EP Delete for vlan: 0x%lx and mac: 0x%lx\n",
           request.key_or_handle().endpoint_key().l2_key().l2segment_key_handle().segment_id(),
           request.key_or_handle().endpoint_key().l2_key().mac_address());
#if 0
    if (request.key_or_handle().endpoint_key().l2_key().l2segment_key_handle().segment_id() == 4 &&
        request.key_or_handle().endpoint_key().l2_key().mac_address() == 0x000102030407) {
        printf("Hal Mock: Returning error for vlan: 0x%lx and mac: 0x%lx\n",
               request.key_or_handle().endpoint_key().l2_key().l2segment_key_handle().segment_id(),
               request.key_or_handle().endpoint_key().l2_key().mac_address());
        response->set_api_status(types::API_STATUS_ERR);
        goto end;
    }
#endif
    response->set_api_status(types::API_STATUS_OK);
    return Status::OK;
}
// HAL_CREATE_API(endpoint, Endpoint, endpoint);
// HAL_DELETE_API(endpoint, Endpoint, endpoint);
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
    return Status::OK;
#if 0
    grpc::ClientContext         context;
    grpc::Status                status;

    status = endpoint_stub_->EndpointUpdate(&context, req_msg, &rsp_msg);

    return status;
#endif
}

// Interface Calls
Status
HalGRPCClient::interface_create(InterfaceRequestMsg& req_msg,
                                InterfaceResponseMsg& rsp_msg)
{
    printf("Hal Mock: Interface Create\n");
    auto response = rsp_msg.add_response();
    response->set_api_status(types::API_STATUS_OK);
    return Status::OK;
}

Status
HalGRPCClient::interface_update(InterfaceRequestMsg& req_msg,
                                InterfaceResponseMsg& rsp_msg)
{
    printf("Hal Mock: Interface Update\n");
    auto response = rsp_msg.add_response();
    response->set_api_status(types::API_STATUS_OK);
    return Status::OK;
}

// HAL_CREATE_API(interface, Interface, interface);
// HAL_UPDATE_API(interface, Interface, interface);
HAL_DELETE_API(interface, Interface, interface);
HAL_GET_API(interface, Interface, interface);

// Lif Calls
Status
HalGRPCClient::lif_create(LifRequestMsg& req_msg,
                                LifResponseMsg& rsp_msg)
{
    printf("Hal Mock: Lif Create\n");
    auto response = rsp_msg.add_response();
    response->set_api_status(types::API_STATUS_OK);
    return Status::OK;
}
// HAL_CREATE_API(lif, Lif, interface);
HAL_UPDATE_API(lif, Lif, interface);
HAL_DELETE_API(lif, Lif, interface);
HAL_GET_API(lif, Lif, interface);

// HalMulticast Calls
Status
HalGRPCClient::multicast_create(MulticastEntryRequestMsg& req_msg,
                                MulticastEntryResponseMsg& rsp_msg)
{
    auto response = rsp_msg.add_response();
    auto request = req_msg.request(0);
    uint64_t seg_id = request.key_or_handle().key().l2segment_key_handle().segment_id();
    uint64_t mac = request.key_or_handle().key().mac().group();
    printf("Hal Mock: Mcast Create for segid: 0x%lx and mac: 0x%lx\n", seg_id, mac);
    if ((seg_id == 7 && mac == 0x01005E010103) ||
        (seg_id == 1 && mac == 0x1015e010106)) {
        printf("Hal Mock: Mcast Create error. Returning error for segid: 0x%lx and mac: 0x%lx\n",
               seg_id, mac);
        response->set_api_status(types::API_STATUS_ERR);
        goto end;
    }
    response->set_api_status(types::API_STATUS_OK);
end:
    return Status::OK;
}
Status
HalGRPCClient::multicast_delete(MulticastEntryDeleteRequestMsg& req_msg,
                                MulticastEntryDeleteResponseMsg& rsp_msg)
{
    auto response = rsp_msg.add_response();
    auto request = req_msg.request(0);
    printf("Hal Mock: Mcast Delete for vlan: 0x%lx and mac: 0x%lx\n",
           request.key_or_handle().key().l2segment_key_handle().segment_id(),
           request.key_or_handle().key().mac().group());
    response->set_api_status(types::API_STATUS_OK);
    return Status::OK;
}
Status
HalGRPCClient::multicast_update(MulticastEntryRequestMsg& req_msg,
                                MulticastEntryResponseMsg& rsp_msg)
{
    auto response = rsp_msg.add_response();
    auto request = req_msg.request(0);
    printf("Hal Mock: Mcast Update for vlan: 0x%lx and mac: 0x%lx\n",
           request.key_or_handle().key().l2segment_key_handle().segment_id(),
           request.key_or_handle().key().mac().group());
    response->set_api_status(types::API_STATUS_OK);
    return Status::OK;
}
// HAL_CREATE_API(multicast, MulticastEntry, multicast);
// HAL_UPDATE_API(multicast, MulticastEntry, multicast);
// HAL_DELETE_API(multicast, MulticastEntry, multicast);
HAL_GET_API(multicast, MulticastEntry, multicast);

// Filter Calls
HAL_CREATE_API(filter, Filter, endpoint);
HAL_DELETE_API(filter, Filter, endpoint);
HAL_GET_API(filter, Filter, endpoint);

// Qos Calls
HAL_GET_API(qos_class, QosClass, qos);

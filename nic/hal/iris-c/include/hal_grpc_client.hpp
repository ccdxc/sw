//-----------------------------------------------------------------------------
// - hal_client
//      - Communicates with HAL
//-----------------------------------------------------------------------------
#ifndef __HAL_GRPC_CLIENT_HPP__
#define __HAL_GRPC_CLIENT_HPP__

#include <map>
#include <grpc++/grpc++.h>

#include "gen/proto/vrf.grpc.pb.h"
#include "gen/proto/interface.grpc.pb.h"
#include "gen/proto/endpoint.grpc.pb.h"
#include "gen/proto/l2segment.grpc.pb.h"
#include "gen/proto/multicast.grpc.pb.h"
#include "gen/proto/rdma.grpc.pb.h"
#include "nic/hal/iris-c/include/hal_client.hpp"


#define CREATE_API(obj_api, obj_class)                                      \
    Status obj_api ## _create(obj_class ## RequestMsg& req_msg,             \
                              obj_class ## ResponseMsg& rsp_msg)

#define UPDATE_API(obj_api, obj_class)                                      \
    Status obj_api ## _update(obj_class ## RequestMsg& req_msg,             \
                              obj_class ## ResponseMsg& rsp_msg)

#define DELETE_API(obj_api, obj_class)                                      \
    Status obj_api ## _delete(obj_class ## DeleteRequestMsg& req_msg,       \
                              obj_class ## DeleteResponseMsg& rsp_msg)

#define GET_API(obj_api, obj_class)                                         \
    Status obj_api ## _get(obj_class ## GetRequestMsg& req_msg,             \
                           obj_class ## GetResponseMsg& rsp_msg)

class HalGRPCClient: public HalClient
{
public:

    // static std::shared_ptr<HalClient> GetInstance();
    // static std::shared_ptr<HalClient> GetInstance(enum ForwardingMode mode);
    static HalClient *Factory(enum ForwardingMode mode);
    static void Destroy(HalClient *hal_client);

    // Vrf APIs
    Status vrf_create(VrfRequestMsg& req_msg,
                      VrfResponseMsg& rsp_msg);
    Status vrf_update(VrfRequestMsg& req_msg,
                      VrfResponseMsg& rsp_msg);
    Status vrf_delete(VrfDeleteRequestMsg &req_msg,
                      VrfDeleteResponseMsg &rsp_msg);
    Status vrf_get(VrfGetRequestMsg &req_msg,
                   VrfGetResponseMsg &rsp_msg);

    // L2Segment APIs
    CREATE_API(l2segment, L2Segment);
    UPDATE_API(l2segment, L2Segment);
    DELETE_API(l2segment, L2Segment);
    GET_API(l2segment, L2Segment);

    // EP APIs
    CREATE_API(endpoint, Endpoint);
    UPDATE_API(endpoint, EndpointUpdate);
    DELETE_API(endpoint, Endpoint);
    GET_API(endpoint, Endpoint);

    // Interface APIs
    CREATE_API(interface, Interface);
    UPDATE_API(interface, Interface);
    DELETE_API(interface, Interface);
    GET_API(interface, Interface);

    // Lif APIS
    CREATE_API(lif, Lif);
    UPDATE_API(lif, Lif);
    DELETE_API(lif, Lif);
    GET_API(lif, Lif);

    // MulticastEntry APIs
    CREATE_API(multicast, MulticastEntry);
    UPDATE_API(multicast, MulticastEntry);
    DELETE_API(multicast, MulticastEntry);
    GET_API(multicast, MulticastEntry);

    // Filter APIs
    CREATE_API(filter, Filter);
    DELETE_API(filter, Filter);
    GET_API(filter, Filter);

private:
    HalGRPCClient() {};
    HalGRPCClient(enum ForwardingMode mode);
    ~HalGRPCClient() {};

    // API stubs
    std::unique_ptr<vrf::Vrf::Stub> vrf_stub_;
    std::unique_ptr<intf::Interface::Stub> interface_stub_;
    std::unique_ptr<endpoint::Endpoint::Stub> endpoint_stub_;
    std::unique_ptr<l2segment::L2Segment::Stub> l2segment_stub_;
    std::unique_ptr<multicast::Multicast::Stub> multicast_stub_;
    std::unique_ptr<rdma::Rdma::Stub> rdma_stub_;

    std::shared_ptr<grpc::Channel> channel;
};

#endif /* __HAL_GRPC_CLIENT_HPP__ */

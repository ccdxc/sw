//-----------------------------------------------------------------------------
// - hal_client
//      - Communicates with HAL
//-----------------------------------------------------------------------------
#ifndef __HAL_COMMON_CLIENT_HPP__
#define __HAL_COMMON_CLIENT_HPP__

#include <map>
#include <grpc++/grpc++.h>

#include "gen/proto/vrf.grpc.pb.h"
#include "gen/proto/interface.grpc.pb.h"
#include "gen/proto/endpoint.grpc.pb.h"
#include "gen/proto/l2segment.grpc.pb.h"
#include "gen/proto/multicast.grpc.pb.h"
#include "gen/proto/rdma.grpc.pb.h"
#include "gen/proto/qos.grpc.pb.h"

#include "platform/src/lib/nicmgr/include/logger.hpp"

using grpc::Status;
using vrf::VrfRequestMsg;
using vrf::VrfResponseMsg;
using vrf::VrfDeleteRequestMsg;
using vrf::VrfDeleteResponseMsg;
using vrf::VrfGetRequestMsg;
using vrf::VrfGetResponseMsg;
using endpoint::EndpointRequestMsg;
using endpoint::EndpointResponseMsg;
using endpoint::EndpointUpdateRequestMsg;
using endpoint::EndpointUpdateResponseMsg;
using endpoint::EndpointDeleteRequestMsg;
using endpoint::EndpointDeleteResponseMsg;
using endpoint::EndpointGetRequestMsg;
using endpoint::EndpointGetResponseMsg;
using endpoint::FilterRequestMsg;
using endpoint::FilterResponseMsg;
using endpoint::FilterDeleteRequestMsg;
using endpoint::FilterDeleteResponseMsg;
using endpoint::FilterGetRequestMsg;
using endpoint::FilterGetResponseMsg;
using l2segment::L2SegmentRequestMsg;
using l2segment::L2SegmentResponseMsg;
using l2segment::L2SegmentDeleteRequestMsg;
using l2segment::L2SegmentDeleteResponseMsg;
using l2segment::L2SegmentGetRequestMsg;
using l2segment::L2SegmentGetResponseMsg;
using intf::InterfaceRequestMsg;
using intf::InterfaceResponseMsg;
using intf::InterfaceDeleteRequestMsg;
using intf::InterfaceDeleteResponseMsg;
using intf::InterfaceGetRequestMsg;
using intf::InterfaceGetResponseMsg;
using intf::InterfaceGetRequest;
using intf::InterfaceGetResponse;
using intf::InterfaceSpec;
using intf::InterfaceResponse;
using intf::LifRequestMsg;
using intf::LifResponseMsg;
using intf::LifDeleteRequestMsg;
using intf::LifDeleteResponseMsg;
using intf::LifGetRequestMsg;
using intf::LifGetResponseMsg;
using intf::LifQStateMapEntry;
using intf::QStateSetReq;
using multicast::MulticastEntryRequestMsg;
using multicast::MulticastEntryResponseMsg;
using multicast::MulticastEntryDeleteRequestMsg;
using multicast::MulticastEntryDeleteResponseMsg;
using multicast::MulticastEntryGetRequestMsg;
using multicast::MulticastEntryGetResponseMsg;
using qos::QosClassGetRequestMsg;
using qos::QosClassGetResponseMsg;

/**
 * Forwarding Modes
 */
enum HalForwardingMode
{
    FWD_MODE_CLASSIC,
    FWD_MODE_SMART,
};

#if 0
#define API_OPER(ENTRY)                             \
    ENTRY(API_OPER_CREATE,    0, "Create")          \
    ENTRY(API_OPER_RETRIEVE,  1, "Retrieve")        \
    ENTRY(API_OPER_UPDATE,    2, "Update")          \
    ENTRY(API_OPER_DELETE,    3, "Delete")
DEFINE_ENUM(api_oper_t, API_OPER)
#undef API_OPER
#endif

class HalCommonClient
{
public:
    // throws an exception if channel is not created
    // static std::shared_ptr<HalCommonClient> GetInstance();

    // call this first to set the mode and create the channel
    // static std::shared_ptr<HalCommonClient> GetInstance(enum HalForwardingMode mode){};
    static HalCommonClient *Factory(enum HalForwardingMode mode) {return NULL;};
    static void Destroy(HalCommonClient *hal_client);

    static HalCommonClient *GetInstance();

    enum HalForwardingMode GetMode();

    // TODO: Move these to private.
    // std::unique_ptr<multicast::HalMulticast::Stub> multicast_stub_;
    // std::unique_ptr<rdma::Rdma::Stub> rdma_stub_;

    // Vrf APIS
    virtual Status vrf_create(VrfRequestMsg& req_msg,
                              VrfResponseMsg& rsp_msg) = 0;
    virtual Status vrf_update(VrfRequestMsg& req_msg,
                              VrfResponseMsg& rsp_msg) = 0;
    virtual Status vrf_delete(VrfDeleteRequestMsg &req_msg,
                              VrfDeleteResponseMsg &rsp_msg) = 0;
    virtual Status vrf_get(VrfGetRequestMsg &req_msg,
                           VrfGetResponseMsg &rsp_msg) = 0;

    // HalL2Segment APIs
    virtual Status l2segment_create(L2SegmentRequestMsg& req_msg,
                                    L2SegmentResponseMsg& rsp_msg) = 0;
    virtual Status l2segment_update(L2SegmentRequestMsg& req_msg,
                                    L2SegmentResponseMsg& rsp_msg) = 0;
    virtual Status l2segment_delete(L2SegmentDeleteRequestMsg &req_msg,
                                    L2SegmentDeleteResponseMsg &rsp_msg) = 0;
    virtual Status l2segment_get(L2SegmentGetRequestMsg &req_msg,
                                 L2SegmentGetResponseMsg &rsp_msg) = 0;

    // EP APIs
    virtual Status endpoint_create(EndpointRequestMsg& req_msg,
                                   EndpointResponseMsg& rsp_msg) = 0;
    virtual Status endpoint_update(EndpointUpdateRequestMsg& req_msg,
                                   EndpointUpdateResponseMsg& rsp_msg) = 0;
    virtual Status endpoint_delete(EndpointDeleteRequestMsg &req_msg,
                                   EndpointDeleteResponseMsg &rsp_msg) = 0;
    virtual Status endpoint_get(EndpointGetRequestMsg &req_msg,
                                EndpointGetResponseMsg &rsp_msg) = 0;


    // Interface APIs
    virtual Status interface_create(InterfaceRequestMsg& req_msg,
                                    InterfaceResponseMsg& rsp_msg) = 0;
    virtual Status interface_update(InterfaceRequestMsg& req_msg,
                                    InterfaceResponseMsg& rsp_msg) = 0;
    virtual Status interface_delete(InterfaceDeleteRequestMsg &req_msg,
                                    InterfaceDeleteResponseMsg &rsp_msg) = 0;
    virtual Status interface_get(InterfaceGetRequestMsg &req_msg,
                                 InterfaceGetResponseMsg &rsp_msg) = 0;

    // Lif APIS
    virtual Status lif_create(LifRequestMsg& req_msg,
                              LifResponseMsg& rsp_msg) = 0;
    virtual Status lif_update(LifRequestMsg& req_msg,
                              LifResponseMsg& rsp_msg) = 0;
    virtual Status lif_delete(LifDeleteRequestMsg &req_msg,
                              LifDeleteResponseMsg &rsp_msg) = 0;
    virtual Status lif_get(LifGetRequestMsg &req_msg,
                           LifGetResponseMsg &rsp_msg) = 0;


    // MulticastEntry APIs
    virtual Status multicast_create(MulticastEntryRequestMsg& req_msg,
                                    MulticastEntryResponseMsg& rsp_msg) = 0;
    virtual Status multicast_update(MulticastEntryRequestMsg& req_msg,
                                    MulticastEntryResponseMsg& rsp_msg) = 0;
    virtual Status multicast_delete(MulticastEntryDeleteRequestMsg &req_msg,
                                    MulticastEntryDeleteResponseMsg &rsp_msg) = 0;
    virtual Status multicast_get(MulticastEntryGetRequestMsg &req_msg,
                                 MulticastEntryGetResponseMsg &rsp_msg) = 0;

    // Filter APIs
    virtual Status filter_create(FilterRequestMsg& req_msg,
                                 FilterResponseMsg& rsp_msg) = 0;
#if 0
    virtual Status filter_update(FilterRequestMsg& req_msg,
                                 FilterResponseMsg& rsp_msg) = 0;
#endif
    virtual Status filter_delete(FilterDeleteRequestMsg &req_msg,
                                 FilterDeleteResponseMsg &rsp_msg) = 0;
    virtual Status filter_get(FilterGetRequestMsg &req_msg,
                              FilterGetResponseMsg &rsp_msg) = 0;

    virtual Status qos_class_get(QosClassGetRequestMsg &req_msg,
                                 QosClassGetResponseMsg &rsp_msg) = 0;

    // Singleton
    HalCommonClient();
    HalCommonClient(enum HalForwardingMode mode);
    virtual ~HalCommonClient() {};

    enum HalForwardingMode mode;
    static HalCommonClient *instance;

private:


    // non-copyable
    // HalCommonClient(HalCommonClient const&);
    // void operator=(HalCommonClient const&);

};

#endif /* __HAL_COMMON_CLIENT_HPP__ */

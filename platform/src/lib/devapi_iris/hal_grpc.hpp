//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __HAL_GRPC_HPP__
#define __HAL_GRPC_HPP__

#include <map>
#include <grpc++/grpc++.h>
#include "gen/proto/vrf.grpc.pb.h"
#include "gen/proto/interface.grpc.pb.h"
#include "gen/proto/endpoint.grpc.pb.h"
#include "gen/proto/l2segment.grpc.pb.h"
#include "gen/proto/multicast.grpc.pb.h"
#ifdef __x86_64__
#include "gen/proto/rdma.grpc.pb.h"
#endif
#include "gen/proto/qos.grpc.pb.h"
#include "gen/proto/port.grpc.pb.h"
#include "gen/proto/internal.grpc.pb.h"
#include "gen/proto/accel_rgroup.grpc.pb.h"
#include "gen/proto/system.grpc.pb.h"
#include "gen/proto/device.grpc.pb.h"
#include "hal_grpc.hpp"
#include "include/sdk/base.hpp"
#include "devapi_iris_types.hpp"

using grpc::Status;
using vrf::VrfRequestMsg;
using vrf::VrfResponseMsg;
using vrf::VrfDeleteRequestMsg;
using vrf::VrfDeleteResponseMsg;
using vrf::VrfGetRequestMsg;
using vrf::VrfGetResponseMsg;
using endpoint::EndpointSpec;
using endpoint::EndpointRequestMsg;
using endpoint::EndpointResponseMsg;
using endpoint::EndpointResponse;
using endpoint::EndpointUpdateRequestMsg;
using endpoint::EndpointUpdateResponseMsg;
using endpoint::EndpointDeleteRequestMsg;
using endpoint::EndpointDeleteResponseMsg;
using endpoint::EndpointDeleteRequest;
using endpoint::EndpointDeleteResponse;
using endpoint::EndpointGetRequestMsg;
using endpoint::EndpointGetResponseMsg;
using endpoint::FilterRequestMsg;
using endpoint::FilterResponseMsg;
using endpoint::FilterDeleteRequestMsg;
using endpoint::FilterDeleteResponseMsg;
using endpoint::FilterGetRequestMsg;
using endpoint::FilterGetResponseMsg;
using endpoint::FilterSpec;
using endpoint::FilterResponse;
using endpoint::FilterDeleteRequest;
using endpoint::FilterDeleteResponse;
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
using intf::InterfaceDeleteRequest;
using intf::InterfaceDeleteResponse;
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
using intf::LifSpec;
using intf::LifResponse;
using intf::LifDeleteRequest;
using intf::LifDeleteResponse;
using intf::QStateSetReq;
using port::PortGetRequestMsg;
using port::PortGetResponseMsg;
using port::PortGetResponse;
using port::PortRequestMsg;
using port::PortResponseMsg;
using port::PortSpec;
using port::PortResponse;
using multicast::MulticastEntryRequestMsg;
using multicast::MulticastEntryResponseMsg;
using multicast::MulticastEntryDeleteRequestMsg;
using multicast::MulticastEntryDeleteResponseMsg;
using multicast::MulticastEntryGetRequestMsg;
using multicast::MulticastEntryGetResponseMsg;
using multicast::MulticastEntrySpec;
using multicast::MulticastEntryResponse;
using multicast::MulticastEntryDeleteRequest;
using multicast::MulticastEntryDeleteResponse;
using qos::QosClassGetRequestMsg;
using qos::QosClassGetResponseMsg;
using qos::QosClassRequestMsg;
using qos::QosClassResponseMsg;
using qos::QosClassDeleteRequestMsg;
using qos::QosClassDeleteResponseMsg;
using qos::QosClassSetGlobalPauseTypeRequestMsg;
using qos::QosClassSetGlobalPauseTypeResponseMsg;
using accelRGroup::AccelRGroupAddRequestMsg;
using accelRGroup::AccelRGroupAddResponseMsg;
using accelRGroup::AccelRGroupDelRequestMsg;
using accelRGroup::AccelRGroupDelResponseMsg;
using accelRGroup::AccelRGroupRingAddRequestMsg;
using accelRGroup::AccelRGroupRingAddResponseMsg;
using accelRGroup::AccelRGroupRingDelRequestMsg;
using accelRGroup::AccelRGroupRingDelResponseMsg;
using accelRGroup::AccelRGroupResetSetRequestMsg;
using accelRGroup::AccelRGroupResetSetResponseMsg;
using accelRGroup::AccelRGroupEnableSetRequestMsg;
using accelRGroup::AccelRGroupEnableSetResponseMsg;
using accelRGroup::AccelRGroupPndxSetRequestMsg;
using accelRGroup::AccelRGroupPndxSetResponseMsg;
using accelRGroup::AccelRGroupInfoGetRequestMsg;
using accelRGroup::AccelRGroupInfoGetResponseMsg;
using accelRGroup::AccelRGroupIndicesGetRequestMsg;
using accelRGroup::AccelRGroupIndicesGetResponseMsg;
using accelRGroup::AccelRGroupMetricsGetRequestMsg;
using accelRGroup::AccelRGroupMetricsGetResponseMsg;
using accelRGroup::AccelRGroupMiscGetRequestMsg;
using accelRGroup::AccelRGroupMiscGetResponseMsg;
using internal::CryptoKeyCreateWithIdRequestMsg;
using internal::CryptoKeyCreateWithIdResponseMsg;
using internal::CryptoKeyUpdateRequestMsg;
using internal::CryptoKeyUpdateResponseMsg;
using sys::MicroSegRequestMsg;
using sys::MicroSegResponseMsg;
using sys::MicroSegSpec;
using sys::MicroSegResponse;
using sys::SysSpecGetRequest;
using sys::SysSpecGetResponse;
using std::chrono::seconds;

namespace iris {

#define HAL_GRPC_API_TIMEOUT 25             /* Secs */
#define HAL_GRPC_CLASSIC_API_TIMEOUT 2      /* Secs */

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

class hal_grpc {
private:
    static hal_grpc *hal_;
    bool micro_seg_en_;
    // API stubs
    std::unique_ptr<vrf::Vrf::Stub> vrf_stub_;
    std::unique_ptr<intf::Interface::Stub> interface_stub_;
    std::unique_ptr<endpoint::Endpoint::Stub> endpoint_stub_;
    std::unique_ptr<l2segment::L2Segment::Stub> l2segment_stub_;
    std::unique_ptr<multicast::Multicast::Stub> multicast_stub_;
#ifdef __x86_64__
    std::unique_ptr<rdma::Rdma::Stub> rdma_stub_;
#endif
    std::unique_ptr<qos::QOS::Stub> qos_stub_;
    std::unique_ptr<port::Port::Stub> port_stub_;
    std::unique_ptr<internal::Internal::Stub> crypto_stub_;
    std::unique_ptr<accelRGroup::AccelRGroup::Stub> accel_rgroup_stub_;
    std::unique_ptr<sys::System::Stub> sys_stub_;
    std::shared_ptr<grpc::Channel> channel;
private:
    sdk_ret_t init_(void);
    hal_grpc() {}
    ~hal_grpc() {}

public:
    static hal_grpc *factory(void);
    static void destroy(hal_grpc *hal);
    static hal_grpc *get_hal_grpc(void);

    // Make GRPC connection to HAL
    sdk_ret_t connect_hal(void);

    void set_micro_seg_en(bool en) { micro_seg_en_ = en; }
    bool get_micro_seg_en(void) { return micro_seg_en_; }

    // System APIs
    Status micro_seg_update(MicroSegRequestMsg& req_msg,
                            MicroSegResponseMsg& rsp_msg);
    Status sys_spec_get(SysSpecGetRequest& req,
                        SysSpecGetResponse& rsp);

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

    // HalMulticastEntry APIs
    CREATE_API(multicast, MulticastEntry);
    UPDATE_API(multicast, MulticastEntry);
    DELETE_API(multicast, MulticastEntry);
    GET_API(multicast, MulticastEntry);

    // Filter APIs
    CREATE_API(filter, Filter);
    DELETE_API(filter, Filter);
    GET_API(filter, Filter);

    // QoS APIs
    Status qos_class_create(QosClassRequestMsg& req_msg,
                            QosClassResponseMsg& rsp_msg);
    Status qos_class_delete(QosClassDeleteRequestMsg& req_msg,
                            QosClassDeleteResponseMsg& rsp_msg);
    Status qos_class_set_global_pause_type(
                                QosClassSetGlobalPauseTypeRequestMsg& req_msg,
                                QosClassSetGlobalPauseTypeResponseMsg& rsp_msg);
    GET_API(qos_class, QosClass);

    // Port APIs
    UPDATE_API(port, Port);
    GET_API(port, Port);

    // Accel APIs
    Status accel_rgroup_add(AccelRGroupAddRequestMsg& req_msg,
                            AccelRGroupAddResponseMsg& rsp_msg);
    Status accel_rgroup_del(AccelRGroupDelRequestMsg& req_msg,
                            AccelRGroupDelResponseMsg& rsp_msg);
    Status accel_rgroup_ring_add(AccelRGroupRingAddRequestMsg& req_msg,
                                 AccelRGroupRingAddResponseMsg& rsp_msg);
    Status accel_rgroup_ring_del(AccelRGroupRingDelRequestMsg& req_msg,
                                 AccelRGroupRingDelResponseMsg& rsp_msg);
    Status accel_rgroup_reset_set(AccelRGroupResetSetRequestMsg& req_msg,
                                  AccelRGroupResetSetResponseMsg& rsp_msg);
    Status accel_rgroup_enable_set(AccelRGroupEnableSetRequestMsg& req_msg,
                                   AccelRGroupEnableSetResponseMsg& rsp_msg);
    Status accel_rgroup_pndx_set(AccelRGroupPndxSetRequestMsg& req_msg,
                                 AccelRGroupPndxSetResponseMsg& rsp_msg);
    Status accel_rgroup_info_get(AccelRGroupInfoGetRequestMsg& req_msg,
                                 AccelRGroupInfoGetResponseMsg& rsp_msg);
    Status accel_rgroup_indices_get(AccelRGroupIndicesGetRequestMsg& req_msg,
                                    AccelRGroupIndicesGetResponseMsg& rsp_msg);
    Status accel_rgroup_metrics_get(AccelRGroupMetricsGetRequestMsg& req_msg,
                                    AccelRGroupMetricsGetResponseMsg& rsp_msg);
    Status accel_rgroup_misc_get(AccelRGroupMiscGetRequestMsg& req_msg,
                                 AccelRGroupMiscGetResponseMsg& rsp_msg);
    Status crypto_create(CryptoKeyCreateWithIdRequestMsg& req_msg,
                         CryptoKeyCreateWithIdResponseMsg& rsp_msg);
    Status crypto_update(CryptoKeyUpdateRequestMsg& req_msg,
                         CryptoKeyUpdateResponseMsg& rsp_msg);

};

}    // namespace iris

using iris::hal_grpc;
#endif /* __HAL_GRPC_HPP__ */

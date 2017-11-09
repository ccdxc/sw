#ifndef __VRF_SVC_HPP__
#define __VRF_SVC_HPP__

#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/key_handles.pb.h"
#include "nic/gen/proto/hal/vrf.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using vrf::Vrf;
using vrf::VrfSpec;
using vrf::VrfStatus;
using vrf::VrfResponse;
using key_handles::VrfKeyHandle;
using vrf::VrfRequestMsg;
using vrf::VrfResponseMsg;
using vrf::VrfDeleteRequest;
using vrf::VrfDeleteRequestMsg;
using vrf::VrfDeleteResponseMsg;
using vrf::VrfGetRequest;
using vrf::VrfGetRequestMsg;
using vrf::VrfGetResponse;
using vrf::VrfGetResponseMsg;

class VrfServiceImpl final : public Vrf::Service {
public:
    Status VrfCreate(ServerContext *context,
                        const VrfRequestMsg *req,
                        VrfResponseMsg *rsp) override;

    Status VrfUpdate(ServerContext *context,
                        const VrfRequestMsg *req,
                        VrfResponseMsg *rsp) override;


    Status VrfDelete(ServerContext *context,
                        const VrfDeleteRequestMsg *req,
                        VrfDeleteResponseMsg *rsp) override;

    Status VrfGet(ServerContext *context,
                     const VrfGetRequestMsg *req,
                     VrfGetResponseMsg *rsp) override;
};
#endif    // __VRF_SVC_HPP__


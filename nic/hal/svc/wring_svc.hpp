#ifndef __WRING_SVC_HPP__
#define __WRING_SVC_HPP__

#include <base.h>
#include <grpc++/grpc++.h>
#include <types.pb.h>
#include <wring.grpc.pb.h>

using grpc::ServerContext;
using grpc::Status;

using wring::WRing;
using wring::WRingSpec;
using wring::WRingStatus;
using wring::WRingResponse;
using wring::WRingKeyHandle;
using wring::WRingRequestMsg;
using wring::WRingResponseMsg;
using wring::WRingDeleteRequestMsg;
using wring::WRingDeleteResponseMsg;
using wring::WRingGetRequest;
using wring::WRingGetRequestMsg;
using wring::WRingGetResponse;
using wring::WRingGetResponseMsg;

class WRingServiceImpl final : public WRing::Service {
public:
    Status WRingCreate(ServerContext *context,
                       const WRingRequestMsg *req,
                       WRingResponseMsg *rsp) override;

    Status WRingUpdate(ServerContext *context,
                       const WRingRequestMsg *req,
                       WRingResponseMsg *rsp) override;


    Status WRingDelete(ServerContext *context,
                       const WRingDeleteRequestMsg *req,
                       WRingDeleteResponseMsg *rsp) override;

    Status WRingGet(ServerContext *context,
                    const WRingGetRequestMsg *req,
                    WRingGetResponseMsg *rsp) override;
};

#endif    // __WRING_SVC_HPP__


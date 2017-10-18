#ifndef __WRING_SVC_HPP__
#define __WRING_SVC_HPP__

#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/wring.grpc.pb.h"

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
using wring::WRingGetEntriesRequest;
using wring::WRingGetEntriesRequestMsg;
using wring::WRingGetEntriesResponse;
using wring::WRingGetEntriesResponseMsg;
using wring::WRingGetMetaResponseMsg;
using wring::WRingGetMetaResponse;

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

    Status WRingGetEntries(ServerContext *context,
                    const WRingGetEntriesRequestMsg *req,
                    WRingGetEntriesResponseMsg *rsp) override;

    Status WRingGetMeta(ServerContext *context,
                    const WRingRequestMsg *req,
                    WRingGetMetaResponseMsg *rsp) override;
};

#endif    // __WRING_SVC_HPP__


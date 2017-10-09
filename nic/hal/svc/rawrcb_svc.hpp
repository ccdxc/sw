#ifndef __RAWRCB_SVC_HPP__
#define __RAWRCB_SVC_HPP__

#include "nic/include/base.h"
#include <grpc++/grpc++.h>
#include "nic/proto/types.pb.h"
#include "nic/proto/hal/rawrcb.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using rawrcb::RawrCb;
using rawrcb::RawrCbSpec;
using rawrcb::RawrCbStatus;
using rawrcb::RawrCbResponse;
using rawrcb::RawrCbKeyHandle;
using rawrcb::RawrCbRequestMsg;
using rawrcb::RawrCbResponseMsg;
using rawrcb::RawrCbDeleteRequestMsg;
using rawrcb::RawrCbDeleteResponseMsg;
using rawrcb::RawrCbGetRequest;
using rawrcb::RawrCbGetRequestMsg;
using rawrcb::RawrCbGetResponse;
using rawrcb::RawrCbGetResponseMsg;

class RawrCbServiceImpl final : public RawrCb::Service {
public:
    Status RawrCbCreate(ServerContext *context,
                        const RawrCbRequestMsg *req,
                        RawrCbResponseMsg *rsp) override;

    Status RawrCbUpdate(ServerContext *context,
                        const RawrCbRequestMsg *req,
                        RawrCbResponseMsg *rsp) override;


    Status RawrCbDelete(ServerContext *context,
                        const RawrCbDeleteRequestMsg *req,
                        RawrCbDeleteResponseMsg *rsp) override;

    Status RawrCbGet(ServerContext *context,
                     const RawrCbGetRequestMsg *req,
                     RawrCbGetResponseMsg *rsp) override;
};

#endif    // __RAWRCB_SVC_HPP__


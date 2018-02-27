#ifndef __RAWCCB_SVC_HPP__
#define __RAWCCB_SVC_HPP__

#include "nic/include/base.h"
#include <grpc++/grpc++.h>
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/rawccb.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using rawccb::RawcCb;
using rawccb::RawcCbSpec;
using rawccb::RawcCbStatus;
using rawccb::RawcCbResponse;
using rawccb::RawcCbKeyHandle;
using rawccb::RawcCbRequestMsg;
using rawccb::RawcCbResponseMsg;
using rawccb::RawcCbDeleteRequestMsg;
using rawccb::RawcCbDeleteResponseMsg;
using rawccb::RawcCbGetRequest;
using rawccb::RawcCbGetRequestMsg;
using rawccb::RawcCbGetResponse;
using rawccb::RawcCbGetResponseMsg;

class RawcCbServiceImpl final : public RawcCb::Service {
public:
    Status RawcCbCreate(ServerContext *context,
                        const RawcCbRequestMsg *req,
                        RawcCbResponseMsg *rsp) override;

    Status RawcCbUpdate(ServerContext *context,
                        const RawcCbRequestMsg *req,
                        RawcCbResponseMsg *rsp) override;


    Status RawcCbDelete(ServerContext *context,
                        const RawcCbDeleteRequestMsg *req,
                        RawcCbDeleteResponseMsg *rsp) override;

    Status RawcCbGet(ServerContext *context,
                     const RawcCbGetRequestMsg *req,
                     RawcCbGetResponseMsg *rsp) override;
};

#endif    // __RAWCCB_SVC_HPP__


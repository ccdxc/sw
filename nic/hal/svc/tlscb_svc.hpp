#ifndef __TLSCB_SVC_HPP__
#define __TLSCB_SVC_HPP__

#include <base.h>
#include <grpc++/grpc++.h>
#include <types.pb.h>
#include <tls_proxy_cb.grpc.pb.h>

using grpc::ServerContext;
using grpc::Status;

using tlscb::TlsCb;
using tlscb::TlsCbSpec;
using tlscb::TlsCbStatus;
using tlscb::TlsCbResponse;
using tlscb::TlsCbKeyHandle;
using tlscb::TlsCbRequestMsg;
using tlscb::TlsCbResponseMsg;
using tlscb::TlsCbDeleteRequestMsg;
using tlscb::TlsCbDeleteResponseMsg;
using tlscb::TlsCbGetRequest;
using tlscb::TlsCbGetRequestMsg;
using tlscb::TlsCbGetResponse;
using tlscb::TlsCbGetResponseMsg;

class TlsCbServiceImpl final : public TlsCb::Service {
public:
    Status TlsCbCreate(ServerContext *context,
                       const TlsCbRequestMsg *req,
                       TlsCbResponseMsg *rsp) override;

    Status TlsCbUpdate(ServerContext *context,
                       const TlsCbRequestMsg *req,
                       TlsCbResponseMsg *rsp) override;


    Status TlsCbDelete(ServerContext *context,
                       const TlsCbDeleteRequestMsg *req,
                       TlsCbDeleteResponseMsg *rsp) override;

    Status TlsCbGet(ServerContext *context,
                    const TlsCbGetRequestMsg *req,
                    TlsCbGetResponseMsg *rsp) override;
};

#endif    // __TLSCB_SVC_HPP__


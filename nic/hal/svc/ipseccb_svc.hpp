#ifndef __IPSECCB_SVC_HPP__
#define __IPSECCB_SVC_HPP__

#include <base.h>
#include <grpc++/grpc++.h>
#include <types.pb.h>
#include <ipseccb.grpc.pb.h>

using grpc::ServerContext;
using grpc::Status;

using ipseccb::IpsecCb;
using ipseccb::IpsecCbSpec;
using ipseccb::IpsecCbStatus;
using ipseccb::IpsecCbResponse;
using ipseccb::IpsecCbKeyHandle;
using ipseccb::IpsecCbRequestMsg;
using ipseccb::IpsecCbResponseMsg;
using ipseccb::IpsecCbDeleteRequestMsg;
using ipseccb::IpsecCbDeleteResponseMsg;
using ipseccb::IpsecCbGetRequest;
using ipseccb::IpsecCbGetRequestMsg;
using ipseccb::IpsecCbGetResponse;
using ipseccb::IpsecCbGetResponseMsg;

class IpsecCbServiceImpl final : public IpsecCb::Service {
public:
    Status IpsecCbCreate(ServerContext *context,
                       const IpsecCbRequestMsg *req,
                       IpsecCbResponseMsg *rsp) override;

    Status IpsecCbUpdate(ServerContext *context,
                       const IpsecCbRequestMsg *req,
                       IpsecCbResponseMsg *rsp) override;


    Status IpsecCbDelete(ServerContext *context,
                       const IpsecCbDeleteRequestMsg *req,
                       IpsecCbDeleteResponseMsg *rsp) override;

    Status IpsecCbGet(ServerContext *context,
                    const IpsecCbGetRequestMsg *req,
                    IpsecCbGetResponseMsg *rsp) override;
};

#endif    // __IPSECCB_SVC_HPP__


#ifndef __TCPCB_SVC_HPP__
#define __TCPCB_SVC_HPP__

#include <base.h>
#include <grpc++/grpc++.h>
#include <types.pb.h>
#include <tcp_proxy_cb.grpc.pb.h>

using grpc::ServerContext;
using grpc::Status;

using tcpcb::TcpCb;
using tcpcb::TcpCbSpec;
using tcpcb::TcpCbStatus;
using tcpcb::TcpCbResponse;
using tcpcb::TcpCbKeyHandle;
using tcpcb::TcpCbRequestMsg;
using tcpcb::TcpCbResponseMsg;
using tcpcb::TcpCbDeleteRequestMsg;
using tcpcb::TcpCbDeleteResponseMsg;
using tcpcb::TcpCbGetRequest;
using tcpcb::TcpCbGetRequestMsg;
using tcpcb::TcpCbGetResponse;
using tcpcb::TcpCbGetResponseMsg;

class TcpCbServiceImpl final : public TcpCb::Service {
public:
    Status TcpCbCreate(ServerContext *context,
                       const TcpCbRequestMsg *req,
                       TcpCbResponseMsg *rsp) override;

    Status TcpCbUpdate(ServerContext *context,
                       const TcpCbRequestMsg *req,
                       TcpCbResponseMsg *rsp) override;


    Status TcpCbDelete(ServerContext *context,
                       const TcpCbDeleteRequestMsg *req,
                       TcpCbDeleteResponseMsg *rsp) override;

    Status TcpCbGet(ServerContext *context,
                    const TcpCbGetRequestMsg *req,
                    TcpCbGetResponseMsg *rsp) override;
};

#endif    // __TCPCB_SVC_HPP__


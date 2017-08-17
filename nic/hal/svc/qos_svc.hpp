#ifndef __QOS_SVC_HPP__
#define __QOS_SVC_HPP__

#include <base.h>
#include <grpc++/grpc++.h>
#include <types.pb.h>
#include <qos.grpc.pb.h>

using grpc::ServerContext;
using grpc::Status;

using qos::QOS;
using qos::BufPoolSpec;
using qos::BufPoolKeyHandle;
using qos::BufPoolRequestMsg;
using qos::BufPoolResponse;
using qos::BufPoolResponseMsg;
using qos::BufPoolDeleteRequestMsg;
using qos::BufPoolDeleteResponseMsg;
using qos::BufPoolGetRequestMsg;
using qos::BufPoolGetResponseMsg;
using qos::QueueSpec;
using qos::QueueKeyHandle;
using qos::QueueRequestMsg;
using qos::QueueResponse;
using qos::QueueResponseMsg;
using qos::QueueDeleteRequestMsg;
using qos::QueueDeleteResponseMsg;
using qos::QueueGetRequestMsg;
using qos::QueueGetResponseMsg;
using qos::PolicerSpec;
using qos::PolicerKeyHandle;
using qos::PolicerRequestMsg;
using qos::PolicerResponse;
using qos::PolicerResponseMsg;
using qos::PolicerDeleteRequestMsg;
using qos::PolicerDeleteResponseMsg;
using qos::PolicerGetRequestMsg;
using qos::PolicerGetResponseMsg;

class QOSServiceImpl final : public QOS::Service {
public:
    Status BufPoolCreate(ServerContext *context,
                         const BufPoolRequestMsg *req,
                         BufPoolResponseMsg *rsp) override;

    Status BufPoolUpdate(ServerContext *context,
                         const BufPoolRequestMsg *req,
                         BufPoolResponseMsg *rsp) override;

    Status BufPoolDelete(ServerContext *context,
                         const BufPoolDeleteRequestMsg *req,
                         BufPoolDeleteResponseMsg *rsp) override;

    Status BufPoolGet(ServerContext *context,
                      const BufPoolGetRequestMsg *req,
                      BufPoolGetResponseMsg *rsp) override;

    Status QueueCreate(ServerContext *context,
                       const QueueRequestMsg *req,
                       QueueResponseMsg *rsp) override;

    Status QueueUpdate(ServerContext *context,
                       const QueueRequestMsg *req,
                       QueueResponseMsg *rsp) override;

    Status QueueDelete(ServerContext *context,
                       const QueueDeleteRequestMsg *req,
                       QueueDeleteResponseMsg *rsp) override;

    Status QueueGet(ServerContext *context,
                    const QueueGetRequestMsg *req,
                    QueueGetResponseMsg *rsp) override;

    Status PolicerCreate(ServerContext *context,
                         const PolicerRequestMsg *req,
                         PolicerResponseMsg *rsp) override;

    Status PolicerUpdate(ServerContext *context,
                         const PolicerRequestMsg *req,
                         PolicerResponseMsg *rsp) override;

    Status PolicerDelete(ServerContext *context,
                         const PolicerDeleteRequestMsg *req,
                         PolicerDeleteResponseMsg *rsp) override;

    Status PolicerGet(ServerContext *context,
                      const PolicerGetRequestMsg *req,
                      PolicerGetResponseMsg *rsp) override;

};

#endif // __QOS_SVC_HPP__

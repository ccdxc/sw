#ifndef __QOS_SVC_HPP__
#define __QOS_SVC_HPP__

#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/qos.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using qos::QOS;
using qos::QosClassSpec;
using qos::QosClassKeyHandle;
using qos::QosClassRequestMsg;
using qos::QosClassResponse;
using qos::QosClassResponseMsg;
using qos::QosClassDeleteRequestMsg;
using qos::QosClassDeleteResponseMsg;
using qos::QosClassGetRequestMsg;
using qos::QosClassGetResponseMsg;

class QOSServiceImpl final : public QOS::Service {
public:
 Status QosClassCreate(ServerContext *context,
                       const QosClassRequestMsg *req,
                       QosClassResponseMsg *rsp) override;

 Status QosClassUpdate(ServerContext *context,
                       const QosClassRequestMsg *req,
                       QosClassResponseMsg *rsp) override;

 Status QosClassDelete(ServerContext *context,
                       const QosClassDeleteRequestMsg *req,
                       QosClassDeleteResponseMsg *rsp) override;

 Status QosClassGet(ServerContext *context,
                    const QosClassGetRequestMsg *req,
                    QosClassGetResponseMsg *rsp) override;

};

#endif // __QOS_SVC_HPP__

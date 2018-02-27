#ifndef __CPUCB_SVC_HPP__
#define __CPUCB_SVC_HPP__

#include "nic/include/base.h"
#include <grpc++/grpc++.h>
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/cpucb.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using cpucb::CpuCb;
using cpucb::CpuCbSpec;
using cpucb::CpuCbStatus;
using cpucb::CpuCbResponse;
using cpucb::CpuCbKeyHandle;
using cpucb::CpuCbRequestMsg;
using cpucb::CpuCbResponseMsg;
using cpucb::CpuCbDeleteRequestMsg;
using cpucb::CpuCbDeleteResponseMsg;
using cpucb::CpuCbGetRequest;
using cpucb::CpuCbGetRequestMsg;
using cpucb::CpuCbGetResponse;
using cpucb::CpuCbGetResponseMsg;

class CpuCbServiceImpl final : public CpuCb::Service {
public:
    Status CpuCbCreate(ServerContext *context,
                       const CpuCbRequestMsg *req,
                       CpuCbResponseMsg *rsp) override;

    Status CpuCbUpdate(ServerContext *context,
                       const CpuCbRequestMsg *req,
                       CpuCbResponseMsg *rsp) override;


    Status CpuCbDelete(ServerContext *context,
                       const CpuCbDeleteRequestMsg *req,
                       CpuCbDeleteResponseMsg *rsp) override;

    Status CpuCbGet(ServerContext *context,
                    const CpuCbGetRequestMsg *req,
                    CpuCbGetResponseMsg *rsp) override;
};

#endif    // __CPUCB_SVC_HPP__


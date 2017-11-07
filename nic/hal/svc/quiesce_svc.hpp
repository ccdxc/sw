#ifndef __QUIESCE_SVC_HPP__
#define __QUIESCE_SVC_HPP__
#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/quiesce.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using vmotion::Quiesce;
using types::Empty;

class QuiesceServiceImpl final : public Quiesce::Service {

    public:
        Status QuiesceMsgSnd(ServerContext* context,
                             const Empty* request,
                             Empty* response) override;
};



#endif  /* __QUIESCE_SVC_HPP__ */

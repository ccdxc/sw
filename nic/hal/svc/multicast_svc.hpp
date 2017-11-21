#ifndef __MULTICAST_SVC_HPP__
#define __MULTICAST_SVC_HPP__

#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/multicast.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using multicast::Multicast;
using multicast::MulticastEntrySpec;
using multicast::MulticastEntryStatus;
using multicast::MulticastEntryResponse;
using multicast::MulticastEntryKeyHandle;
using multicast::MulticastEntryRequestMsg;
using multicast::MulticastEntryResponseMsg;
using multicast::MulticastEntryDeleteRequestMsg;
using multicast::MulticastEntryDeleteResponseMsg;
using multicast::MulticastEntryGetRequest;
using multicast::MulticastEntryGetRequestMsg;
using multicast::MulticastEntryGetResponse;
using multicast::MulticastEntryGetResponseMsg;

class MulticastServiceImpl final : public Multicast::Service {
public:
    Status MulticastEntryCreate(ServerContext *context,
                                const MulticastEntryRequestMsg *req,
                                MulticastEntryResponseMsg *rsp) override;

    Status MulticastEntryUpdate(ServerContext *context,
                                const MulticastEntryRequestMsg *req,
                                MulticastEntryResponseMsg *rsp) override;


    Status MulticastEntryDelete(ServerContext *context,
                                const MulticastEntryDeleteRequestMsg *req,
                                MulticastEntryDeleteResponseMsg *rsp) override;

    Status MulticastEntryGet(ServerContext *context,
                             const MulticastEntryGetRequestMsg *req,
                             MulticastEntryGetResponseMsg *rsp) override;
};

#endif    // __MULTICAST_SVC_HPP__


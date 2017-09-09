#ifndef __RDMA_SVC_HPP__
#define __RDMA_SVC_HPP__

#include <base.h>
#include <grpc++/grpc++.h>
#include <types.pb.h>
#include <rdma.grpc.pb.h>

using grpc::ServerContext;
using grpc::Status;

using rdma::Rdma;
using rdma::RdmaServiceType;
using rdma::RdmaQpSpec;
using rdma::RdmaQpRequestMsg;
using rdma::RdmaQpResponse;
using rdma::RdmaQpResponseMsg;
using rdma::RdmaMemRegSpec;
using rdma::RdmaMemRegRequestMsg;
using rdma::RdmaMemRegResponse;
using rdma::RdmaMemRegResponseMsg;

class RdmaServiceImpl final : public Rdma::Service {
public:
    Status RdmaQpCreate(ServerContext *context,
                        const RdmaQpRequestMsg *req,
                        RdmaQpResponseMsg *rsp) override;

    Status RdmaQpUpdate(ServerContext *context,
                        const RdmaQpRequestMsg *req,
                        RdmaQpResponseMsg *rsp) override;
#if 0
    Status RdmaQpDelete(ServerContext *context,
                        const LifDeleteRequestMsg *req,
                        RdmaQpDeleteResponseMsg *rsp) override;

    Status RdmaQpGet(ServerContext *context,
                     const RdmaQpDeleteRequestMsg *req,
                     RdmaQpGetResponseMsg *rsp) override;
#endif

    Status RdmaMemReg(ServerContext *context,
                      const RdmaMemRegRequestMsg *reqs,
                      RdmaMemRegResponseMsg *resps) override;

};

#endif    // __RDMA_SVC_HPP__


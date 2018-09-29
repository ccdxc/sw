#ifndef __RDMA_SVC_HPP__
#define __RDMA_SVC_HPP__

#include "nic/include/base.hpp"
#include <grpc++/grpc++.h>
#include "gen/proto/types.pb.h"
#include "gen/proto/rdma.grpc.pb.h"
#include "gen/proto/rdma.pb.h"

using grpc::ServerContext;
using grpc::Status;

using rdma::Rdma;
using rdma::RdmaServiceType;
using rdma::RdmaQpSpec;
using rdma::RdmaQpRequestMsg;
using rdma::RdmaQpResponse;
using rdma::RdmaQpResponseMsg;
using rdma::RdmaQpUpdateSpec;
using rdma::RdmaQpUpdateRequestMsg;
using rdma::RdmaQpUpdateResponse;
using rdma::RdmaQpUpdateResponseMsg;
using rdma::RdmaQpUpdateOper;
using rdma::RdmaCqSpec;
using rdma::RdmaCqRequestMsg;
using rdma::RdmaCqResponse;
using rdma::RdmaCqResponseMsg;
using rdma::RdmaEqSpec;
using rdma::RdmaEqRequestMsg;
using rdma::RdmaEqResponse;
using rdma::RdmaEqResponseMsg;
using rdma::RdmaMemRegSpec;
using rdma::RdmaMemRegRequestMsg;
using rdma::RdmaMemRegResponse;
using rdma::RdmaMemRegResponseMsg;
using rdma::RdmaAllocLkeySpec;
using rdma::RdmaAllocLkeyRequestMsg;
using rdma::RdmaAllocLkeyResponse;
using rdma::RdmaAllocLkeyResponseMsg;
using rdma::RdmaMemWindowSpec;
using rdma::RdmaMemWindowRequestMsg;
using rdma::RdmaMemWindowResponse;
using rdma::RdmaMemWindowResponseMsg;
using rdma::RdmaAhRequestMsg;
using rdma::RdmaAhResponseMsg;
using rdma::RdmaAhSpec;
using rdma::RdmaAhResponse;
using rdma::RdmaAqSpec;
using rdma::RdmaAqRequestMsg;
using rdma::RdmaAqResponseMsg;
using rdma::RdmaAqResponse;

class RdmaServiceImpl final : public Rdma::Service {
public:
    Status RdmaQpCreate(ServerContext *context,
                        const RdmaQpRequestMsg *req,
                        RdmaQpResponseMsg *rsp) override;

    Status RdmaQpUpdate(ServerContext *context,
                        const RdmaQpUpdateRequestMsg *req,
                        RdmaQpUpdateResponseMsg *rsp) override;
#if 0
    Status RdmaQpDelete(ServerContext *context,
                        const LifDeleteRequestMsg *req,
                        RdmaQpDeleteResponseMsg *rsp) override;

    Status RdmaQpGet(ServerContext *context,
                     const RdmaQpDeleteRequestMsg *req,
                     RdmaQpGetResponseMsg *rsp) override;
#endif

    Status RdmaCqCreate(ServerContext *context,
                        const RdmaCqRequestMsg *req,
                        RdmaCqResponseMsg *rsp) override;
    Status RdmaEqCreate(ServerContext *context,
                        const RdmaEqRequestMsg *req,
                        RdmaEqResponseMsg *rsp) override;

    Status RdmaMemReg(ServerContext *context,
                      const RdmaMemRegRequestMsg *reqs,
                      RdmaMemRegResponseMsg *resps) override;

    Status RdmaAllocLkey(ServerContext *context,
                         const RdmaAllocLkeyRequestMsg *reqs,
                         RdmaAllocLkeyResponseMsg *resps) override;

    Status RdmaMemWindow(ServerContext *context,
                         const RdmaMemWindowRequestMsg *reqs,
                         RdmaMemWindowResponseMsg *resps) override;

    Status RdmaAhCreate(ServerContext *context,
                        const RdmaAhRequestMsg *req,
                        RdmaAhResponseMsg *rsp) override;
    Status
    RdmaAqCreate(ServerContext *context,
                 const RdmaAqRequestMsg *req,
                 RdmaAqResponseMsg *rsp);
};

#endif    // __RDMA_SVC_HPP__


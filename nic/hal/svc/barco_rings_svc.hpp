#ifndef __BARCO_RINGS_SVC_HPP__
#define __BARCO_RINGS_SVC_HPP__
#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/barco_rings.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using barcoRings::BarcoRings;
using barcoRings::GetOpaqueTagAddrRequest;
using barcoRings::GetOpaqueTagAddrResponse;
using barcoRings::GetOpaqueTagAddrRequestMsg;
using barcoRings::GetOpaqueTagAddrResponseMsg;
using barcoRings::BarcoGetReqDescrEntryRequest;
using barcoRings::BarcoGetReqDescrEntryRequestMsg;
using barcoRings::BarcoGetReqDescrEntryResponse;
using barcoRings::BarcoGetReqDescrEntryResponseMsg;
using barcoRings::BarcoGetRingMetaRequest;
using barcoRings::BarcoGetRingMetaRequestMsg;
using barcoRings::BarcoGetRingMetaResponse;
using barcoRings::BarcoGetRingMetaResponseMsg;

class BarcoRingsServiceImpl final : public BarcoRings::Service {

    public:
        Status GetOpaqueTagAddr(ServerContext* context,
                const GetOpaqueTagAddrRequestMsg* request,
                GetOpaqueTagAddrResponseMsg* response) override;

	Status BarcoGetReqDescrEntry(ServerContext *context,
                const BarcoGetReqDescrEntryRequestMsg *req,
                BarcoGetReqDescrEntryResponseMsg *rsp) override;

	Status BarcoGetRingMeta(ServerContext *context,
                const BarcoGetRingMetaRequestMsg *req,
                BarcoGetRingMetaResponseMsg *rsp) override;
};

#endif  /* __BARCO_RINGS_SVC_HPP__ */

//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/meter_svc.hpp"

Status
MeterSvcImpl::MeterCreate(ServerContext *context,
                          const pds::MeterRequest *proto_req,
                          pds::MeterResponse *proto_rsp) {
    pds_svc_meter_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
MeterSvcImpl::MeterUpdate(ServerContext *context,
                          const pds::MeterRequest *proto_req,
                          pds::MeterResponse *proto_rsp) {
    pds_svc_meter_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
MeterSvcImpl::MeterDelete(ServerContext *context,
                          const pds::MeterDeleteRequest *proto_req,
                          pds::MeterDeleteResponse *proto_rsp) {
    pds_svc_meter_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
MeterSvcImpl::MeterGet(ServerContext *context,
                       const pds::MeterGetRequest *proto_req,
                       pds::MeterGetResponse *proto_rsp) {
    pds_svc_meter_get(proto_req, proto_rsp);
    return Status::OK;
}

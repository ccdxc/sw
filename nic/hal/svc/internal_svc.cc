//------------------------------------------------------------------------------
// internal service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/internal_svc.hpp"
#include "nic/hal/src/internal.hpp"


Status
InternalServiceImpl::GetProgramAddress(
    ServerContext *context, const GetProgramAddressRequestMsg *reqs,
    ProgramAddressResponseMsg *resps) {

    for (int i = 0; i < reqs->reqs_size(); i++) {
        hal::GetProgramAddress(reqs->reqs(i), resps->add_resps());
    }
    return Status::OK;
}

Status
InternalServiceImpl::AllocHbmAddress(
    ServerContext *context, const AllocHbmAddressRequestMsg *reqs,
    AllocHbmAddressResponseMsg *resps) {

    for (int i = 0; i < reqs->reqs_size(); i++) {
        hal::AllocHbmAddress(reqs->reqs(i), resps->add_resps());
    }
    return Status::OK;
}

Status
InternalServiceImpl::ConfigureLifBdf(
    ServerContext *context, const ConfigureLifBdfRequestMsg *reqs,
    ConfigureLifBdfResponseMsg *resps) {

    for (int i = 0; i < reqs->reqs_size(); i++) {
        hal::ConfigureLifBdf(reqs->reqs(i), resps->add_resps());
    }
    return Status::OK;
}

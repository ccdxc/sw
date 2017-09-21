//------------------------------------------------------------------------------
// internal service implementation
//------------------------------------------------------------------------------

#include <base.h>
#include <trace.hpp>
#include <internal_svc.hpp>
#include <internal.hpp>


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

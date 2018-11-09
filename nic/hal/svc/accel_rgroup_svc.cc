#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/hal/svc/accel_rgroup_svc.hpp"
#include "nic/hal/src/internal/accel_rgroup.hpp"

Status 
AccelRGroupServiceImpl::AccelRGroupAdd(ServerContext* context,
                                       const AccelRGroupAddRequestMsg* request,
                                       AccelRGroupAddResponseMsg* response)
{
    uint32_t    nreqs = request->request_size();
    uint32_t    i;

    HAL_TRACE_DEBUG("{}: {} requests", __FUNCTION__, nreqs);
    for (i = 0; i < nreqs; i++) {
        auto rsp = response->add_response();
        auto req = request->request(i);
        hal::AccelRGroupAdd(req, rsp);

    }
    return Status::OK;
}

Status
AccelRGroupServiceImpl::AccelRGroupDel(ServerContext *context,
                                       const AccelRGroupDelRequestMsg *request,
                                       AccelRGroupDelResponseMsg *response)
{
    uint32_t    nreqs = request->request_size();
    uint32_t    i;

    HAL_TRACE_DEBUG("{}: {} requests", __FUNCTION__, nreqs);
    for (i = 0; i < nreqs; i++) {
        auto rsp = response->add_response();
        auto req = request->request(i);
        hal::AccelRGroupDel(req, rsp);
    }
    return Status::OK;
}

Status
AccelRGroupServiceImpl::AccelRGroupRingAdd(ServerContext *context,
                                           const AccelRGroupRingAddRequestMsg *request,
                                           AccelRGroupRingAddResponseMsg *response)
{
    uint32_t    nreqs = request->request_size();
    uint32_t    i;

    HAL_TRACE_DEBUG("{}: {} requests", __FUNCTION__, nreqs);
    for (i = 0; i < nreqs; i++) {
        auto rsp = response->add_response();
        auto req = request->request(i);
        hal::AccelRGroupRingAdd(req, rsp);
    }
    return Status::OK;
}

Status
AccelRGroupServiceImpl::AccelRGroupRingDel(ServerContext *context,
                                           const AccelRGroupRingDelRequestMsg *request,
                                           AccelRGroupRingDelResponseMsg *response)
{
    uint32_t    nreqs = request->request_size();
    uint32_t    i;

    HAL_TRACE_DEBUG("{}: {} requests", __FUNCTION__, nreqs);
    for (i = 0; i < nreqs; i++) {
        auto rsp = response->add_response();
        auto req = request->request(i);
        hal::AccelRGroupRingDel(req, rsp);
    }
    return Status::OK;
}


Status
AccelRGroupServiceImpl::AccelRGroupResetSet(ServerContext *context,
                                            const AccelRGroupResetSetRequestMsg *request,
                                            AccelRGroupResetSetResponseMsg *response)
{
    uint32_t    nreqs = request->request_size();
    uint32_t    i;

    HAL_TRACE_DEBUG("{}: {} requests", __FUNCTION__, nreqs);
    for (i = 0; i < nreqs; i++) {
        auto rsp = response->add_response();
        auto req = request->request(i);
        hal::AccelRGroupResetSet(req, rsp);
    }
    return Status::OK;
}

Status
AccelRGroupServiceImpl::AccelRGroupEnableSet(ServerContext *context,
                                             const AccelRGroupEnableSetRequestMsg *request,
                                             AccelRGroupEnableSetResponseMsg *response)
{
    uint32_t    nreqs = request->request_size();
    uint32_t    i;

    HAL_TRACE_DEBUG("{}: {} requests", __FUNCTION__, nreqs);
    for (i = 0; i < nreqs; i++) {
        auto rsp = response->add_response();
        auto req = request->request(i);
        hal::AccelRGroupEnableSet(req, rsp);
    }
    return Status::OK;
}

Status
AccelRGroupServiceImpl::AccelRGroupPndxSet(ServerContext *context,
                                           const AccelRGroupPndxSetRequestMsg *request,
                                           AccelRGroupPndxSetResponseMsg *response)
{
    uint32_t    nreqs = request->request_size();
    uint32_t    i;

    HAL_TRACE_DEBUG("{}: {} requests", __FUNCTION__, nreqs);
    for (i = 0; i < nreqs; i++) {
        auto rsp = response->add_response();
        auto req = request->request(i);
        hal::AccelRGroupPndxSet(req, rsp);
    }
    return Status::OK;
}

Status
AccelRGroupServiceImpl::AccelRGroupInfoGet(ServerContext *context,
                                           const AccelRGroupInfoGetRequestMsg *request,
                                           AccelRGroupInfoGetResponseMsg *response)
{
    uint32_t    nreqs = request->request_size();
    uint32_t    i;

    HAL_TRACE_DEBUG("{}: {} requests", __FUNCTION__, nreqs);
    for (i = 0; i < nreqs; i++) {
        auto rsp = response->add_response();
        auto req = request->request(i);
        hal::AccelRGroupInfoGet(req, rsp);
    }
    return Status::OK;
}

Status
AccelRGroupServiceImpl::AccelRGroupIndicesGet(ServerContext *context,
                                              const AccelRGroupIndicesGetRequestMsg *request,
                                              AccelRGroupIndicesGetResponseMsg *response)
{
    uint32_t    nreqs = request->request_size();
    uint32_t    i;

    HAL_TRACE_DEBUG("{}: {} requests", __FUNCTION__, nreqs);
    for (i = 0; i < nreqs; i++) {
        auto rsp = response->add_response();
        auto req = request->request(i);
        hal::AccelRGroupIndicesGet(req, rsp);
    }
    return Status::OK;
}

Status
AccelRGroupServiceImpl::AccelRGroupMetricsGet(ServerContext *context,
                                              const AccelRGroupMetricsGetRequestMsg *request,
                                              AccelRGroupMetricsGetResponseMsg *response)
{
    uint32_t    nreqs = request->request_size();
    uint32_t    i;

    HAL_TRACE_DEBUG("{}: {} requests", __FUNCTION__, nreqs);
    for (i = 0; i < nreqs; i++) {
        auto rsp = response->add_response();
        auto req = request->request(i);
        hal::AccelRGroupMetricsGet(req, rsp);
    }
    return Status::OK;
}


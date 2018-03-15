#include "fte.hpp"
#include "fte_flow.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/p4/iris/include/defines.h"

namespace fte {

static void
hal_ret_to_api_status(hal_ret_t ret, SessionResponse *rsp)
{
    //update api status
    switch (ret) {
    case HAL_RET_OK:
        rsp->set_api_status(types::API_STATUS_OK);
        break;
    case HAL_RET_HW_PROG_ERR:
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        break;
    case HAL_RET_TABLE_FULL:
    case HAL_RET_OTCAM_FULL:
        rsp->set_api_status(types::API_STATUS_OUT_OF_RESOURCE);
        break;
    case HAL_RET_OOM:
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        break;
    case HAL_RET_INVALID_ARG:
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        break;
    case HAL_RET_VRF_NOT_FOUND:
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        break;
    case HAL_RET_L2SEG_NOT_FOUND:
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        break;
    case HAL_RET_IF_NOT_FOUND:
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        break;
    case HAL_RET_SECURITY_PROFILE_NOT_FOUND:
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        break;
    case HAL_RET_QOS_CLASS_NOT_FOUND:
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        break;
    case HAL_RET_HANDLE_INVALID:
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        break;
    default:
        rsp->set_api_status(types::API_STATUS_ERR);
        break;
    }
}

// Process grpc session_create
hal_ret_t
session_create (SessionSpec& spec, SessionResponse *rsp)
{
    hal_ret_t ret;
    ctx_t ctx = {};
    flow_t iflow[ctx_t::MAX_STAGES], rflow[ctx_t::MAX_STAGES];
    uint16_t num_features;
    size_t fstate_size = feature_state_size(&num_features);
    feature_state_t *feature_state;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("fte::{}: Session id {} Create in Vrf id {}", __FUNCTION__, 
                    spec.session_id(), spec.meta().vrf_id());


    feature_state = (feature_state_t*)HAL_MALLOC(hal::HAL_MEM_ALLOC_FTE, fstate_size);
    if (!feature_state) {
        ret = HAL_RET_OOM;
        goto end;
    }

    //Init context
    ret = ctx.init(&spec, rsp,  iflow, rflow, feature_state, num_features);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failied to init context, ret={}", ret);
        goto end;
    }
    
    ret = ctx.process();

 end:
    if (feature_state) {
        HAL_FREE(hal::HAL_MEM_ALLOC_FTE, feature_state);
    }

    hal_ret_to_api_status(ret, rsp);

    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

hal_ret_t
session_delete (SessionSpec& spec, SessionResponse *rsp)
{
    hal_ret_t ret;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("fte::{}: Session id {} Delete in Vrf id {}", __FUNCTION__,
                    spec.session_id(), spec.meta().vrf_id());

    ret = session_delete(hal::find_session_by_id(spec.session_id()));

    hal_ret_to_api_status(ret, rsp);

    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}


// Process Session delete from HAL
hal_ret_t
session_delete (hal::session_t *session)
{
    hal_ret_t ret;
    ctx_t ctx = {};
    uint16_t num_features;
    size_t fstate_size = feature_state_size(&num_features);
    feature_state_t *feature_state = NULL;
    flow_t iflow[ctx_t::MAX_STAGES], rflow[ctx_t::MAX_STAGES];

    HAL_TRACE_DEBUG("fte::{}: Received session Delete for session id {} in Vrf id {}", 
                    __FUNCTION__, session->config.session_id, 
                    (hal::vrf_lookup_by_handle(session->vrf_handle))->vrf_id);

    HAL_TRACE_DEBUG("num features: {} feature state size: {}", num_features, fstate_size);

    feature_state = (feature_state_t*)HAL_MALLOC(hal::HAL_MEM_ALLOC_FTE, fstate_size);
    if (!feature_state) {
        ret = HAL_RET_OOM;
        goto end;
    }

    //Init context
    ret = ctx.init(session, iflow, rflow, feature_state, num_features);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failied to init context, ret={}", ret);
        goto end;
    }
    ctx.set_pipeline_event(FTE_SESSION_DELETE);

    ret = ctx.process();

end:
    if (feature_state) {
        HAL_FREE(hal::HAL_MEM_ALLOC_FTE, feature_state);
    }
    return ret;
}

} // namespace fte

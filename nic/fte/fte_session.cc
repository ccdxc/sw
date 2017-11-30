#include "fte.hpp"
#include "fte_flow.hpp"
#include "nic/p4/nw/include/defines.h"

namespace fte {

// Process grpc session_create
hal_ret_t
session_create (SessionSpec& spec, SessionResponse *rsp)
{
    hal_ret_t ret;
    ctx_t ctx = {};
    flow_t iflow[ctx_t::MAX_STAGES], rflow[ctx_t::MAX_STAGES];
    size_t fstate_size = feature_state_size();
    uint8_t *feature_state;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("fte::{}: Session id {} Create in Vrf id {}", __FUNCTION__, 
                    spec.session_id(), spec.meta().vrf_id());


    feature_state = (uint8_t*)HAL_MALLOC(hal::HAL_MEM_ALLOC_FTE, fstate_size);
    if (!feature_state) {
        ret = HAL_RET_OOM;
        goto end;
    }

    //Init context
    ret = ctx.init(&spec, rsp,  iflow, rflow, feature_state, fstate_size);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failied to init context, ret={}", ret);
        goto end;
    }
    
    ret = ctx.process();

 end:
    if (feature_state) {
        HAL_FREE(hal::HAL_MEM_ALLOC_FTE, feature_state);
    }

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
        rsp->set_api_status(types::API_STATUS_VRF_NOT_FOUND);
        break;
    case HAL_RET_L2SEG_NOT_FOUND:
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_NOT_FOUND);
        break;
    case HAL_RET_IF_NOT_FOUND:
        rsp->set_api_status(types::API_STATUS_INTERFACE_NOT_FOUND);
        break;
    case HAL_RET_SECURITY_PROFILE_NOT_FOUND:
        rsp->set_api_status(types::API_STATUS_NWSEC_PROFILE_NOT_FOUND);
        break;
    case HAL_RET_POLICER_NOT_FOUND:
        rsp->set_api_status(types::API_STATUS_POLICER_NOT_FOUND);
        break;
    case HAL_RET_HANDLE_INVALID:
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        break;
    default:
        rsp->set_api_status(types::API_STATUS_ERR);
        break;
    }


    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

} // namespace fte

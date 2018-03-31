// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/include/mtrack.hpp"
#include "nic/hal/src/debug/debug.hpp"
#include "sdk/slab.hpp"
#include "sdk/catalog.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/pd_api.hpp"

using sdk::lib::slab;

namespace hal {

static sdk::lib::catalog *
catalog (void)
{
    return g_hal_state->catalog();
}

static uint32_t
max_mpu_per_stage (void)
{
    return catalog()->max_mpu_per_stage();
}

// TODO: This doesn't look right !!
static uint32_t
mpu_trace_size (void)
{
    return catalog()->mpu_trace_size();
}

bool
mtrack_map_walk_cb (void *ctxt, uint32_t alloc_id,
                    utils::mtrack_info_t *minfo)
{
    debug::MemTrackGetResponse       *response;
    debug::MemTrackGetResponseMsg    *rsp;
    
    if (!ctxt || !minfo) {
        HAL_ABORT(FALSE);
        return true;
    }
    rsp = (debug::MemTrackGetResponseMsg *)ctxt;
    response = rsp->add_response();
    if (!response) {
        // memory allocation failure, stop walking !!
        return true;
    }
    response->set_api_status(types::API_STATUS_OK);
    response->mutable_spec()->set_alloc_id(alloc_id);
    response->mutable_stats()->set_num_allocs(minfo->num_allocs);
    response->mutable_stats()->set_num_frees(minfo->num_frees);

    return false;
}

hal_ret_t
mtrack_get (debug::MemTrackGetRequest& req,
            debug::MemTrackGetResponseMsg *rsp)
{
    utils::g_hal_mem_mgr.walk(rsp, mtrack_map_walk_cb);
    return HAL_RET_OK;
}

hal_ret_t
set_slab_response (slab *s, debug::SlabGetResponseMsg *rsp)
{
    debug::SlabGetResponse  *response;

    response = rsp->add_response();
    if (!response) {
        // memory allocation failure!!
        return HAL_RET_OOM;
    }

    if (!s) {
        response->set_api_status(types::API_STATUS_ERR);
        return HAL_RET_INVALID_ARG;
    }

    response->set_api_status(types::API_STATUS_OK);

    response->mutable_spec()->set_name(s->name());
    response->mutable_spec()->set_id(s->slab_id());
    response->mutable_spec()->set_element_size(s->elem_sz());
    response->mutable_spec()->set_elements_per_block(s->elems_per_block());
    response->mutable_spec()->set_thread_safe(s->thread_safe());
    response->mutable_spec()->set_grow_on_demand(s->grow_on_demand());
    //response->mutable_spec()->set_delay_delete(s->delay_delete());
    response->mutable_spec()->set_zero_on_allocation(s->zero_on_alloc());

    response->mutable_stats()->set_num_elements_in_use(s->num_in_use());
    response->mutable_stats()->set_num_allocs(s->num_allocs());
    response->mutable_stats()->set_num_frees(s->num_frees());
    response->mutable_stats()->set_num_alloc_errors(s->num_alloc_fails());
    response->mutable_stats()->set_num_blocks(s->num_blocks());

    return HAL_RET_OK;
}

hal_ret_t
slab_get_from_req (debug::SlabGetRequest& req, debug::SlabGetResponseMsg *rsp)
{
#if 0
    hal_slab_t  slab_id;
    hal_ret_t   ret = HAL_RET_OK;
    uint32_t i = 0;
    slab *s;
   
    slab_id = (hal_slab_t)req.id();

    if (slab_id < HAL_SLAB_PI_MAX) {
        s = hal::g_hal_state->get_slab(slab_id);
        ret = set_slab_response(s, rsp);
    } else if (slab_id < HAL_SLAB_PD_MAX) {
        s = hal::pd::g_hal_state_pd->get_slab(slab_id);
        ret = set_slab_response(s, rsp);
    } else if (slab_id == HAL_SLAB_ALL) {
        for (i = (uint32_t) HAL_SLAB_PI_MIN; i < (uint32_t) HAL_SLAB_PI_MAX; i ++) {
            s = hal::g_hal_state->get_slab((hal_slab_t) i);
            ret = set_slab_response (s, rsp);
        }
        for (i = (uint32_t) HAL_SLAB_PD_MIN; i < (uint32_t) HAL_SLAB_PD_MAX; i ++) {
            s = hal::pd::g_hal_state_pd->get_slab((hal_slab_t) i);
            ret = set_slab_response(s, rsp);
        }
    } else {
        HAL_TRACE_ERR("Unexpected slab id {}", slab_id);
        ret = HAL_RET_INVALID_ARG;
    }

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get slab for slab id {}", i ? i : slab_id);
        return ret;
    }
#endif

    return HAL_RET_OK;
}

hal_ret_t
mpu_trace_enable (debug::MpuTraceRequest& req, debug::MpuTraceResponseMsg *rsp)
{
    hal::pd::pd_mpu_trace_enable_args_t args;

    memset(&args, 0, sizeof(hal::pd::pd_mpu_trace_enable_args_t));

    switch(req.pipeline_type()) {
    case debug::MPU_TRACE_PIPELINE_P4_INGRESS:
        args.pipeline_type = hal::pd::MPU_TRACE_PIPELINE_P4_INGRESS;
        break;

    case debug::MPU_TRACE_PIPELINE_P4_EGRESS:
        args.pipeline_type = hal::pd::MPU_TRACE_PIPELINE_P4_EGRESS;
        break;

    default:
        args.pipeline_type = hal::pd::MPU_TRACE_PIPELINE_NONE;
        break;
    }

    args.max_mpu_per_stage = max_mpu_per_stage();
    args.mpu_trace_size    = mpu_trace_size();
    args.stage_id          = req.stage_id();
    args.mpu               = req.mpu();
    args.watch_pc          = req.watch_pc();
    args.base_addr         = req.base_addr();
    args.buf_size          = 5;

    if (req.enable() == true) {
        args.enable = 1;
    }

    if (req.trace_enable() == true) {
        args.trace_enable = 1;
    }

    if (req.phv_debug() == true) {
        args.phv_debug = 1;
    }

    if (req.phv_error() == true) {
        args.phv_error = 1;
    }

    if (req.table_key() == true) {
        args.table_key = 1;
    }

    if (req.instructions() == true) {
        args.instructions = 1;
    }

    if (req.wrap() == true) {
        args.wrap = 1;
    }

    if (req.reset() == true) {
        args.reset = 1;
    }

    return hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_MPU_TRACE_ENABLE,
                                (void *)&args);
}

//------------------------------------------------------------------------------
// process a trace update request
//------------------------------------------------------------------------------
hal_ret_t
trace_update (TraceSpec& spec, TraceResponse *rsp)
{
    if (spec.trace_level() == debug::TRACE_LEVEL_ERROR) {
        hal::utils::g_trace_logger->set_trace_level(hal::utils::trace_err);
        rsp->set_trace_level(debug::TRACE_LEVEL_ERROR);
    } else if (spec.trace_level() == debug::TRACE_LEVEL_DEBUG) {
        hal::utils::g_trace_logger->set_trace_level(hal::utils::trace_debug);
        rsp->set_trace_level(debug::TRACE_LEVEL_DEBUG);
    } else {
        hal::utils::g_trace_logger->set_trace_level(hal::utils::trace_none);
        rsp->set_trace_level(debug::TRACE_LEVEL_NONE);
    }
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a trace get request
//------------------------------------------------------------------------------
hal_ret_t
trace_get (TraceResponseMsg *rsp)
{
    auto response = rsp->add_response();

    if (!response) {
        return HAL_RET_OOM;
    }
    if (hal::utils::g_trace_logger) {
        response->set_api_status(types::API_STATUS_OK);
        if (hal::utils::g_trace_logger->trace_level() == hal::utils::trace_err) {
            response->set_trace_level(debug::TRACE_LEVEL_ERROR);
        } else if (hal::utils::g_trace_logger->trace_level() == hal::utils::trace_debug) {
            response->set_trace_level(debug::TRACE_LEVEL_DEBUG);
        } else {
            response->set_trace_level(debug::TRACE_LEVEL_NONE);
        }
    } else {
        response->set_api_status(types::API_STATUS_NOT_FOUND);
        response->set_trace_level(debug::TRACE_LEVEL_NONE);
    }

    return HAL_RET_OK;
}

}    // namespace hal

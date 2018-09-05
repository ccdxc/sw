//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/include/mtrack.hpp"
#include "nic/hal/src/debug/debug.hpp"
#include "sdk/slab.hpp"
#include "sdk/catalog.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/pd_api.hpp"
#include "sdk/timestamp.hpp"

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
    response->mutable_spec()->set_raw_block_size(s->raw_block_sz());
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
    hal_slab_t  slab_id;
    hal_ret_t   ret = HAL_RET_OK;
    pd::pd_func_args_t  pd_func_args = {0};
    pd::pd_get_slab_args_t args;
    uint32_t i = 0;
    slab *s;

    slab_id = (hal_slab_t)req.id();

    if (slab_id < HAL_SLAB_PI_MAX) {
        s = hal::g_hal_state->get_slab(slab_id);
        ret = set_slab_response(s, rsp);
    } else if (slab_id < HAL_SLAB_PD_MAX) {
        args.slab_id = slab_id;
        pd_func_args.pd_get_slab = &args;
        ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_GET_SLAB, &pd_func_args);
        if (ret == HAL_RET_OK) {
            s = args.slab;
            ret = set_slab_response(s, rsp);
        }
    } else if (slab_id == HAL_SLAB_ALL) {
        for (i = (uint32_t) HAL_SLAB_PI_MIN; i < (uint32_t) HAL_SLAB_PI_MAX; i ++) {
            s = hal::g_hal_state->get_slab((hal_slab_t) i);
            ret = set_slab_response (s, rsp);
        }
        for (i = (uint32_t) HAL_SLAB_PD_MIN; i < (uint32_t) HAL_SLAB_PD_MAX; i ++) {
            args.slab_id = (hal_slab_t) i;
            pd_func_args.pd_get_slab = &args;
            ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_GET_SLAB, &pd_func_args);
            if (ret == HAL_RET_OK) {
                s = args.slab;
                ret = set_slab_response(s, rsp);
            }
        }
    } else {
        HAL_TRACE_ERR("Unexpected slab id {}", slab_id);
        ret = HAL_RET_INVALID_ARG;
    }

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get slab for slab id {}", i ? i : slab_id);
        return ret;
    }

    return HAL_RET_OK;
}

hal_ret_t
mpu_trace_enable (debug::MpuTraceRequest& req, debug::MpuTraceResponseMsg *rsp)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd::pd_func_args_t  pd_func_args = {0};

    hal::pd::pd_mpu_trace_enable_args_t args;

    memset(&args, 0, sizeof(hal::pd::pd_mpu_trace_enable_args_t));

    switch(req.pipeline_type()) {
    case debug::MPU_TRACE_PIPELINE_P4_INGRESS:
        args.pipeline_type = hal::pd::MPU_TRACE_PIPELINE_P4_INGRESS;
        break;

    case debug::MPU_TRACE_PIPELINE_P4_EGRESS:
        args.pipeline_type = hal::pd::MPU_TRACE_PIPELINE_P4_EGRESS;
        break;

    case debug::MPU_TRACE_PIPELINE_P4P_RXDMA:
        args.pipeline_type = hal::pd::MPU_TRACE_PIPELINE_P4P_RXDMA;
        break;

    case debug::MPU_TRACE_PIPELINE_P4P_TXDMA:
        args.pipeline_type = hal::pd::MPU_TRACE_PIPELINE_P4P_TXDMA;
        break;

    default:
        args.pipeline_type = hal::pd::MPU_TRACE_PIPELINE_NONE;
        break;
    }

    args.max_mpu_per_stage             = max_mpu_per_stage();
    args.stage_id                      = req.stage_id();
    args.mpu                           = req.mpu();
    args.mpu_trace_info.watch_pc       = req.spec().watch_pc();
    args.mpu_trace_info.base_addr      = req.spec().base_addr();
    args.mpu_trace_info.buf_size       = req.spec().buf_size();

    if (req.spec().enable() == true) {
        args.mpu_trace_info.enable = 1;
    }

    if (req.spec().trace_enable() == true) {
        args.mpu_trace_info.trace_enable = 1;
    }

    if (req.spec().phv_debug() == true) {
        args.mpu_trace_info.phv_debug = 1;
    }

    if (req.spec().phv_error() == true) {
        args.mpu_trace_info.phv_error = 1;
    }

    if (req.spec().table_key() == true) {
        args.mpu_trace_info.table_key = 1;
    }

    if (req.spec().instructions() == true) {
        args.mpu_trace_info.instructions = 1;
    }

    if (req.spec().wrap() == true) {
        args.mpu_trace_info.wrap = 1;
    }

    if (req.spec().reset() == true) {
        args.mpu_trace_info.reset = 1;
    }

    pd_func_args.pd_mpu_trace_enable = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_MPU_TRACE_ENABLE,
                               &pd_func_args);

    debug::MpuTraceResponse *resp = rsp->add_response();

    if (ret != HAL_RET_OK) {
        resp->set_api_status(types::API_STATUS_ERR);
        return ret;
    }

    resp->set_api_status(types::API_STATUS_OK);
    resp->mutable_spec()->set_base_addr(args.mpu_trace_info.base_addr);

    return ret;
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

//------------------------------------------------------------------------------
// process a flush_logs request
//------------------------------------------------------------------------------
hal_ret_t
flush_logs (FlushLogsResponse *rsp)
{
    if (hal::utils::hal_logger()) {
        hal::utils::hal_logger()->flush();
        rsp->set_api_status(types::API_STATUS_OK);
    } else {
        rsp->set_api_status(types::API_STATUS_ERR);
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a flush_logs request
//------------------------------------------------------------------------------
hal_ret_t
thread_get (ThreadResponseMsg *response)
{
    timespec_t          curr_ts, ts_diff, hb_ts;
    sdk::lib::thread    *hal_thread;
    ThreadResponse      *rsp;
    uint64_t            diff_ns;

    clock_gettime(CLOCK_MONOTONIC, &curr_ts);
    for (uint32_t tid = HAL_THREAD_ID_CFG + 1; tid < HAL_THREAD_ID_MAX; tid++) {
        if ((hal_thread = hal_thread_get(tid)) != NULL) {
            rsp = response->add_response();
            rsp->set_control_core_mask(hal_thread->control_cores_mask());
            rsp->set_data_core_mask(hal_thread->data_cores_mask());
            rsp->mutable_spec()->set_name(hal_thread->name());
            rsp->mutable_spec()->set_id(hal_thread->thread_id());
            rsp->mutable_spec()->set_pthread_id(hal_thread->pthread_id());
            rsp->mutable_spec()->set_prio(hal_thread->priority());

            switch (hal_thread->sched_policy()) {
            case SCHED_OTHER:
                rsp->mutable_spec()->set_sched_policy(debug::SCHED_POLICY_OTHER);
                break;
            case SCHED_FIFO:
                rsp->mutable_spec()->set_sched_policy(debug::SCHED_POLICY_FIFO);
                break;
            case SCHED_RR:
                rsp->mutable_spec()->set_sched_policy(debug::SCHED_POLICY_RR);
                break;
            default:
                break;
            }

            switch (hal_thread->thread_role()) {
            case sdk::lib::THREAD_ROLE_CONTROL:
                rsp->mutable_spec()->set_role(debug::THREAD_ROLE_CONTROL);
                break;
            case sdk::lib::THREAD_ROLE_DATA:
                rsp->mutable_spec()->set_role(debug::THREAD_ROLE_DATA);
                break;
            default:
                break;
            }

            rsp->mutable_spec()->set_running(hal_thread->is_running());
            rsp->mutable_spec()->set_core_mask(hal_thread->cores_mask());

            hb_ts = hal_thread->heartbeat_ts();
            ts_diff = sdk::timestamp_diff(&curr_ts, &hb_ts);
            sdk::timestamp_to_nsecs(&ts_diff, &diff_ns);
            rsp->mutable_status()->set_last_hb(diff_ns);
            
            rsp->set_api_status(types::API_STATUS_OK);
        }
    }

    return HAL_RET_OK;
}

}    // namespace hal

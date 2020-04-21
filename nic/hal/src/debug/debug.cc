//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/sdk/lib/utils/mtrack.hpp"
#include "nic/hal/src/debug/debug.hpp"
#include "lib/slab/slab.hpp"
#include "nic/sdk/lib/catalog/catalog.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/sdk/include/sdk/timestamp.hpp"
#include "nic/hal/pd/asicpd/asic_pd_common.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/fte/acl/list.hpp"
#include "nic/fte/acl/itree.hpp"
#include "nic/fte/acl/acl_ctx.hpp"
#include "nic/fte/fte.hpp"

using sdk::lib::slab;

namespace sdk {
namespace lib {
extern sdk::lib::twheel *g_twheel;
}    // lib
}    // sdk

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
                    sdk::utils::mtrack_info_t *minfo)
{
    debug::MemTrackGetResponse       *response;
    debug::MemTrackGetResponseMsg    *rsp;

    if (!ctxt || !minfo) {
        HAL_ABORT(FALSE);
        return false;
    }
    rsp = (debug::MemTrackGetResponseMsg *)ctxt;
    response = rsp->add_response();
    if (!response) {
        // memory allocation failure, stop walking !!
        return false;
    }
    response->set_api_status(types::API_STATUS_OK);
    response->mutable_spec()->set_alloc_id(alloc_id);
    response->mutable_stats()->set_num_allocs(minfo->num_allocs);
    response->mutable_stats()->set_num_frees(minfo->num_frees);

    return true;
}

hal_ret_t
mtrack_get (debug::MemTrackGetRequest& req,
            debug::MemTrackGetResponseMsg *rsp)
{
    g_hal_mem_mgr.walk(rsp, mtrack_map_walk_cb);
    sdk::utils::g_sdk_mem_mgr.walk(rsp, mtrack_map_walk_cb);
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
        // Sdk slabs
        set_slab_response(acl_ctx_t::get_slab(), rsp);
        set_slab_response(acl::list_t::get_list_slab(), rsp);
        set_slab_response(acl::list_t::get_list_item_slab(), rsp);
        set_slab_response(acl::itree_t::get_itree_slab(), rsp);
        set_slab_response(acl::itree_t::get_itree_node_slab(), rsp);
        set_slab_response(sdk::lib::g_twheel->get_slab(), rsp);
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
// process a register update request
//------------------------------------------------------------------------------
hal_ret_t
register_update (debug::RegisterRequest& req, debug::RegisterResponse *rsp)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd::pd_func_args_t  pd_func_args = {0};

    hal::pd::pd_reg_write_args_t args;

    memset(&args, 0, sizeof(hal::pd::pd_reg_write_args_t));

    args.register_id             = static_cast<pd::pd_reg_write_type_t>(req.reg_id());
    args.instance                = req.instance();
    args.value                   = req.reg_data();

    pd_func_args.pd_reg_write = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_REG_WRITE,
                               &pd_func_args);

    if (ret != HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_ERR);
        return ret;
    }

    rsp->set_api_status(types::API_STATUS_OK);

    return ret;
}
//------------------------------------------------------------------------------
// process a trace update request
//------------------------------------------------------------------------------
hal_ret_t
trace_update (TraceSpec& spec, TraceResponse *rsp)
{
    if (spec.trace_level() == debug::TRACE_LEVEL_ERROR) {
        utils::trace_update(::utils::trace_err);
        rsp->set_trace_level(debug::TRACE_LEVEL_ERROR);
    } else if (spec.trace_level() == debug::TRACE_LEVEL_DEBUG) {
        utils::trace_update(::utils::trace_debug);
        rsp->set_trace_level(debug::TRACE_LEVEL_DEBUG);
    } else if (spec.trace_level() == debug::TRACE_LEVEL_VERBOSE) {
        utils::trace_update(::utils::trace_verbose);
        rsp->set_trace_level(debug::TRACE_LEVEL_VERBOSE);
    } else {
        utils::trace_update(::utils::trace_none);
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
    if (utils::g_trace_logger) {
        response->set_api_status(types::API_STATUS_OK);
        if (utils::g_trace_logger->trace_level() == ::utils::trace_err) {
            response->set_trace_level(debug::TRACE_LEVEL_ERROR);
        } else if (utils::g_trace_logger->trace_level() == ::utils::trace_debug) {
            response->set_trace_level(debug::TRACE_LEVEL_DEBUG);
        } else if (utils::g_trace_logger->trace_level() == ::utils::trace_verbose) {
            response->set_trace_level(debug::TRACE_LEVEL_VERBOSE);
        } else {
            response->set_trace_level(debug::TRACE_LEVEL_NONE);
        }
    } else {
        response->set_api_status(types::API_STATUS_NOT_FOUND);
        response->set_trace_level(debug::TRACE_LEVEL_NONE);
    }

    return HAL_RET_OK;
}


hal_ret_t
session_ctrl_update (SessionCtrlSpec& spec)
{
    for (uint32_t i = 0; i < hal::g_hal_cfg.num_data_cores; i++) {
        fte::set_fte_max_sessions(i, spec.max_session());
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a flush_logs request
//------------------------------------------------------------------------------
hal_ret_t
flush_logs (FlushLogsResponse *rsp)
{
    if (utils::hal_logger()) {
        utils::hal_logger()->flush();
        rsp->set_api_status(types::API_STATUS_OK);
    } else {
        rsp->set_api_status(types::API_STATUS_ERR);
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process thread get request
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

static inline fte_span_t *
fte_span_alloc (void)
{
    fte_span_t    *fte_span;

    fte_span = (fte_span_t *)g_hal_state->fte_span_slab()->alloc();
    if (fte_span == NULL) {
        return NULL;
    }
    return fte_span;
}

static inline fte_span_t *
fte_span_init (fte_span_t *fte_span)
{
    if (!fte_span) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&fte_span->slock, PTHREAD_PROCESS_SHARED);

    memset(fte_span, 0, sizeof(fte_span_t));

    return fte_span;
}

static inline fte_span_t *
fte_span_alloc_init (void)
{
    return fte_span_init(fte_span_alloc());
}

static inline hal_ret_t
fte_span_free (fte_span_t *fte_span)
{
    SDK_SPINLOCK_DESTROY(&fte_span->slock);
    hal::delay_delete_to_slab(HAL_SLAB_FTE_SPAN, fte_span);
    return HAL_RET_OK;
}

static inline hal_ret_t
fte_span_cleanup (fte_span_t *fte_span)
{
    fte_span_free(fte_span);

    return HAL_RET_OK;
}

hal_ret_t
validate_fte_span_create (FteSpanRequest& req,
                          FteSpanResponse *rsp)
{
    return HAL_RET_OK;
}

static hal_ret_t
fte_span_prepare_rsp (FteSpanResponse *rsp, hal_ret_t ret, uint32_t stats_index)
{
    rsp->set_stats_index(stats_index);
    rsp->set_api_status(hal_prepare_rsp(ret));
    return HAL_RET_OK;
}

static hal_ret_t
fte_span_init_from_spec (fte_span_t *fte_span, FteSpanRequest* req)
{
    fte_span->sel = req->selector();
    fte_span->src_lport = req->src_lport();
    fte_span->src_lif = req->src_lif();
    fte_span->dst_lport = req->dst_lport();
    fte_span->drop_reason = req->drop_reason();
    fte_span->flow_lkup_dir = req->flow_lkup_dir();
    fte_span->flow_lkup_type = req->flow_lkup_type();
    fte_span->flow_lkup_vrf = req->flow_lkup_vrf();
    fte_span->ip_family = req->flow_lkup_src().ip_af();
    fte_span->flow_lkup_src.v4_addr = req->flow_lkup_src().v4_addr();
    fte_span->flow_lkup_dst.v4_addr = req->flow_lkup_dst().v4_addr();
    fte_span->flow_lkup_proto = req->flow_lkup_proto();
    fte_span->flow_lkup_sport = req->flow_lkup_sport();
    fte_span->flow_lkup_dport = req->flow_lkup_dport();
    fte_span->eth_dmac = req->eth_dmac();
    fte_span->from_cpu = req->from_cpu();
    fte_span->is_egress = req->is_egress();
    fte_span->span_lport = req->span_lport();
    fte_span->attach_stats = req->attach_stats();

    return HAL_RET_OK;
}

static hal_ret_t
fte_span_to_spec (FteSpanRequest* req, fte_span_t *fte_span)
{
    req->set_selector(fte_span->sel);
    req->set_src_lif(fte_span->src_lif);
    req->set_src_lport(fte_span->src_lport);
    req->set_dst_lport(fte_span->dst_lport);
    req->set_span_lport(fte_span->span_lport);
    req->set_drop_reason(fte_span->drop_reason);
    req->set_flow_lkup_dir(fte_span->flow_lkup_dir);
    req->set_flow_lkup_type(fte_span->flow_lkup_type);
    req->set_flow_lkup_vrf(fte_span->flow_lkup_vrf);
    req->mutable_flow_lkup_src()->set_ip_af(fte_span->ip_family);
    req->mutable_flow_lkup_src()->set_v4_addr(fte_span->flow_lkup_src.v4_addr);
    req->mutable_flow_lkup_dst()->set_v4_addr(fte_span->flow_lkup_dst.v4_addr);
    req->set_flow_lkup_proto(fte_span->flow_lkup_proto);
    req->set_flow_lkup_sport(fte_span->flow_lkup_sport);
    req->set_flow_lkup_dport(fte_span->flow_lkup_dport);
    req->set_eth_dmac(fte_span->eth_dmac);
    req->set_from_cpu(fte_span->from_cpu);
    req->set_is_egress(fte_span->is_egress);
    req->set_attach_stats(fte_span->attach_stats);

    return HAL_RET_OK;
}

static hal_ret_t
fte_span_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                        ret = HAL_RET_OK;
    pd::pd_fte_span_create_args_t    pd_fte_span_args = { 0 };
    dllist_ctxt_t                    *lnode = NULL;
    dhl_entry_t                      *dhl_entry = NULL;
    fte_span_t                       *fte_span = NULL;
    pd::pd_func_args_t               pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    fte_span = (fte_span_t *)dhl_entry->obj;

    // PD Call to allocate PD resources and HW programming
    pd_fte_span_args.fte_span           = fte_span;
    pd_fte_span_args.stats_index        = (uint32_t *)cfg_ctxt->app_ctxt;
    pd_func_args.pd_fte_span_create = &pd_fte_span_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_FTE_SPAN_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create fte_span pd, err : {}", ret);
    }
    return ret;
}

static hal_ret_t
fte_span_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret        = HAL_RET_OK;
    dllist_ctxt_t               *lnode     = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    fte_span_t                  *fte_span  = NULL;

    SDK_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    fte_span = (fte_span_t *)dhl_entry->obj;

    g_hal_state->set_fte_span(fte_span);

    return ret;
}

hal_ret_t
fte_span_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t
fte_span_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t fte_span_create(FteSpanRequest& req,
                          FteSpanResponse *rsp)
{
    hal_ret_t       ret = HAL_RET_OK;
    fte_span_t      *fte_span = NULL;
    dhl_entry_t     dhl_entry = { 0 };
    cfg_op_ctxt_t   cfg_ctxt  = { 0 };
    uint32_t        stats_index = 0;

    hal_api_trace(" API Begin: FTE Span create ");
    proto_msg_dump(req);

    ret = validate_fte_span_create(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte span validation failed, err : {}", ret);
        goto end;
    }

    if(g_hal_state->fte_span()) {
        HAL_TRACE_ERR("Failed to create fte span, exists already");
        ret = HAL_RET_ENTRY_EXISTS;
        goto end;
    }

    fte_span = fte_span_alloc_init();
    if (fte_span == NULL) {
        HAL_TRACE_ERR("Failed to alloc/init fte_span");
        ret = HAL_RET_OOM;
        goto end;
    }

    ret = fte_span_init_from_spec(fte_span, &req);
    if (ret != HAL_RET_OK)  {
        HAL_TRACE_ERR("Failed to init fte_span from req");
        goto end;
    }

    fte_span->hal_handle = hal_handle_alloc(HAL_OBJ_ID_FTE_SPAN);
    if (fte_span->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("Failed to alloc handle for fte span");
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        fte_span_cleanup(fte_span);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form ctxt and call infra add
    dhl_entry.handle  = fte_span->hal_handle;
    dhl_entry.obj     = fte_span;
    cfg_ctxt.app_ctxt = &stats_index;
    // cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(fte_span->hal_handle, &cfg_ctxt,
                             fte_span_create_add_cb,
                             fte_span_create_commit_cb,
                             fte_span_create_abort_cb,
                             fte_span_create_cleanup_cb);

end:

    if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
        if (fte_span) {
            fte_span_cleanup(fte_span);
            fte_span = NULL;
        }
        // HAL_API_STATS_INC(HAL_API_FTE_SPAN_CREATE_FAIL);
    } else {
        // HAL_API_STATS_INC(HAL_API_FTE_SPAN_CREATE_SUCCESS);
    }

    fte_span_prepare_rsp(rsp, ret, stats_index);
    return ret;
}

hal_ret_t
validate_fte_span_update (FteSpanRequest& req,
                          FteSpanResponse *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t
fte_span_make_clone (fte_span_t *fte_span, fte_span_t **fte_span_clone)
{
    pd::pd_fte_span_make_clone_args_t args;
    pd::pd_func_args_t                pd_func_args = {0};

    *fte_span_clone = fte_span_alloc_init();
    memcpy(*fte_span_clone, fte_span, sizeof(fte_span_t));

    args.fte_span = fte_span;
    args.clone = *fte_span_clone;
    pd_func_args.pd_fte_span_make_clone = &args;
    pd::hal_pd_call(pd::PD_FUNC_ID_FTE_SPAN_MAKE_CLONE, &pd_func_args);

    return HAL_RET_OK;
}


static hal_ret_t
fte_span_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                        ret = HAL_RET_OK;
    pd::pd_fte_span_update_args_t    pd_fte_span_args = { 0 };
    dllist_ctxt_t                    *lnode = NULL;
    dhl_entry_t                      *dhl_entry = NULL;
    fte_span_t                       /**fte_span  = NULL, */*fte_span_clone = NULL;
    pd::pd_func_args_t               pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    // fte_span = (fte_span_t *)dhl_entry->obj;
    fte_span_clone = (fte_span_t *)dhl_entry->cloned_obj;

    pd_fte_span_args.fte_span_clone = fte_span_clone;
    pd_fte_span_args.stats_index = (uint32_t *)cfg_ctxt->app_ctxt;
    pd_func_args.pd_fte_span_update = &pd_fte_span_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_FTE_SPAN_UPDATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update fte_span pd, err : {}", ret);
    }
    return ret;
}

static hal_ret_t
fte_span_update_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret        = HAL_RET_OK;
    dllist_ctxt_t               *lnode     = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    fte_span_t                  /**fte_span  = NULL, */*fte_span_clone = NULL;

    SDK_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    // fte_span = (fte_span_t *)dhl_entry->obj;
    fte_span_clone = (fte_span_t *)dhl_entry->cloned_obj;

    g_hal_state->set_fte_span(fte_span_clone);

    return ret;
}

hal_ret_t
fte_span_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t
fte_span_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t
fte_span_update(FteSpanRequest& req,
                FteSpanResponse *rsp)
{
    hal_ret_t       ret = HAL_RET_OK;
    fte_span_t      *fte_span = g_hal_state->fte_span();
    dhl_entry_t     dhl_entry = { 0 };
    cfg_op_ctxt_t   cfg_ctxt  = { 0 };
    uint32_t        stats_index = 0;

    hal_api_trace(" API Begin: FTE span update ");
    proto_msg_dump(req);

    ret = validate_fte_span_update(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte span validation failed, err : {}", ret);
        goto end;
    }

    fte_span_make_clone(fte_span, (fte_span_t **)&dhl_entry.cloned_obj);

    ret = fte_span_init_from_spec((fte_span_t *)dhl_entry.cloned_obj, &req);
    if (ret != HAL_RET_OK)  {
        HAL_TRACE_ERR("Failed to init fte_span from req");
        goto end;
    }

    // form ctxt and call infra update object
    dhl_entry.handle  = fte_span->hal_handle;
    dhl_entry.obj     = fte_span;
    cfg_ctxt.app_ctxt = &stats_index;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(fte_span->hal_handle, &cfg_ctxt,
                             fte_span_update_upd_cb,
                             fte_span_update_commit_cb,
                             fte_span_update_abort_cb,
                             fte_span_update_cleanup_cb);

end:

    if (ret == HAL_RET_OK) {
        // HAL_API_STATS_INC(HAL_API_FTE_SPAN_UPDATE_SUCCESS);
    } else {
        // HAL_API_STATS_INC(HAL_API_FTE_SPAN_UPDATE_FAIL);
    }

    fte_span_prepare_rsp(rsp, ret, stats_index);
    return ret;
}

hal_ret_t
fte_span_get(FteSpanResponseMsg *rsp_msg)
{
    pd::pd_fte_span_get_args_t    pd_fte_span_args = { 0 };
    pd::pd_func_args_t               pd_func_args = {0};
    hal_ret_t                        ret = HAL_RET_OK;
    fte_span_t                      *fte_span = g_hal_state->fte_span();
    uint32_t                         stats_index = 0;

    auto response = rsp_msg->add_response();
    FteSpanRequest *req = response->mutable_request();

    ret = fte_span_to_spec(req, fte_span);

    pd_fte_span_args.fte_span = fte_span;
    pd_fte_span_args.stats_index = &stats_index;
    pd_func_args.pd_fte_span_get = &pd_fte_span_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_FTE_SPAN_GET, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update fte_span pd, err : {}", ret);
    }

    response->set_stats_index(stats_index);
    response->set_api_status(types::API_STATUS_OK);

    return ret;
}

//------------------------------------------------------------------------------
// process clock get request
//------------------------------------------------------------------------------
hal_ret_t
clock_get(ClockResponse *response)
{
    hal_ret_t                      ret = HAL_RET_OK;
    pd::pd_func_args_t             pd_func_args = {0};
    pd::pd_clock_detail_get_args_t clock_args;

    pd_func_args.pd_clock_detail_get = &clock_args;
    pd::hal_pd_call(pd::PD_FUNC_ID_CLOCK_DETAIL_GET, &pd_func_args);

    response->mutable_spec()->set_hardware_clock(clock_args.hw_clock);
    response->mutable_spec()->set_software_delta(clock_args.sw_delta);
    response->mutable_spec()->set_software_clock(clock_args.sw_clock);
    response->mutable_spec()->set_clock_op_type((debug::ClockOpType)clock_args.clock_op);

    response->set_api_status(types::API_STATUS_OK);

    return ret;
}

hal_ret_t
hbm_bw_get(const HbmBwGetRequest *req, HbmBwGetResponseMsg *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;
    hbm_bw_samples_t hbm_bw_args;
    sdk_ret_t sdk_ret;

    memset(&hbm_bw_args, 0, sizeof(hbm_bw_samples_t));
    hbm_bw_args.num_samples = req->num_samples();
    hbm_bw_args.sleep_interval = req->sleep_interval();

    hbm_bw_args.hbm_bw =
        (asic_hbm_bw_t*)HAL_CALLOC(HAL_MEM_ALLOC_DEBUG_CLI,
                                    hbm_bw_args.num_samples
                                    * asic_block_t::ASIC_BLOCK_MAX
                                    * sizeof(asic_hbm_bw_t));

    sdk_ret = sdk::asic::pd::asicpd_hbm_bw_get(&hbm_bw_args);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_FREE(HAL_MEM_ALLOC_DEBUG_CLI, hbm_bw_args.hbm_bw);
        return ret;
    }

    for (uint32_t i = 0; i < (hbm_bw_args.num_samples * asic_block_t::ASIC_BLOCK_MAX); i ++) {
        auto response = rsp->add_response();
        auto hbm_bw = &hbm_bw_args.hbm_bw[i];
        response->mutable_average()->set_read(hbm_bw->avg.read);
        response->mutable_average()->set_write(hbm_bw->avg.write);
        response->mutable_maximum()->set_read(hbm_bw->max.read);
        response->mutable_maximum()->set_write(hbm_bw->max.write);
        response->set_clk_diff(hbm_bw->clk_diff);
        response->set_type(debug::CapriBlock(hbm_bw->type));
    }

    HAL_FREE(HAL_MEM_ALLOC_DEBUG_CLI, hbm_bw_args.hbm_bw);

    return ret;
}

hal_ret_t
llc_clear (void)
{
    hal_ret_t ret = HAL_RET_OK;
    sdk_ret_t sdk_ret;
    llc_counters_t llc_args;

    memset (&llc_args, 0, sizeof(llc_counters_t));
    llc_args.mask = 0xffffffff;

    sdk_ret = sdk::asic::pd::asicpd_llc_setup(&llc_args);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    return ret;
}

hal_ret_t
llc_setup(const LlcSetupRequest *req, LlcSetupResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    sdk_ret_t sdk_ret;
    llc_counters_t llc_args;

    memset (&llc_args, 0, sizeof(llc_counters_t));
    if (req->type()) {
        llc_args.mask = (1 << (req->type() - 1)); // Req Type starts at 1 so we need to subtract 1
    }

    sdk_ret = sdk::asic::pd::asicpd_llc_setup(&llc_args);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret == HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_OK);
    } else {
        rsp->set_api_status(types::API_STATUS_ERR);
    }

    return ret;
}

hal_ret_t
llc_get(LlcGetResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    sdk_ret_t sdk_ret;
    llc_counters_t llc_args;

    memset (&llc_args, 0, sizeof(llc_counters_t));

    sdk_ret = sdk::asic::pd::asicpd_llc_get(&llc_args);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_ERR);
        return ret;
    }

    if (llc_args.mask == 0xffffffff) {
        rsp->set_type(debug::LLC_COUNTER_CACHE_NONE);
    } else {
        for (int i = 1; i <= 10; i ++) {
            if ((llc_args.mask & (1 << (i - 1))) == 1) {
                rsp->set_type(debug::LlcCounterType(i));
                break;
            }
        }
    }

    for (int i = 0; i < 16; i ++) {
        rsp->add_count(llc_args.data[i]);
    }
    rsp->set_api_status(types::API_STATUS_OK);

    return ret;
}

hal_ret_t
hbm_cache_setup(HbmCacheRequest *req, HbmCacheResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;

    rsp->set_api_status(types::API_STATUS_OK);

    return ret;
}

hal_ret_t
scheduler_stats_get(debug::SchedulerStatsResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    sdk_ret_t sdk_ret;

    scheduler_stats_t sch_stats;

    sdk_ret = sdk::asic::pd::asicpd_scheduler_stats_get(&sch_stats);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_ERR);
        return ret;
    }

    rsp->set_doorbell_set_count(sch_stats.doorbell_set_count);
    rsp->set_doorbell_clear_count(sch_stats.doorbell_clear_count);
    rsp->set_ratelimit_start_count(sch_stats.ratelimit_start_count);
    rsp->set_ratelimit_stop_count(sch_stats.ratelimit_stop_count);
    for (unsigned i = 0; i < sch_stats.num_coses; i++) {
        auto cos_entry = rsp->add_cos_entry();
        cos_entry->set_cos(sch_stats.cos_stats[i].cos);
        cos_entry->set_doorbell_count(sch_stats.cos_stats[i].doorbell_count);
        cos_entry->set_xon_status(sch_stats.cos_stats[i].xon_status);
    }

    rsp->set_api_status(types::API_STATUS_OK);

    return ret;
}

hal_ret_t
packet_buffer_update (debug::PacketBufferRequest *req, debug::PacketBufferResponse *rsp)
{
    hal_ret_t                           ret = HAL_RET_OK;
    pd::pd_func_args_t                  pd_func_args = {0};
    pd::pd_packet_buffer_update_args_t  pb_args;

    pd_func_args.pd_packet_buffer_update = &pb_args;
    pd::hal_pd_call(pd::PD_FUNC_ID_PACKET_BUFFER_UPDATE, &pd_func_args);

    rsp->set_api_status(types::API_STATUS_OK);

    return ret;
}

hal_ret_t
xcvr_valid_check_enable (bool enable)
{
    return pd::xcvr_valid_check_enable(enable);
}

hal_ret_t
span_threshold_update (uint32_t span_threshold)
{
    hal_ret_t                           ret = HAL_RET_OK;
    pd::pd_func_args_t                  pd_func_args = {0};
    pd::pd_span_threshold_update_args_t span_threshold_args;

    span_threshold_args.span_threshold = span_threshold;
    pd_func_args.pd_span_threshold_update = &span_threshold_args;
    pd::hal_pd_call(pd::PD_FUNC_ID_SPAN_THRESHOLD_UPDATE, &pd_func_args);
    return ret;
}

}    // namespace hal

#include "nic/fte/fte.hpp"
#include "nic/fte/fte_flow.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/fte/fte_impl.hpp"

namespace fte {

// Process Session create in fte thread
hal_ret_t
session_create_in_fte (SessionSpec *spec, SessionStatus *status, SessionStats *stats,
                       SessionResponse *rsp)
{
    hal_ret_t        ret = HAL_RET_OK;
    ctx_t            ctx = {};
    flow_t           iflow[ctx_t::MAX_STAGES], rflow[ctx_t::MAX_STAGES];
    uint16_t         num_features;
    size_t           fstate_size = feature_state_size(&num_features);
    feature_state_t *feature_state;

    feature_state = (feature_state_t*)HAL_MALLOC(hal::HAL_MEM_ALLOC_FTE, fstate_size);
    if (!feature_state) {
        ret = HAL_RET_OOM;
        goto end;
    }

    //Init context
    ret = ctx.init(spec, status, stats, rsp,  iflow, rflow, feature_state, num_features);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failied to init context, ret={}", ret);
        goto end;
    }

    ret = ctx.process();

 end:
    rsp->set_api_status(hal::hal_prepare_rsp(ret));

    if (feature_state) {
        HAL_FREE(hal::HAL_MEM_ALLOC_FTE, feature_state);
    }

    if (ret == HAL_RET_OK && ctx.session())
        rsp->mutable_status()->set_session_handle(ctx.session()->hal_handle);

    return ret;
}

// Process grpc session_create
hal_ret_t
session_create (SessionSpec& spec, SessionResponse *rsp)
{
    struct fn_ctx_t {
        SessionSpec&     spec;
        SessionResponse *rsp;
        hal_ret_t ret;
    } fn_ctx = { spec, rsp, HAL_RET_OK };

    hal::hal_api_trace(" API Begin: Session create ");
    hal::proto_msg_dump(spec);

    fte_execute(0, [](void *data) {
            fn_ctx_t *fn_ctx = (fn_ctx_t *)data;
            fn_ctx->ret = session_create_in_fte(&fn_ctx->spec, NULL, NULL, fn_ctx->rsp);
        }, &fn_ctx);


    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return fn_ctx.ret;
}

// Process vMotion session_create
hal_ret_t
session_create (SessionSpec& spec, SessionStatus& status, SessionStats& stats, SessionResponse *rsp)
{
    struct fn_ctx_t {
        SessionSpec&     spec;
        SessionStatus&   status;
        SessionStats&    stats;
        SessionResponse *rsp;
        hal_ret_t ret;
    } fn_ctx = { spec, status, stats, rsp, HAL_RET_OK };

    hal::hal_api_trace(" API Begin: vMotion Session create ");
    hal::proto_msg_dump(spec);

    fte_execute(0, [](void *data) {
            fn_ctx_t *fn_ctx = (fn_ctx_t *)data;
            fn_ctx->ret = session_create_in_fte(&fn_ctx->spec, &fn_ctx->status,
                                                &fn_ctx->stats, fn_ctx->rsp);
        }, &fn_ctx);


    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return fn_ctx.ret;
}

// Process Session delete in fte thread
hal_ret_t
session_delete_in_fte (hal_handle_t session_handle, bool force_delete)
{
    hal_ret_t ret;
    ctx_t ctx = {};
    uint16_t num_features;
    size_t fstate_size = feature_state_size(&num_features);
    feature_state_t *feature_state = NULL;
    flow_t iflow[ctx_t::MAX_STAGES], rflow[ctx_t::MAX_STAGES];
    hal::session_t *session;

    session = hal::find_session_by_handle(session_handle);
    if (session == NULL) {
        HAL_TRACE_DEBUG("Invalid session handle {}", session_handle);
        return  HAL_RET_HANDLE_INVALID;
    }
    session->deleting = 1;

    HAL_TRACE_DEBUG("fte:: Received session Delete for session id {} force_delete: {}",
                    session->hal_handle, force_delete);

    HAL_TRACE_VERBOSE("num features: {} feature state size: {}", num_features, fstate_size);

    feature_state = (feature_state_t*)HAL_MALLOC(hal::HAL_MEM_ALLOC_FTE, fstate_size);
    if (!feature_state) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Process pkt with db open
    fte::impl::cfg_db_open();
    
    //Init context
    ret = ctx.init(session, iflow, rflow, feature_state, num_features);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failied to init context, ret={}", ret);
        goto end;
    }
    ctx.set_force_delete(force_delete);
    ctx.set_pipeline_event(FTE_SESSION_DELETE);

    ret = ctx.process();

    // close the config db
    fte::impl::cfg_db_close();

end:


    if (feature_state) {
        HAL_FREE(hal::HAL_MEM_ALLOC_FTE, feature_state);
    }
    return ret;
}

hal_ret_t
session_delete_async (hal::session_t *session, bool force_delete)
{
    struct fn_ctx_t {
        hal_handle_t session_handle;
        bool force_delete;
        hal_ret_t ret;
    };
    fn_ctx_t *fn_ctx = (fn_ctx_t *)HAL_MALLOC(hal::HAL_MEM_ALLOC_SESS_DEL_DATA, (sizeof(fn_ctx_t)));

    fn_ctx->session_handle  = session->hal_handle;
    fn_ctx->force_delete = force_delete;
    fn_ctx->ret = HAL_RET_OK;

    fte_softq_enqueue(session->fte_id, [](void *data) {
            fn_ctx_t *fn_ctx = (fn_ctx_t *) data;
            fn_ctx->ret = session_delete_in_fte(fn_ctx->session_handle, fn_ctx->force_delete);
            if (fn_ctx->ret != HAL_RET_OK) {
                HAL_TRACE_DEBUG("session delete in fte failed for handle: {}", fn_ctx->session_handle);
            }
        HAL_FREE(hal::HAL_MEM_ALLOC_SESS_DEL_DATA, fn_ctx);
        }, fn_ctx);

    return fn_ctx->ret;
}

hal_ret_t
session_update_in_fte (hal_handle_t session_handle, uint64_t featureid_bitmap)
{
    hal_ret_t ret;
    ctx_t ctx = {};
    uint16_t num_features;
    size_t fstate_size = feature_state_size(&num_features);
    feature_state_t *feature_state = NULL;
    flow_t iflow[ctx_t::MAX_STAGES], rflow[ctx_t::MAX_STAGES];
    hal::session_t *session;

    session = hal::find_session_by_handle(session_handle);
    if (session == NULL) {
        HAL_TRACE_DEBUG("Invalid session handle {}", session_handle);
        return  HAL_RET_HANDLE_INVALID;
    }

    // Bail if its already getting deleted
    if (session->deleting) {
        return HAL_RET_OK;
    }

    HAL_TRACE_DEBUG("fte:: Received session update for session id {}",
                    session->hal_handle);

    HAL_TRACE_VERBOSE("feature_bitmap: {} num features: {} feature state size: {}", featureid_bitmap, num_features, fstate_size);

    feature_state = (feature_state_t*)HAL_MALLOC(hal::HAL_MEM_ALLOC_FTE, fstate_size);
    if (!feature_state) {
        ret = HAL_RET_OOM;
        goto end;
    }

    //Init context
    ret = ctx.init(session, iflow, rflow, feature_state, num_features);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failed to init context, ret={}", ret);
        goto end;
    }
    ctx.set_pipeline_event(FTE_SESSION_UPDATE);
    ctx.set_featureid_bitmap(featureid_bitmap);

    ret = ctx.process();

end:
    if (feature_state) {
        HAL_FREE(hal::HAL_MEM_ALLOC_FTE, feature_state);
    }
    return ret;
}

hal_ret_t
session_update_in_fte (SessionSpec *spec, SessionStatus *status, SessionStats *stats,
                       SessionResponse *rsp, uint64_t featureid_bitmap)
{
    hal_ret_t        ret;
    ctx_t            ctx = {};
    uint16_t         num_features;
    flow_t           iflow[ctx_t::MAX_STAGES], rflow[ctx_t::MAX_STAGES];
    size_t           fstate_size = feature_state_size(&num_features);
    feature_state_t *feature_state = (feature_state_t*)HAL_MALLOC(hal::HAL_MEM_ALLOC_FTE,
                                                                  fstate_size);
    if (!feature_state) {
        ret = HAL_RET_OOM;
        goto end;
    }

    //Init context
    ret = ctx.init(spec, status, stats, rsp, iflow, rflow, feature_state, num_features);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failed to init context, ret={}", ret);
        goto end;
    }

    ctx.set_pipeline_event(FTE_SESSION_UPDATE);
    ctx.set_featureid_bitmap(featureid_bitmap);

    ret = ctx.process();

end:
    rsp->set_api_status(hal::hal_prepare_rsp(ret));

    if (feature_state) {
        HAL_FREE(hal::HAL_MEM_ALLOC_FTE, feature_state);
    }
    if (ret == HAL_RET_OK && ctx.session()) {
        rsp->mutable_status()->set_session_handle(ctx.session()->hal_handle);
    }
    return ret;
}

// Process vMotion session_update
hal_ret_t
session_update (SessionSpec& spec, SessionStatus& status, SessionStats& stats,
                SessionResponse *rsp, uint64_t feature_bitmap)
{
    struct fn_ctx_t {
        SessionSpec&     spec;
        SessionStatus&   status;
        SessionStats&    stats;
        SessionResponse *rsp;
        uint64_t         featureid_bitmap;
        hal_ret_t ret;
    } fn_ctx = { spec, status, stats, rsp, feature_bitmap, HAL_RET_OK};

    hal::hal_api_trace(" API Begin: vMotion Session update ");
    hal::proto_msg_dump(spec);

    fte_execute(0, [](void *data) {
            fn_ctx_t *fn_ctx = (fn_ctx_t *)data;
            fn_ctx->ret = session_update_in_fte(&fn_ctx->spec, &fn_ctx->status,
                                                &fn_ctx->stats, fn_ctx->rsp,
                                                fn_ctx->featureid_bitmap);
        }, &fn_ctx);


    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return fn_ctx.ret;
}

hal_ret_t
session_update_async (hal::session_t *session, uint64_t featureid_bitmap)
{
    struct fn_ctx_t {
        hal_handle_t session_handle;
        uint64_t     featureid_bitmap;
        hal_ret_t ret;
    };
    fn_ctx_t *fn_ctx = (fn_ctx_t *)HAL_MALLOC(hal::HAL_MEM_ALLOC_SESS_UPD_DATA, (sizeof(fn_ctx_t)));

    fn_ctx->session_handle  = session->hal_handle;
    fn_ctx->featureid_bitmap = featureid_bitmap;
    fn_ctx->ret = HAL_RET_OK;
    
    HAL_TRACE_VERBOSE("Feature id bitmap: {}", fn_ctx->featureid_bitmap);
    fte_softq_enqueue(session->fte_id, [](void *data) {
            fn_ctx_t *fn_ctx = (fn_ctx_t *) data;
            fn_ctx->ret = session_update_in_fte(fn_ctx->session_handle, fn_ctx->featureid_bitmap);
            if (fn_ctx->ret != HAL_RET_OK) {
                HAL_TRACE_ERR("session update in fte failed for handle: {}", fn_ctx->session_handle);
            }
            HAL_FREE(hal::HAL_MEM_ALLOC_SESS_UPD_DATA, fn_ctx);
        }, fn_ctx);

    return fn_ctx->ret;
}

//------------------------------------------------------------------------
// Delete the sepecified session
// Should be called from non-fte thread
//------------------------------------------------------------------------
hal_ret_t
session_delete (hal::session_t *session, bool force_delete)
{
    struct fn_ctx_t {
        hal_handle_t session_handle;
        bool force_delete;
        hal_ret_t ret;
    } fn_ctx = { session->hal_handle, force_delete, HAL_RET_OK};

    fte_execute(session->fte_id, [](void *data) {
            fn_ctx_t *fn_ctx = (fn_ctx_t *) data;
            fn_ctx->ret = session_delete_in_fte(fn_ctx->session_handle, fn_ctx->force_delete);
        }, &fn_ctx);

    return fn_ctx.ret;
}

hal_ret_t
session_delete (SessionDeleteRequest& spec, SessionDeleteResponse *rsp)
{
    hal_ret_t        ret;
    hal::session_t  *session = NULL;

    hal::hal_api_trace(" API Begin: Session delete ");
    hal::proto_msg_dump(spec);

    session = hal::find_session_by_handle(spec.session_handle());
    if (session == NULL) {
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    ret = session_delete(session, true);

end:
    rsp->set_api_status(hal::hal_prepare_rsp(ret));

    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

hal_ret_t
session_get (hal::session_t *session, SessionGetResponse *response)
{
    hal_ret_t        ret;
    ctx_t            ctx = {};
    uint16_t         num_features;
    size_t           fstate_size = feature_state_size(&num_features);
    feature_state_t *feature_state = NULL;
    flow_t           iflow[ctx_t::MAX_STAGES], rflow[ctx_t::MAX_STAGES];

    HAL_TRACE_VERBOSE("--------------------- API Start ------------------------");
    HAL_TRACE_VERBOSE("fte:: Session handle {} Get",
                     session->hal_handle);

    feature_state = (feature_state_t*)HAL_MALLOC(hal::HAL_MEM_ALLOC_FTE, fstate_size);
    if (!feature_state) {
        ret = HAL_RET_OOM;
        goto end;
    }

    //Init context
    ret = ctx.init(session, iflow, rflow, feature_state, num_features);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failed to init context, ret={}", ret);
        goto end;
    }
    ctx.set_pipeline_event(FTE_SESSION_GET);
    ctx.set_sess_get_resp(response);

    // Execute pipeline without any changes to the session
    ctx.set_ignore_session_create(true);

    ret = ctx.process();

end:
    response->set_api_status(hal::hal_prepare_rsp(ret));

    if (feature_state) {
        HAL_FREE(hal::HAL_MEM_ALLOC_FTE, feature_state);
    }

    HAL_TRACE_VERBOSE("----------------------- API End ------------------------");
    return ret;
}

bool
session_is_feature_enabled(hal::session_t *session, const char *feature)
{
    sdk::lib::dllist_ctxt_t   *entry = NULL;
    uint16_t                   num_features;

    if (session == NULL || feature == NULL) {
        HAL_TRACE_ERR("Invalid argumet session: {:p} feature: {:p}",
                       (void *)session, (void *)feature);
        goto end;
    }

    feature_state_size(&num_features);
    dllist_for_each(entry, &session->feature_list_head) {
        feature_session_state_t *state =
            dllist_entry(entry, feature_session_state_t, session_feature_lentry);
        uint16_t id = feature_id(state->feature_name);
        // Look for the feature name in session. If it is
        // present then the assumtion is feature is enabled
        // for the session
        if (id <= num_features &&
            (strstr(state->feature_name, feature) != NULL)) {
            return true;
        }
    }
end:
    return false;
}

} // namespace fte

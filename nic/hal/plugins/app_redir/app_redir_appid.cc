#include "nic/hal/src/session.hpp"
#include "app_redir_plugin.hpp"
#include "app_redir.hpp"
#include "app_redir_scanner.hpp"

using hal::appid_state_t;

namespace hal {
namespace app_redir {

hal_ret_t
appid_scan(fte::ctx_t& ctx)
{
    hal_ret_t ret;
    hal::appid_info_t appid_info;

    fte::appid_info_init(appid_info);
    ret = scanner_run(appid_info, app_redir_pkt(ctx), app_redir_pkt_len(ctx),
                      &ctx.app_redir());
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("scanner_run failed to parse packet");
        goto error;
    }

    if (appid_info.state_ == hal::APPID_STATE_INIT) {
        // scanner_run didn't return flow info, try using flow key
        HAL_TRACE_DEBUG("scanner_run failed to set appid state, try retrieving flow info anyway");
        ret = scanner_get_appid_info(ctx.key(), appid_info);
        if (ret != HAL_RET_OK) {
            goto error;
        }
    }
    ctx.set_appid_info(appid_info);

    // Cleanup Snort flow if we're done scanning
    if (ctx.appid_info().cleanup_handle_ != nullptr) {
        if (ctx.appid_state() == APPID_STATE_FOUND ||
            ctx.appid_state() == APPID_STATE_NOT_NEEDED ||
            ctx.appid_state() == APPID_STATE_ABORT) {
            // Done.  Now cleanup scanner flow resources.
            scanner_cleanup_flow(ctx.appid_info().cleanup_handle_);
            ctx.appid_info().cleanup_handle_ = nullptr;
        }
    }

    return ret;

error:
    ctx.set_appid_state(APPID_STATE_ABORT);
    return ret;
}

hal_ret_t
appid_cleanup_flow(hal::appid_info_t& appid_info)
{
    if (appid_info.cleanup_handle_) {
        scanner_cleanup_flow(appid_info.cleanup_handle_);
        appid_info.cleanup_handle_ = 0;
    }
    return HAL_RET_OK;
}

hal_ret_t
exec_appid_start(fte::ctx_t& ctx)
{
    hal_ret_t ret = HAL_RET_OK;

    // TODO: skip scanning for flow_miss rflow

    ctx.set_appid_state(APPID_STATE_IN_PROGRESS);
    ret = appid_scan(ctx);

    return ret;
}

hal_ret_t
exec_appid_continue(fte::ctx_t& ctx)
{
    hal_ret_t ret = HAL_RET_OK;

    // TODO: add packet counting here, to avoid going past 5 packets max
    // TODO: skip scanning for flow_miss rflow

    ret = appid_scan(ctx);

    return ret;
}

fte::pipeline_action_t
appid_exec(fte::ctx_t& ctx)
{
    hal_ret_t ret = HAL_RET_OK;
    hal::appid_info_t orig_appid_info = ctx.appid_info();

    // Assume appid state will not change, until it does
    ctx.set_appid_updated(false);

    // appid state should have been initialized by DFW
//    assert(ctx.appid_state() != APPID_STATE_INIT);

    switch (ctx.appid_state()) {
    case APPID_STATE_INIT:
        HAL_TRACE_DEBUG("appid state not initialized, skipping appid scanning");
        ret = HAL_RET_OK; // TODO: HAL_RET_ERR
        break;
    case APPID_STATE_NOT_NEEDED:
    case APPID_STATE_NOT_FOUND:
    case APPID_STATE_FOUND:
        HAL_TRACE_DEBUG("appid state not needed, state {}", ctx.appid_state());
        ret = HAL_RET_OK;
        break;
    case APPID_STATE_NEEDED:
        // appid not previously started
        HAL_TRACE_DEBUG("appid state needed, begin scanning pkt");
        ret = exec_appid_start(ctx);
        HAL_TRACE_DEBUG("appid state needed, end scanning pkt, status={}", ret);
        break;
    case APPID_STATE_IN_PROGRESS:
        HAL_TRACE_DEBUG("appid state in progress, begin scanning pkt");
        ret = exec_appid_continue(ctx);
        HAL_TRACE_DEBUG("appid state in progress, end scanning pkt, status={}", ret);
        break;
    default:
        HAL_TRACE_ERR("Unknown appid state {}", ctx.appid_state());
        ret = HAL_RET_ERR;
        break;
    }

    // Update flow if appid state has changed
    if (0 != memcmp((uint8_t*)&orig_appid_info, (uint8_t*)&ctx.appid_info(),
                    sizeof(hal::appid_info_t))) {
        // app state and/or app id
        HAL_TRACE_INFO("appid_info changing from {} to {}", orig_appid_info, ctx.appid_info());
        ctx.set_appid_updated(true);
    }

    if(ctx.flow_miss()) {
        if (ctx.appid_started() && !ctx.appid_completed())
            app_redir_policy_applic_set(ctx);
    } else if(!ctx.appid_updated() && !ctx.appid_completed()) {
        fte::app_redir_ctx_t&   redir_ctx = ctx.app_redir();
        redir_ctx.set_pipeline_end(true);
        return app_redir_exec_fini(ctx);
    }

    ctx.set_feature_status(ret);
    return fte::PIPELINE_CONTINUE;
}

} // namespace app_redir
} // namespace hal

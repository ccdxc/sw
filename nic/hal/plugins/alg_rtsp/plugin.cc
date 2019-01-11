//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "core.hpp"
#include "lib/slab/slab.hpp"
#include "nic/hal/plugins/alg_utils/core.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/iris/include/hal_state.hpp"

namespace hal {
namespace plugins {
namespace alg_rtsp {

alg_utils::alg_state_t *g_rtsp_state;

extern "C" hal_ret_t alg_rtsp_init(hal_cfg_t *hal_cfg) {
    slab *appsess_slab_ = NULL;
    slab *l4sess_slab_ = NULL;
    slab *rtspinfo_slab_ = NULL;
    fte::feature_info_t info = {
        state_size:  0,
        state_init_fn: NULL,
        sess_del_cb: alg_rtsp_session_delete_cb,
        sess_get_cb: alg_rtsp_session_get_cb,
    };
    hal_slab_args_t slab_args;

    fte::register_feature(FTE_FEATURE_ALG_RTSP, alg_rtsp_exec, info);

    appsess_slab_ = g_hal_state->register_slab(HAL_SLAB_RTSP_ALG_APPSESS,
                        slab_args={.name="rtsp_alg_appsess",
                       .size=sizeof(alg_utils::app_session_t), .num_elements=64,
                       .thread_safe=true, .grow_on_demand=true, .zero_on_alloc=true}); 
    HAL_ASSERT_RETURN((appsess_slab_ != NULL), HAL_RET_OOM);

    l4sess_slab_ = g_hal_state->register_slab(HAL_SLAB_RTSP_ALG_L4SESS,
                        slab_args={.name="rtsp_alg_l4sess",
                       .size=sizeof(alg_utils::l4_alg_status_t), .num_elements=64,
                       .thread_safe=true, .grow_on_demand=true, .zero_on_alloc=true});
    HAL_ASSERT_RETURN((l4sess_slab_ != NULL), HAL_RET_OOM);

    rtspinfo_slab_ = g_hal_state->register_slab(HAL_SLAB_RTSP_ALG_INFO,
                        slab_args={.name="rtsp_session",
                       .size=sizeof(rtsp_session_t), .num_elements=64,
                       .thread_safe=true, .grow_on_demand=true, .zero_on_alloc=true}); 
    HAL_ASSERT_RETURN((rtspinfo_slab_ != NULL), HAL_RET_OOM);

    g_rtsp_state = alg_utils::alg_state_t::factory(FTE_FEATURE_ALG_RTSP.c_str(),
                                                   appsess_slab_, l4sess_slab_,
                                                   rtspinfo_slab_, 
                                                   rtsp_app_sess_cleanup_hdlr, NULL);

    return HAL_RET_OK;
}

extern "C" void alg_rtsp_exit() {
    fte::unregister_feature(FTE_FEATURE_ALG_RTSP);
}

}  // namespace alg_rtsp
}  // namespace plugins
}  // namespace hal

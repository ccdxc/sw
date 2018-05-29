//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "core.hpp"
#include "sdk/slab.hpp"
#include "nic/hal/plugins/alg_utils/core.hpp"
#include "nic/include/hal_mem.hpp"

namespace hal {
namespace plugins {
namespace alg_rtsp {

alg_utils::alg_state_t *g_rtsp_state;

extern "C" hal_ret_t alg_rtsp_init(hal_cfg_t *hal_cfg) {
    slab *appsess_slab_ = NULL;
    slab *l4sess_slab_ = NULL;
    slab *rtspinfo_slab_ = NULL;

    fte::register_feature(FTE_FEATURE_ALG_RTSP, alg_rtsp_exec);

    appsess_slab_ = slab::factory("rtsp_alg_appsess", HAL_SLAB_RTSP_ALG_APPSESS,
                                  sizeof(alg_utils::app_session_t), 64,
                                  true, true, true);
    HAL_ASSERT_RETURN((appsess_slab_ != NULL), HAL_RET_OOM);

    l4sess_slab_ = slab::factory("rtsp_alg_l4sess", HAL_SLAB_RTSP_ALG_L4SESS,
                                 sizeof(alg_utils::l4_alg_status_t), 64,
                                 true, true, true);
    HAL_ASSERT_RETURN((l4sess_slab_ != NULL), HAL_RET_OOM);

    rtspinfo_slab_  = slab::factory("rtsp_session", HAL_SLAB_RTSP_ALG_INFO,
                                     sizeof(rtsp_session_t), 64,
                                    true, true, true);
    HAL_ASSERT_RETURN((rtspinfo_slab_ != NULL), HAL_RET_OOM);

    g_rtsp_state = alg_utils::alg_state_t::factory(FTE_FEATURE_ALG_RTSP.c_str(),
                                                   appsess_slab_, l4sess_slab_,
                                                   rtspinfo_slab_, NULL, NULL);

    return HAL_RET_OK;
}

extern "C" void alg_rtsp_exit() {
    fte::unregister_feature(FTE_FEATURE_ALG_RTSP);
}

}  // namespace alg_rtsp
}  // namespace plugins
}  // namespace hal

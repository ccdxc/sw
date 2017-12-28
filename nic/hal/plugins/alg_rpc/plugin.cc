/*
 * plugin.cc
 */

#include "core.hpp"
#include "sdk/slab.hpp"
#include "nic/hal/plugins/alg_utils/core.hpp"
#include "nic/include/hal_mem.hpp"

namespace hal {
namespace plugins {
namespace alg_rpc {

using namespace hal::plugins::alg_utils;

alg_state_t *g_rpc_state;

extern "C" hal_ret_t alg_rpc_init() {
    slab *appsess_slab_ = NULL;
    slab *l4sess_slab_ = NULL;
    slab *rpcinfo_slab_ = NULL;

    fte::register_feature(FTE_FEATURE_ALG_RPC, alg_rpc_exec);
    HAL_TRACE_DEBUG("Registering feature: {}", FTE_FEATURE_ALG_RPC);

    appsess_slab_ = slab::factory("rpc_alg_appsess", HAL_SLAB_RPC_ALG_APPSESS,
                                  sizeof(app_session_t), 64,
                                  true, true, true);
    HAL_ASSERT_RETURN((appsess_slab_ != NULL), HAL_RET_OOM);

    l4sess_slab_ = slab::factory("rpc_alg_l4sess", HAL_SLAB_RPC_ALG_L4SESS,
                                 sizeof(l4_alg_status_t), 64,
                                 true, true, true);
    HAL_ASSERT_RETURN((l4sess_slab_ != NULL), HAL_RET_OOM);

    rpcinfo_slab_  = slab::factory("rpc_alg_l4sess", HAL_SLAB_RPC_ALG_RPCINFO,
                                     sizeof(rpc_info_t), 64,
                                    true, true, true);
    HAL_ASSERT_RETURN((rpcinfo_slab_ != NULL), HAL_RET_OOM);

    g_rpc_state = alg_state_t::factory(FTE_FEATURE_ALG_RPC.c_str(),
                          appsess_slab_, l4sess_slab_, rpcinfo_slab_, NULL,
                          rpcinfo_cleanup_hdlr);

    return HAL_RET_OK;
}

extern "C" void alg_rpc_exit() {
    fte::unregister_feature(FTE_FEATURE_ALG_RPC);
}

}  // namespace alg_rpc
}  // namespace plugins
}  // namespace hal

//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "core.hpp"
#include "nic/sdk/include/sdk/slab.hpp"
#include "nic/hal/plugins/alg_utils/core.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/iris/include/hal_state.hpp"

namespace hal {
namespace plugins {
namespace alg_rpc {

using namespace hal::plugins::alg_utils;

alg_state_t *g_rpc_state;

extern "C" hal_ret_t alg_rpc_init(hal_cfg_t *hal_cfg) {
    slab *appsess_slab_ = NULL;
    slab *l4sess_slab_ = NULL;
    slab *rpcinfo_slab_ = NULL;
    fte::feature_info_t info = {
        state_size:  0,
        state_init_fn: NULL,
        sess_del_cb: alg_rpc_session_delete_cb,
        sess_get_cb: alg_rpc_session_get_cb,
    };
    hal_slab_args_t slab_args;

    fte::register_feature(FTE_FEATURE_ALG_RPC, alg_rpc_exec, info);
    HAL_TRACE_DEBUG("Registering feature: {}", FTE_FEATURE_ALG_RPC);

    appsess_slab_ = g_hal_state->register_slab(HAL_SLAB_RPC_ALG_APPSESS,
                        slab_args={.name="rpc_alg_appsess",
                       .size=sizeof(app_session_t), .num_elements=64,
                       .thread_safe=true, .grow_on_demand=true, .zero_on_alloc=true});
    HAL_ASSERT_RETURN((appsess_slab_ != NULL), HAL_RET_OOM);

    l4sess_slab_ = g_hal_state->register_slab(HAL_SLAB_RPC_ALG_L4SESS,
                        slab_args={.name="rpc_alg_l4sess",
                       .size=sizeof(l4_alg_status_t), .num_elements=64,
                       .thread_safe=true, .grow_on_demand=true, .zero_on_alloc=true});
    HAL_ASSERT_RETURN((l4sess_slab_ != NULL), HAL_RET_OOM);

    rpcinfo_slab_  = g_hal_state->register_slab(HAL_SLAB_RPC_ALG_RPCINFO,
                        slab_args={.name="rpc_alg_rpcinfo",
                       .size=sizeof(rpc_info_t), .num_elements=64,
                       .thread_safe=true, .grow_on_demand=true, .zero_on_alloc=true});
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

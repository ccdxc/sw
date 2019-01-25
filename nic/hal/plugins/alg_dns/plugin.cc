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
namespace alg_dns {

using namespace hal::plugins::alg_utils;

alg_state_t *g_dns_state;

extern "C" hal_ret_t alg_dns_init(hal_cfg_t *hal_cfg) {
    slab *appsess_slab_ = NULL;
    slab *l4sess_slab_ = NULL;
    slab *dnsinfo_slab_ = NULL;
    fte::feature_info_t info = {
        state_size:  0,
        state_init_fn: NULL,
        sess_del_cb: alg_dns_session_delete_cb,
        sess_get_cb: alg_dns_session_get_cb,
    };
    hal_slab_args_t slab_args;

    fte::register_feature(FTE_FEATURE_ALG_DNS, alg_dns_exec, info);
    HAL_TRACE_DEBUG("Registering feature: {}", FTE_FEATURE_ALG_DNS);

    appsess_slab_ = g_hal_state->register_slab(HAL_SLAB_DNS_ALG_APPSESS,
                        slab_args={.name="dns_alg_appsess",
                       .size=sizeof(app_session_t), .num_elements=64,
                       .thread_safe=true, .grow_on_demand=true, .zero_on_alloc=true});
    SDK_ASSERT_RETURN((appsess_slab_ != NULL), HAL_RET_OOM);

    l4sess_slab_ = g_hal_state->register_slab(HAL_SLAB_DNS_ALG_L4SESS,
                        slab_args={.name="dns_alg_l4sess",
                       .size=sizeof(l4_alg_status_t), .num_elements=64,
                       .thread_safe=true, .grow_on_demand=true, .zero_on_alloc=true});
    SDK_ASSERT_RETURN((l4sess_slab_ != NULL), HAL_RET_OOM);

    dnsinfo_slab_  = g_hal_state->register_slab(HAL_SLAB_DNS_ALG_DNSINFO,
                        slab_args={.name="dns_alg_dnsinfo",
                       .size=sizeof(dns_info_t), .num_elements=64,
                       .thread_safe=true, .grow_on_demand=true, .zero_on_alloc=true});
    SDK_ASSERT_RETURN((dnsinfo_slab_ != NULL), HAL_RET_OOM);

    g_dns_state = alg_state_t::factory(FTE_FEATURE_ALG_DNS.c_str(),
                                       appsess_slab_, l4sess_slab_,
                                       dnsinfo_slab_, NULL,
                                       dnsinfo_cleanup_hdlr);

    return HAL_RET_OK;
}

extern "C" void alg_dns_exit() {
    fte::unregister_feature(FTE_FEATURE_ALG_DNS);
}

}  // namespace alg_dns
}  // namespace plugins
}  // namespace hal

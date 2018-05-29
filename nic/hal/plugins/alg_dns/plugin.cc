//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "core.hpp"
#include "sdk/slab.hpp"
#include "nic/hal/plugins/alg_utils/core.hpp"
#include "nic/include/hal_mem.hpp"

namespace hal {
namespace plugins {
namespace alg_dns {

using namespace hal::plugins::alg_utils;

alg_state_t *g_dns_state;

extern "C" hal_ret_t alg_dns_init(hal_cfg_t *hal_cfg) {
    slab *appsess_slab_ = NULL;
    slab *l4sess_slab_ = NULL;
    slab *dnsinfo_slab_ = NULL;

    fte::register_feature(FTE_FEATURE_ALG_DNS, alg_dns_exec);
    HAL_TRACE_DEBUG("Registering feature: {}", FTE_FEATURE_ALG_DNS);

    appsess_slab_ = slab::factory("dns_alg_appsess", HAL_SLAB_DNS_ALG_APPSESS,
                                  sizeof(app_session_t), 64,
                                  true, true, true);
    HAL_ASSERT_RETURN((appsess_slab_ != NULL), HAL_RET_OOM);

    l4sess_slab_ = slab::factory("dns_alg_l4sess", HAL_SLAB_DNS_ALG_L4SESS,
                                 sizeof(l4_alg_status_t), 64,
                                 true, true, true);
    HAL_ASSERT_RETURN((l4sess_slab_ != NULL), HAL_RET_OOM);

    dnsinfo_slab_  = slab::factory("dns_alg_l4sess", HAL_SLAB_DNS_ALG_DNSINFO,
                                   sizeof(dns_info_t), 64,
                                   true, true, true);
    HAL_ASSERT_RETURN((dnsinfo_slab_ != NULL), HAL_RET_OOM);

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

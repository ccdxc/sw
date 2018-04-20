/*
 * plugin.cc
 */

#include "core.hpp"
#include "sdk/slab.hpp"
#include "nic/include/hal_mem.hpp"

namespace hal {
namespace plugins {
namespace nat {

static hal_ret_t
nat_mem_slab_init (void)
{
    return HAL_RET_OK;
}

extern "C" hal_ret_t
nat_init (void)
{
    hal_ret_t ret = HAL_RET_OK;

    fte::register_feature(FTE_FEATURE_NAT, nat_exec);
    HAL_TRACE_DEBUG("Registering feature: {}", FTE_FEATURE_NAT);

    if ((ret = nat_mem_slab_init()) != HAL_RET_OK)
        return ret;

    return ret;
}

extern "C" void
nat_exit()
{
    fte::unregister_feature(FTE_FEATURE_NAT);
}

}  // namespace alg_dns
}  // namespace plugins
}  // namespace hal

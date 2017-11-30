#include "nic/include/fte.hpp"
#include "firewall.hpp"

namespace hal {
namespace firewall {

fte::pipeline_action_t dfw_exec(fte::ctx_t &ctx);



extern "C" hal_ret_t firewall_init() {
    fte::feature_info_t info = {
        state_size: sizeof(firewall_info_t),
    };
    fte::register_feature(FTE_FEATURE_FIREWALL, dfw_exec, info);
    return HAL_RET_OK;
}

extern "C" void firewall_exit() {
    fte::unregister_feature(FTE_FEATURE_FIREWALL);
}

}
}

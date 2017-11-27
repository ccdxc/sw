#include "nic/include/fte.hpp"

namespace hal {
namespace firewall {

fte::pipeline_action_t dfw_exec(fte::ctx_t &ctx);

const std::string FTE_FEATURE_FIREWALL("pensando.io/firewall:firewall");

extern "C" hal_ret_t firewall_init() {
    fte::register_feature(FTE_FEATURE_FIREWALL, dfw_exec);
    return HAL_RET_OK;
}

extern "C" void firewall_exit() {
    fte::unregister_feature(FTE_FEATURE_FIREWALL);
}

}
}

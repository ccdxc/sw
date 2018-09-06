#include "flow_miss/fmiss_learn.hpp"
#include "nic/include/fte.hpp"
#include "nic/hal/plugins/eplearn/eplearn.hpp"
#include "nic/hal/plugins/eplearn/dhcp/dhcp_learn.hpp"
#include "nic/hal/plugins/eplearn/arp/arp_learn.hpp"

namespace hal {
namespace eplearn {

fte::pipeline_action_t ep_learn_exec(fte::ctx_t &ctx);


extern "C" hal_ret_t eplearn_init(hal_cfg_t *hal_cfg) {
    fte::feature_info_t info = {
        state_size: sizeof(eplearn_info_t),
    };
    arp_init();
    dhcp_init();
    flow_miss_learn_init();
    fte::register_feature(FTE_FEATURE_EP_LEARN, ep_learn_exec, info);
    return HAL_RET_OK;
}

extern "C" void eplearn_exit() {
    fte::unregister_feature(FTE_FEATURE_EP_LEARN);
}
}
}


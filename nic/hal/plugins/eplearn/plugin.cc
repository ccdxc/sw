#include "nic/include/fte.hpp"

namespace hal {
namespace network {

fte::pipeline_action_t ep_learn_exec(fte::ctx_t &ctx);

}
}

extern "C" void __plugin_init() {
    fte::register_feature(fte::FTE_FEATURE_EP_LEARN, "net.ep-learn", hal::network::ep_learn_exec);
}

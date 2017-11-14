#include "nic/include/fte.hpp"

namespace hal {
namespace net {

fte::pipeline_action_t alg_exec(fte::ctx_t &ctx);

}
}

extern "C" void __plugin_init() {
    fte::register_feature(fte::FTE_FEATURE_ALG, "net.app-layer-gw", hal::net::alg_exec);
}


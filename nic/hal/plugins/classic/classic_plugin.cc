#include "nic/include/fte.hpp"

namespace hal {
namespace classic {

fte::pipeline_action_t fwding_exec(fte::ctx_t &ctx);

}
}

extern "C" void __plugin_init() {
    fte::register_feature(fte::FTE_FEATURE_CLASSIC_FWDING, "classic.fwding", hal::classic::fwding_exec);
}

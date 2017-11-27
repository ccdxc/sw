#include "nic/include/fte.hpp"

namespace hal {
namespace classic {

fte::pipeline_action_t fwding_exec(fte::ctx_t &ctx);

const std::string FTE_FEATURE_CLASSIC_FWDING("pensando.io/classic:fwding");

extern "C" hal_ret_t classic_init() {
    fte::register_feature(FTE_FEATURE_CLASSIC_FWDING, hal::classic::fwding_exec);
    return HAL_RET_OK;
}

extern "C" void classic_exit() {
    fte::unregister_feature(FTE_FEATURE_CLASSIC_FWDING);
}

}
}


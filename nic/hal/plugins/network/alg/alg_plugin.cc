#include "nic/include/fte.hpp"

namespace hal {
namespace net {

fte::pipeline_action_t alg_exec(fte::ctx_t &ctx);

const std::string FTE_FEATURE_ALG("pensando.io/alg:alg");

extern "C" hal_ret_t alg_init() {
    fte::register_feature(FTE_FEATURE_ALG,  alg_exec);
    return HAL_RET_OK;
}

extern "C" void alg_exit() {
    fte::unregister_feature(FTE_FEATURE_ALG);
}


}
}



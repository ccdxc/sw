#include "nic/include/fte.hpp"

namespace hal {
namespace plugins {
namespace alg_utils {

extern "C" hal_ret_t alg_utils_init() {
    return HAL_RET_OK;
}

extern "C" void alg_utils_exit() {
}

}
}
}

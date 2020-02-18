#include "nic/hal/plugins/cfg/aclqos/init.hpp"
#include "nic/hal/iris/include/hal_state.hpp"

#define HAL_VMOTION_MAX_THREADS \
    (HAL_THREAD_ID_VMOTION_THREADS_MAX - HAL_THREAD_ID_VMOTION_THREADS_MIN + 1)
#define HAL_VMOTION_PORT 50055

namespace hal {

hal_ret_t hal_handle_microseg_enable(void) {
    hal::aclqos::hal_acl_config_init();

    return HAL_RET_OK;
}

}

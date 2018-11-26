#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {

hal_ret_t quiesce_start(void)
{
    hal_ret_t           ret = HAL_RET_OK;
    hal::pd::pd_func_args_t args;

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QUIESCE_START, &args);
    return ret;
}

hal_ret_t quiesce_stop(void)
{
    hal_ret_t           ret = HAL_RET_OK;
    hal::pd::pd_func_args_t args;

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QUIESCE_STOP, &args);
    return ret;
}
} /* hal */

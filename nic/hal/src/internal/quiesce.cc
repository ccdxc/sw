#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/sdk/asic/pd/pd.hpp"

using namespace sdk::asic::pd;

namespace hal {

hal_ret_t
quiesce_start (void)
{
    sdk_ret_t sdk_ret;
    sdk_ret = asicpd_quiesce_start();
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

hal_ret_t
quiesce_stop(void)
{
    sdk_ret_t sdk_ret;
    sdk_ret = asicpd_quiesce_stop();
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

} /* hal */

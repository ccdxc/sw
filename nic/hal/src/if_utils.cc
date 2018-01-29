#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/hal/src/if_utils.hpp"
namespace hal {

// ----------------------------------------------------------------------------
// Get port number from front panel port number
// ----------------------------------------------------------------------------
hal_ret_t pltfm_get_port_from_front_port_num(uint32_t fp_num, 
                                             uint32_t *port_num)
{
    // TODO: Eventually have to call platform api to get the mapping of
    //       front panel number to port number.
    //       Should parse a cataglog file eventually
    *port_num = fp_num - 1;

    return HAL_RET_OK;
}

}    // namespace hal

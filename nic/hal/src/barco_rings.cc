#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/barco_rings.hpp"

namespace hal {

hal_ret_t   GetOpaqueTagAddr(const GetOpaqueTagAddrRequest& request,
    GetOpaqueTagAddrResponse *response)
{
    hal_ret_t           ret = HAL_RET_OK;
    uint64_t addr;

    ret = pd::get_opaque_tag_addr(request.ring_type(), &addr);
    if ((ret != HAL_RET_OK)) {
        response->set_api_status(types::API_STATUS_ERR);
    }
    else {
        response->set_opaque_tag_addr(addr);
        response->set_api_status(types::API_STATUS_OK);
    }
    return ret;
}


}

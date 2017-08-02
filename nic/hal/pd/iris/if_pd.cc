#include <hal_lock.hpp>
#include <pd_api.hpp>
#include <interface_api.hpp>
#include <if_pd.hpp>
#include <uplinkif_pd.hpp>
#include <enicif_pd.hpp>

namespace hal {
namespace pd {

hal_ret_t
pd_if_create (pd_if_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    intf::IfType    if_type;

    HAL_TRACE_DEBUG("PD-If:{}: If Create ", __FUNCTION__);


    if_type = hal::intf_get_if_type(args->intf);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_create(args);
            break;
        case intf::IF_TYPE_UPLINK:
            ret = pd_uplinkif_create(args);
            break;
        case intf::IF_TYPE_UPLINK_PC:
            break;
        case intf::IF_TYPE_TUNNEL:
            break;
        default:
            HAL_ASSERT(0);


    }
    // Branch out for different interface types


    return ret;
}

}    // namespace pd
}    // namespace hal

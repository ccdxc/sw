#include "nic/include/base.hpp"
#include "nic/linkmgr/linkmgr.hpp"

namespace linkmgr {
hal_ret_t port_get(port_args_t *port_args)
{
    printf("Mocking port_get: %d\n", port_args->port_num);
    port_args->oper_status = port_oper_status_t::PORT_OPER_STATUS_UP;
#if 0
    if (port_args->port_num == 1) {
        port_args->oper_status = port_oper_status_t::PORT_OPER_STATUS_UP;
    }
#endif
    return HAL_RET_OK;
}
}


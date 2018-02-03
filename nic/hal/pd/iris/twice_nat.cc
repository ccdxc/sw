#include "nic/include/base.h"
#include <arpa/inet.h>
#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/iris/rw_pd.hpp"
#include "nic/hal/pd/iris/if_pd_utils.hpp"
#include <netinet/ether.h>

namespace hal {
namespace pd {

#if 0
hal_ret_t pd_twice_nat_add(pd_twice_nat_entry_args_t *args,
                           uint32_t *twice_nat_idx)
#endif
hal_ret_t
pd_twice_nat_add(pd_twice_nat_add_args_t *t_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    sdk_ret_t               sdk_ret;
    directmap               *dm;
    twice_nat_actiondata    data = { 0 };
    pd_twice_nat_entry_args_t *args = t_args->args;
    uint32_t *twice_nat_idx = t_args->twice_nat_idx;


    dm = g_hal_state_pd->dm_table(P4TBL_ID_TWICE_NAT);
    HAL_ASSERT(dm != NULL);

    data.actionid = args->twice_nat_act;
    data.twice_nat_action_u.twice_nat_twice_nat_rewrite_info.l4_port = args->nat_l4_port;

    memcpy(data.twice_nat_action_u.twice_nat_twice_nat_rewrite_info.ip, 
           args->nat_ip.addr.v6_addr.addr8, IP6_ADDR8_LEN);

    if (args->nat_ip.af == IP_AF_IPV6) {
        memrev(data.twice_nat_action_u.twice_nat_twice_nat_rewrite_info.ip, 
               sizeof(data.twice_nat_action_u.twice_nat_twice_nat_rewrite_info.ip));
    }

    sdk_ret = dm->insert(&data, twice_nat_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to program twice nat for [nat_ip, l4_port] : [{}, {}]",
                      __FUNCTION__, ipaddr2str(&args->nat_ip), args->nat_l4_port);
    }

    HAL_TRACE_DEBUG("{}: programmed twice nat for [nat_ip, l4_port] : "
                    "[{}, {}] at index:{}",
                    __FUNCTION__, ipaddr2str(&args->nat_ip), args->nat_l4_port,
                    *twice_nat_idx);

    return ret;
}

// hal_ret_t pd_twice_nat_del(pd_twice_nat_entry_args_t *args)
hal_ret_t
pd_twice_nat_del(pd_twice_nat_del_args_t *t_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    sdk_ret_t               sdk_ret;
    directmap               *dm;
    twice_nat_actiondata    data = { 0 };
    pd_twice_nat_entry_args_t *args = t_args->args;

    dm = g_hal_state_pd->dm_table(P4TBL_ID_TWICE_NAT);
    HAL_ASSERT(dm != NULL);

    data.actionid = args->twice_nat_act;
    data.twice_nat_action_u.twice_nat_twice_nat_rewrite_info.l4_port = args->nat_l4_port;

    memcpy(data.twice_nat_action_u.twice_nat_twice_nat_rewrite_info.ip, 
           args->nat_ip.addr.v6_addr.addr8, IP6_ADDR8_LEN);

    if (args->nat_ip.af == IP_AF_IPV6) {
        memrev(data.twice_nat_action_u.twice_nat_twice_nat_rewrite_info.ip, 
               sizeof(data.twice_nat_action_u.twice_nat_twice_nat_rewrite_info.ip));
    }

    sdk_ret = dm->remove(0, &data);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to de-program twice nat for [nat_ip, l4_port] : [{}, {}]",
                      __FUNCTION__, ipaddr2str(&args->nat_ip), args->nat_l4_port);
    }

    HAL_TRACE_DEBUG("{}: de-programmed twice nat for [nat_ip, l4_port] : "
                    "[{}, {}] ",
                    __FUNCTION__, ipaddr2str(&args->nat_ip), args->nat_l4_port);

    return ret;
}

}    // namespace pd
}    // namespace hal

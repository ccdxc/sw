#include "nic/include/base.hpp"
#include <arpa/inet.h>
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/pd/iris/nw/rw_pd.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"
#include <netinet/ether.h>

namespace hal {
namespace pd {

hal_ret_t
pd_twice_nat_add(pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_twice_nat_add_args_t *t_args = pd_func_args->pd_twice_nat_add;
    sdk_ret_t               sdk_ret;
    directmap               *dm;
    twice_nat_actiondata_t    data = { 0 };
    pd_twice_nat_entry_args_t *args = t_args->args;
    uint32_t *twice_nat_idx = t_args->twice_nat_idx;


    dm = g_hal_state_pd->dm_table(P4TBL_ID_TWICE_NAT);
    SDK_ASSERT(dm != NULL);

    data.action_id = args->twice_nat_act;
    data.action_u.twice_nat_twice_nat_rewrite_info.l4_port = args->nat_l4_port;

    memcpy(data.action_u.twice_nat_twice_nat_rewrite_info.ip,
           args->nat_ip.addr.v6_addr.addr8, IP6_ADDR8_LEN);

    if (args->nat_ip.af == IP_AF_IPV6) {
        memrev(data.action_u.twice_nat_twice_nat_rewrite_info.ip,
               sizeof(data.action_u.twice_nat_twice_nat_rewrite_info.ip));
    }

    sdk_ret = dm->insert(&data, twice_nat_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program twice nat for [nat_ip, l4_port] : [{}, {}]",
                      ipaddr2str(&args->nat_ip), args->nat_l4_port);
    }

    HAL_TRACE_DEBUG("Programmed twice nat for [nat_ip, l4_port] : "
                    "[{}, {}] at index:{}",
                    ipaddr2str(&args->nat_ip), args->nat_l4_port,
                    *twice_nat_idx);

    return ret;
}

// hal_ret_t pd_twice_nat_del(pd_twice_nat_entry_args_t *args)
hal_ret_t
pd_twice_nat_del(pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_twice_nat_del_args_t *t_args = pd_func_args->pd_twice_nat_del;
    sdk_ret_t               sdk_ret;
    directmap               *dm;
    twice_nat_actiondata_t    data = { 0 };
    pd_twice_nat_entry_args_t *args = t_args->args;

    dm = g_hal_state_pd->dm_table(P4TBL_ID_TWICE_NAT);
    SDK_ASSERT(dm != NULL);

    data.action_id = args->twice_nat_act;
    data.action_u.twice_nat_twice_nat_rewrite_info.l4_port = args->nat_l4_port;

    memcpy(data.action_u.twice_nat_twice_nat_rewrite_info.ip,
           args->nat_ip.addr.v6_addr.addr8, IP6_ADDR8_LEN);

    if (args->nat_ip.af == IP_AF_IPV6) {
        memrev(data.action_u.twice_nat_twice_nat_rewrite_info.ip,
               sizeof(data.action_u.twice_nat_twice_nat_rewrite_info.ip));
    }

    sdk_ret = dm->remove(0, &data);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to de-program twice nat for [nat_ip, l4_port] : [{}, {}]",
                      ipaddr2str(&args->nat_ip), args->nat_l4_port);
    }

    HAL_TRACE_DEBUG("Deprogrammed twice nat for [nat_ip, l4_port] : "
                    "[{}, {}] ", ipaddr2str(&args->nat_ip), args->nat_l4_port);

    return ret;
}

}    // namespace pd
}    // namespace hal

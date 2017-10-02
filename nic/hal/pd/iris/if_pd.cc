#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/hal/pd/iris/if_pd.hpp"
#include "nic/hal/pd/iris/uplinkif_pd.hpp"
#include "nic/hal/pd/iris/uplinkpc_pd.hpp"
#include "nic/hal/pd/iris/enicif_pd.hpp"
#include "nic/hal/pd/iris/cpuif_pd.hpp"
#include "nic/hal/pd/iris/tunnelif_pd.hpp"
#include "nic/hal/pd/iris/l2seg_uplink_pd.hpp"

namespace hal {
namespace pd {

hal_ret_t
pd_if_create (pd_if_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    intf::IfType    if_type;

    HAL_TRACE_DEBUG("pd-if:{}: if create ", __FUNCTION__);


    if_type = hal::intf_get_if_type(args->intf);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_create(args);
            break;
        case intf::IF_TYPE_UPLINK:
            ret = pd_uplinkif_create(args);
            break;
        case intf::IF_TYPE_UPLINK_PC:
            ret = pd_uplinkpc_create(args);
            break;
        case intf::IF_TYPE_TUNNEL:
            ret = pd_tunnelif_create(args);
            break;
        case intf::IF_TYPE_CPU:
            ret = pd_cpuif_create(args);
            break;
        default:
            HAL_ASSERT(0);
    }
    // Branch out for different interface types
    return ret;
}

hal_ret_t
pd_if_update (pd_if_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    intf::IfType    if_type;

    HAL_TRACE_DEBUG("pd-if:{}: if update", __FUNCTION__);

    if_type = hal::intf_get_if_type(args->intf);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_update(args);
            break;
        case intf::IF_TYPE_UPLINK:
            ret = pd_uplinkif_update(args);
            break;
        case intf::IF_TYPE_UPLINK_PC:
            ret = pd_uplinkpc_update(args);
            break;
        case intf::IF_TYPE_TUNNEL:
            ret = pd_tunnelif_update(args);
            break;
        case intf::IF_TYPE_CPU:
            ret = pd_cpuif_update(args);
            break;
        default:
            HAL_ASSERT(0);
    }

    return ret;
}

hal_ret_t
pd_if_delete (pd_if_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    intf::IfType    if_type;

    HAL_TRACE_DEBUG("pd-if:{}: if delete", __FUNCTION__);

    if_type = hal::intf_get_if_type(args->intf);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_delete(args);
            break;
        case intf::IF_TYPE_UPLINK:
            ret = pd_uplinkif_delete(args);
            break;
        case intf::IF_TYPE_UPLINK_PC:
            ret = pd_uplinkpc_delete(args);
            break;
        case intf::IF_TYPE_TUNNEL:
            ret = pd_tunnelif_delete(args);
            break;
        case intf::IF_TYPE_CPU:
            ret = pd_cpuif_delete(args);
            break;
        default:
            HAL_ASSERT(0);
    }

    return ret;
}

hal_ret_t
pd_if_mem_free (pd_if_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    intf::IfType    if_type;

    HAL_TRACE_DEBUG("pd-if:{}: if mem_free", __FUNCTION__);

    if_type = hal::intf_get_if_type(args->intf);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_mem_free(args);
            break;
        case intf::IF_TYPE_UPLINK:
            ret = pd_uplinkif_mem_free(args);
            break;
        case intf::IF_TYPE_UPLINK_PC:
            ret = pd_uplinkpc_mem_free(args);
            break;
        case intf::IF_TYPE_TUNNEL:
            ret = pd_tunnelif_mem_free(args);
            break;
        case intf::IF_TYPE_CPU:
            ret = pd_cpuif_mem_free(args);
            break;
        default:
            HAL_ASSERT(0);
    }

    return ret;
}
hal_ret_t
pd_if_make_clone (if_t *hal_if, if_t *clone)
{
    hal_ret_t       ret = HAL_RET_OK;
    intf::IfType    if_type;

    HAL_TRACE_DEBUG("pd-if:{}: if clone", __FUNCTION__);

    if_type = hal::intf_get_if_type(hal_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_make_clone(hal_if, clone);
            break;
        case intf::IF_TYPE_UPLINK:
            ret = pd_uplinkif_make_clone(hal_if, clone);
            break;
        case intf::IF_TYPE_UPLINK_PC:
            ret = pd_uplinkpc_make_clone(hal_if, clone);
            break;
        case intf::IF_TYPE_TUNNEL:
            ret = pd_tunnelif_make_clone(hal_if, clone);
            break;
        case intf::IF_TYPE_CPU:
            ret = pd_cpuif_make_clone(hal_if, clone);
            break;
        default:
            HAL_ASSERT(0);
    }

    return ret;
}
//-----------------------------------------------------------------------------
// Nwsec profile update 
//      - Triggered from tenant update
//-----------------------------------------------------------------------------
hal_ret_t
pd_if_nwsec_update(pd_if_nwsec_upd_args_t *args)
{
    hal_ret_t                   ret = HAL_RET_OK;
    intf::IfType                if_type;
    void                        *pd_if = args->intf->pd_if;
    pd_l2seg_uplink_args_t      uplink_args = { 0 };

    HAL_TRACE_DEBUG("pd-if:{}:", __FUNCTION__);

    if_type = hal::intf_get_if_type(args->intf);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_upd_inp_prop_mac_vlan_tbl((pd_enicif_t *)pd_if,
                                                      args->nwsec_prof);
            break;
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC:
            uplink_args.l2seg = args->l2seg;
            uplink_args.intf = args->intf;
            ret = l2set_uplink_upd_input_properties_tbl(&uplink_args, 
                                                        args->nwsec_prof);
            break;
        default:
            HAL_ASSERT(0);
    }

    return ret;
}



// ----------------------------------------------------------------------------
// Returns the encap data and rewrite idx used for l2seg on an if. This is to be called from pd side
// Assumption: Ingress & Egress are same.
// ----------------------------------------------------------------------------
hal_ret_t
if_l2seg_get_encap_rewrite(if_t *pi_if, l2seg_t *pi_l2seg, uint32_t *encap_data,
                           uint32_t *rewrite_idx, uint32_t *tnnl_rewrite_idx)
{
    hal_ret_t ret = HAL_RET_OK;

    HAL_ASSERT_RETURN(pi_if && pi_l2seg && encap_data, HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN(rewrite_idx && tnnl_rewrite_idx, HAL_RET_INVALID_ARG);

    switch(hal::intf_get_if_type(pi_if)) {
        case intf::IF_TYPE_ENIC:
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC:
        {
                uint8_t is_tagged;
                uint16_t vlan_id;

                ret = if_l2seg_get_encap(pi_if, pi_l2seg, &is_tagged, &vlan_id);

                if (ret != HAL_RET_OK) {
                    break;
                }

                (*encap_data) = vlan_id;

                if (is_tagged) {
                    *tnnl_rewrite_idx = TUNNEL_REWRITE_ENCAP_VLAN_ID;
                } else {
                    *tnnl_rewrite_idx = TUNNEL_REWRITE_NOP_ID;
                }

                *rewrite_idx = REWRITE_NOP_ID;
                break;
        }
        case intf::IF_TYPE_TUNNEL:
            // TODO: Handle for Tunnel case
            break;
        default:
            HAL_ASSERT(0);
    }

    return ret;
}

}    // namespace pd
}    // namespace hal

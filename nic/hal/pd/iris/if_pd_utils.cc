// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "if_pd_utils.hpp"
#include "lif_pd.hpp"
#include "enicif_pd.hpp"
#include "uplinkif_pd.hpp"
#include "cpuif_pd.hpp"
#include "app_redir_if_pd.hpp"
#include "tunnelif_pd.hpp"
#include "vrf_pd.hpp"
#include "uplinkpc_pd.hpp"
#include "nwsec_pd.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/p4/nw/include/defines.h"

using namespace hal;

namespace hal {
namespace pd {


// ----------------------------------------------------------------------------
// Given a PI If, return true if it is a CPU If 
// ----------------------------------------------------------------------------
bool
if_is_cpu_if(if_t *pi_if)
{
    intf::IfType    if_type;
    HAL_ASSERT(pi_if != NULL);

    if_type = intf_get_if_type(pi_if);
    return if_type == intf::IF_TYPE_CPU ? true : false;
}

// ----------------------------------------------------------------------------
// Given a PI If, return true if it is an App Redirect If 
// ----------------------------------------------------------------------------
bool
if_is_app_redir_if(if_t *pi_if)
{
    intf::IfType    if_type;
    HAL_ASSERT(pi_if != NULL);

    if_type = intf_get_if_type(pi_if);
    return if_type == intf::IF_TYPE_APP_REDIR ? true : false;
}

// ----------------------------------------------------------------------------
// Given a PI If, return true if it is a Tunnel If 
// ----------------------------------------------------------------------------
bool
if_is_tunnel_if(if_t *pi_if)
{
    intf::IfType    if_type;
    HAL_ASSERT(pi_if != NULL);

    if_type = intf_get_if_type(pi_if);
    return if_type == intf::IF_TYPE_TUNNEL ? true : false;
}

// ----------------------------------------------------------------------------
// Given a PI LIf, get its lport id
// ----------------------------------------------------------------------------
hal_ret_t
pd_lif_get_lport_id(pd_lif_get_lport_id_args_t *args)
{
    // uint32_t        lport_id = 0;
    pd_lif_t        *pd_lif = NULL;
    lif_t *pi_lif = args->pi_lif;


    HAL_ASSERT(pi_lif != NULL);

    pd_lif = (pd_lif_t *)lif_get_pd_lif(pi_lif);
    HAL_ASSERT(pi_lif != NULL);

    args->lport_id =  pd_lif->lif_lport_id;

    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Given a PI If, get its lport id
// ----------------------------------------------------------------------------

uint32_t
if_get_lport_id(if_t *pi_if)
{
    pd_if_get_lport_id_args_t args;
    args.pi_if = pi_if;
    pd_if_get_lport_id(&args);
    return args.lport_id;
}

hal_ret_t
pd_if_get_lport_id(pd_if_get_lport_id_args_t *args)
{
    pd_enicif_t     *pd_enicif = NULL;
    pd_uplinkif_t   *pd_upif = NULL;
    pd_uplinkpc_t   *pd_uppc = NULL;
    pd_cpuif_t      *pd_cpuif = NULL;
    pd_app_redir_if_t *pd_app_redir_if = NULL;
    intf::IfType    if_type;
    // uint32_t        lport_id = 0;
    if_t *pi_if = args->pi_if;

    if (pi_if == NULL) {
        goto end;
    }

    if_type = intf_get_if_type(pi_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            pd_enicif = (pd_enicif_t *)if_get_pd_if(pi_if);
            HAL_ASSERT(pd_enicif!= NULL);

            args->lport_id = pd_enicif->enic_lport_id;
            break;
        case intf::IF_TYPE_UPLINK:
            pd_upif = (pd_uplinkif_t *)if_get_pd_if(pi_if);
            HAL_ASSERT(pd_upif != NULL);

            args->lport_id = pd_upif->upif_lport_id;
            break;
        case intf::IF_TYPE_UPLINK_PC:
            pd_uppc = (pd_uplinkpc_t *)if_get_pd_if((hal::if_t *)pi_if);
            HAL_ASSERT(pd_uppc != NULL);

            args->lport_id = pd_uppc->uppc_lport_id;
            break;
        case intf::IF_TYPE_TUNNEL:
            ep_t *remote_tep_ep;
            if_t *ep_if;
            intf::IfType tif_type;
            remote_tep_ep = find_ep_by_handle(pi_if->rtep_ep_handle);
            ep_if = if_get_if_from_ep(remote_tep_ep);
            tif_type = intf_get_if_type(ep_if);
            HAL_ASSERT(tif_type != intf::IF_TYPE_TUNNEL);
            /* Recursive resolution to get the tunnel LIF */
            args->lport_id = if_get_lport_id(ep_if);
            break;
        case intf::IF_TYPE_CPU:
            pd_cpuif = (pd_cpuif_t *)if_get_pd_if(pi_if);
            HAL_ASSERT(pd_cpuif!= NULL);

            args->lport_id = pd_cpuif->cpu_lport_id;
            break;
        case intf::IF_TYPE_APP_REDIR:
            pd_app_redir_if = (pd_app_redir_if_t *)if_get_pd_if(pi_if);
            HAL_ASSERT(pd_app_redir_if!= NULL);
            args->lport_id = pd_app_redir_if->lport_id;
            break;
        default:
            HAL_ASSERT(0);
    }

end:
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Given a PI If, get its hw lif id
// ----------------------------------------------------------------------------
uint32_t
if_get_hw_lif_id(if_t *pi_if) 
{
    pd_if_get_hw_lif_id_args_t args;
    args.pi_if = pi_if;
    pd_if_get_hw_lif_id(&args);
    return args.hw_lif_id;
}

#if 0
uint32_t
if_get_hw_lif_id(if_t *pi_if) 
#endif
hal_ret_t
pd_if_get_hw_lif_id(pd_if_get_hw_lif_id_args_t *args)
{
    lif_t           *pi_lif = NULL;
    pd_lif_t        *pd_lif = NULL;
    pd_uplinkif_t   *pd_upif = NULL;
    pd_uplinkpc_t   *pd_uppc = NULL;
    intf::IfType    if_type;
    // uint32_t        hw_lif_id = 0;
    if_t *pi_if = args->pi_if;

    HAL_ASSERT(pi_if != NULL);

    if_type = intf_get_if_type(pi_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            pi_lif = if_get_lif(pi_if);
            HAL_ASSERT(pi_lif != NULL);

            pd_lif = (pd_lif_t *)lif_get_pd_lif(pi_lif);
            if (!pd_lif) {
                args->hw_lif_id = INVALID_INDEXER_INDEX;
            }
            HAL_ASSERT(pi_lif != NULL);

            args->hw_lif_id =  pd_lif->hw_lif_id;
            break;
        case intf::IF_TYPE_UPLINK:
            pd_upif = (pd_uplinkif_t *)if_get_pd_if(pi_if);
            HAL_ASSERT(pd_upif != NULL);

            args->hw_lif_id = pd_upif->hw_lif_id;
            break;
        case intf::IF_TYPE_UPLINK_PC:
            pd_uppc = (pd_uplinkpc_t *)if_get_pd_if((hal::if_t *)pi_if);
            HAL_ASSERT(pd_uppc != NULL);

            args->hw_lif_id = pd_uppc->hw_lif_id;
            break;
        case intf::IF_TYPE_TUNNEL:
            ep_t *remote_tep_ep;
            if_t *ep_if;
            intf::IfType tif_type;
            remote_tep_ep = find_ep_by_handle(pi_if->rtep_ep_handle);
            ep_if = if_get_if_from_ep(remote_tep_ep);
            tif_type = intf_get_if_type(ep_if);
            HAL_ASSERT(tif_type != intf::IF_TYPE_TUNNEL);
            /* Recursive resolution to get the tunnel LIF */
            args->hw_lif_id = if_get_hw_lif_id(ep_if);
            break;
        default:
            HAL_ASSERT(0);
    }

    return HAL_RET_OK;
}

#if 0
// ----------------------------------------------------------------------------
// Given a tunnel PI, get the remote TEP EP
// ----------------------------------------------------------------------------
ep_t *
if_get_tunnelif_remote_tep_ep(if_t *pi_if)
{
    ep_t *remote_tep_ep = NULL;

    if (pi_if->encap_type == 
            intf::IfTunnelEncapType::IF_TUNNEL_ENCAP_TYPE_VXLAN) {
        if (pi_if->vxlan_rtep.af == IP_AF_IPV4) {
            remote_tep_ep = find_ep_by_v4_key(pi_if->tid,
                                          pi_if->vxlan_rtep.addr.v4_addr);
            *v4_valid = TRUE;
        } else {
            remote_tep_ep = find_ep_by_v6_key(pi_if->tid, &pi_if->vxlan_rtep);
            *v4_valid = FALSE;
        }
    } else if (pi_if->encap_type == 
            intf::IfTunnelEncapType::IF_TUNNEL_ENCAP_TYPE_GRE) {
        if (pi_if->gre_dest.af == IP_AF_IPV4) {
            remote_tep_ep = find_ep_by_v4_key(pi_if->tid,
                                          pi_if->gre_dest.addr.v4_addr);
            *v4_valid = TRUE;
        } else {
            remote_tep_ep = find_ep_by_v6_key(pi_if->tid, &pi_if->gre_dest);
            *v4_valid = FALSE;
        }
    }

    return remote_tep_ep;
}
#endif

// ----------------------------------------------------------------------------
// Given an EP return PI if 
// ----------------------------------------------------------------------------
if_t *
if_get_if_from_ep(ep_t *ep)
{
    if (!ep)
        return (NULL);
    return (find_if_by_handle(ep->if_handle));
}

// ----------------------------------------------------------------------------
// Given an Uplink If or PC, get its ifpc id
// ----------------------------------------------------------------------------
uint32_t
if_get_uplink_ifpc_id(if_t *pi_if) 
{
    pd_uplinkif_t   *pd_upif = NULL;
    pd_uplinkpc_t   *pd_uppc = NULL;
    intf::IfType    if_type;
    uint32_t        upifpc_id = 0;

    HAL_ASSERT(pi_if != NULL);

    if_type = intf_get_if_type(pi_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            upifpc_id = 0;
            break;
        case intf::IF_TYPE_UPLINK:
            pd_upif = (pd_uplinkif_t *)if_get_pd_if(pi_if);
            HAL_ASSERT(pd_upif != NULL);

            upifpc_id = pd_upif->up_ifpc_id;
            break;
        case intf::IF_TYPE_UPLINK_PC:
            pd_uppc = (pd_uplinkpc_t *)if_get_pd_if((hal::if_t *)pi_if);
            HAL_ASSERT(pd_uppc != NULL);

            upifpc_id = pd_uppc->up_ifpc_id;
            break;
        case intf::IF_TYPE_TUNNEL:
            break;
        default:
            HAL_ASSERT(0);
    }

    return upifpc_id;
}

// ----------------------------------------------------------------------------
// Given an Uplink If or PC, get its lport id
// ----------------------------------------------------------------------------
uint32_t
if_get_uplink_lport_id(if_t *pi_if) 
{
    pd_uplinkif_t   *pd_upif = NULL;
    pd_uplinkpc_t   *pd_uppc = NULL;
    intf::IfType    if_type;
    uint32_t        lport_id = 0;

    HAL_ASSERT(pi_if != NULL);

    if_type = intf_get_if_type(pi_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            break;
        case intf::IF_TYPE_UPLINK:
            pd_upif = (pd_uplinkif_t *)if_get_pd_if(pi_if);
            HAL_ASSERT(pd_upif != NULL);

            lport_id = pd_upif->upif_lport_id;
            break;
        case intf::IF_TYPE_UPLINK_PC:
            pd_uppc = (pd_uplinkpc_t *)if_get_pd_if((hal::if_t *)pi_if);
            HAL_ASSERT(pd_uppc != NULL);

            lport_id = pd_uppc->uppc_lport_id;
            break;
        case intf::IF_TYPE_TUNNEL:
            break;
        default:
            HAL_ASSERT(0);
    }

    return lport_id;
}

// ----------------------------------------------------------------------------
// Given a PI If, get its tm_oport 
// ----------------------------------------------------------------------------
#if 0
uint32_t
if_get_tm_oport(if_t *pi_if) 
#endif
hal_ret_t
pd_if_get_tm_oport(pd_if_get_tm_oport_args_t *args)
{
    intf::IfType               if_type;
    // uint32_t                   tm_port = HAL_PORT_INVALID;
    dllist_ctxt_t              *lnode = NULL;
    hal_handle_id_list_entry_t *entry = NULL;
    if_t                       *pi_up_if;
    ep_t                       *remote_tep_ep;
    if_t                       *ep_if;
    intf::IfType               tif_type;
    pd_if_get_tm_oport_args_t  tmp_args;
    if_t *pi_if = args->pi_if;

    if (pi_if == NULL) {
        goto end;
    }

    if_type = intf_get_if_type(pi_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            args->tm_oport = TM_PORT_DMA;
            break;
        case intf::IF_TYPE_UPLINK:
            args->tm_oport = uplinkif_get_port_num(pi_if);
            break;
        case intf::IF_TYPE_UPLINK_PC:
            dllist_for_each(lnode, &(pi_if->mbr_if_list_head)) {
                entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
                pi_up_if = find_if_by_handle(entry->handle_id);
                // args->tm_oport = if_get_tm_oport(pi_up_if);
                tmp_args.pi_if = pi_up_if;
                pd_if_get_tm_oport(&tmp_args);
                args->tm_oport = tmp_args.tm_oport;
                break;
            }
            break;
        case intf::IF_TYPE_TUNNEL:
            remote_tep_ep = find_ep_by_handle(pi_if->rtep_ep_handle);
            ep_if = if_get_if_from_ep(remote_tep_ep);
            tif_type = intf_get_if_type(ep_if);
            HAL_ASSERT(tif_type != intf::IF_TYPE_TUNNEL);
            /* Recursive resolution to get the tm_oport*/
            // args->tm_oport = if_get_tm_oport(ep_if);
            tmp_args.pi_if = ep_if;
            pd_if_get_tm_oport(&tmp_args);
            args->tm_oport = tmp_args.tm_oport;
            break;
        case intf::IF_TYPE_CPU:
        case intf::IF_TYPE_APP_REDIR:
            args->tm_oport = TM_PORT_DMA;
            break;
        default:
            HAL_ASSERT(0);
    }

end:
    return HAL_RET_OK;
}


// ----------------------------------------------------------------------------
// Get a PI Vrf from if - Applicable only for Enic. Dont call this for Upl.
// ----------------------------------------------------------------------------
vrf_t *
if_get_pi_vrf(if_t *pi_if)
{
    vrf_t        *pi_vrf = NULL;
    intf::IfType    if_type;


    if_type = intf_get_if_type(pi_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            pi_vrf = vrf_lookup_by_id(pi_if->tid);
            HAL_ASSERT_RETURN(pi_vrf != NULL, NULL);
            break;
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC:
            HAL_ASSERT(0);
            break;
        case intf::IF_TYPE_TUNNEL:
            break;
        default:
            HAL_ASSERT(0);
    }
    return pi_vrf;
}

// ----------------------------------------------------------------------------
// Get a PI Vrf from L2 Seg
// ----------------------------------------------------------------------------
vrf_t *
l2seg_get_pi_vrf(l2seg_t *pi_l2seg)
{
    vrf_t    *pi_vrf = NULL;

    pi_vrf = vrf_lookup_by_handle(pi_l2seg->vrf_handle);
    HAL_ASSERT_RETURN(pi_vrf != NULL, NULL);

    return pi_vrf;
}

// ----------------------------------------------------------------------------
// Given a PD tunnel IF, get the tunnel rewrite index
// ----------------------------------------------------------------------------
int
tunnelif_get_rw_idx(pd_tunnelif_t *pd_tif)
{
    pd_tunnelif_get_rw_idx_args_t args;
    args.hal_if = (if_t *)pd_tif->pi_if;
    pd_tunnelif_get_rw_idx(&args);
    return args.tnnl_rw_idx;
}

hal_ret_t
pd_tunnelif_get_rw_idx (pd_tunnelif_get_rw_idx_args_t *args)
{
    pd_tunnelif_t *pd_tif = (pd_tunnelif_t *)args->hal_if->pd_if;

    if (!pd_tif)
        args->tnnl_rw_idx = -1;
    args->tnnl_rw_idx = pd_tif->tunnel_rw_idx;
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Given a PI Vrf, get the nwsec profile hw id
// ----------------------------------------------------------------------------
uint32_t 
ten_get_nwsec_prof_hw_id(vrf_t *pi_ten)
{
    nwsec_profile_t     *pi_nwsec = NULL;
    pd_nwsec_profile_t  *pd_nwsec = NULL;
    uint32_t            nwsec_hw_id = 0;

    pi_nwsec = find_nwsec_profile_by_handle(pi_ten->nwsec_profile_handle);
    if (!pi_nwsec) {
        goto end;
    }

    pd_nwsec = (pd_nwsec_profile_t *)pi_nwsec->pd;

    nwsec_hw_id = pd_nwsec->nwsec_hw_id;

end:
    return nwsec_hw_id;
}

uint32_t
nwsec_get_nwsec_prof_hw_id(nwsec_profile_t *pi_nwsec)
{
    pd_nwsec_profile_t  *pd_nwsec = NULL;

    pd_nwsec = (pd_nwsec_profile_t *)pi_nwsec->pd;
    return pd_nwsec->nwsec_hw_id;
}

hal_ret_t
pd_get_l2seg_ten_masks(uint16_t *l2seg_mask, uint16_t *ten_mask, 
                       uint8_t *ten_shift) 
{
    if (!l2seg_mask || !ten_mask || !ten_shift) {
        return HAL_RET_INVALID_ARG;
    }

    *l2seg_mask = HAL_PD_L2SEG_MASK;
    *ten_mask   = HAL_PD_VRF_MASK;
    *ten_shift  = HAL_PD_VRF_SHIFT;

    return HAL_RET_OK;
}

uint8_t *memrev(uint8_t *block, size_t elnum)
{
    uint8_t *s, *t, tmp;

    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
    return block;
}

}   // namespace pd
}   // namespace hal

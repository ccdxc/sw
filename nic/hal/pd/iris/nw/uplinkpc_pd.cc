#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/nw/interface_api.hpp"
#include "nic/hal/pd/iris/nw/if_pd.hpp"
#include "nic/hal/pd/iris/nw/uplinkpc_pd.hpp"
#include "nic/hal/pd/iris/nw/uplinkif_pd.hpp"
#include "asic/pd/pd.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/hal/pd/iris/nw/l2seg_pd.hpp"

using namespace sdk::asic::pd;

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// Uplink PC Create
// ----------------------------------------------------------------------------
hal_ret_t
pd_uplinkpc_create(pd_if_create_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;;
    pd_uplinkpc_t      *pd_upif;

    HAL_TRACE_DEBUG("if_id:{} creating pd state for if_id",
                    if_get_if_id(args->intf));

    // Create lif PD
    pd_upif = uplinkpc_pd_alloc_init();
    if (pd_upif == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    uplinkpc_link_pi_pd(pd_upif, args->intf);

    // Allocate Resources
    ret = uplinkpc_pd_alloc_res(pd_upif);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("if_id:{} Unable to alloc. resources",
                      if_get_if_id(args->intf));
        goto end;
    }

    // Program HW
    ret = uplinkpc_pd_program_hw(pd_upif);

end:
    if (ret != HAL_RET_OK) {
        uplinkpc_pd_cleanup(pd_upif);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD UplinkPC Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_uplinkpc_update (pd_if_update_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    pd_uplinkpc_t   *pd_uppc = (pd_uplinkpc_t *)args->intf->pd_if;
    if_t        *hal_if = args->intf, *hal_if_clone = args->intf_clone;
    l2seg_t     *curr_nat_l2seg = NULL, *new_nat_l2seg = NULL;

    // update mbr ifs
    if (args->mbrlist_change) {
        // Reprogram output mapping table
        ret = uplinkpc_pd_pgm_output_mapping_tbl(pd_uppc,
                                                 args->aggr_mbrlist,
                                                 TABLE_OPER_UPDATE);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("failed to program "
                          "table:output_mapping, ret:{}", ret);
            goto end;
        }

        // Reprogram tm register
        ret = uplinkpc_pd_upd_tm_register(args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("failed to program "
                          "table:tm_register, ret:{}", ret);
            goto end;
        }
    }

    // checking for native_l2seg change
    if (args->native_l2seg_change) {
        curr_nat_l2seg = find_l2seg_by_id(hal_if->native_l2seg);
        new_nat_l2seg = args->native_l2seg;
        // Current native_l2seg is 0, so skipping deprogramming
        if (hal_if->native_l2seg != 0) {
            // De-pgm input props entry.
            ret = l2seg_uplink_depgm_input_properties_tbl(curr_nat_l2seg, hal_if);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Unable to deprogram input properties table. ret:{}",
                              ret);
                goto end;
            }
        }
        // Update clone with new native l2seg
        hal_if_clone->native_l2seg = new_nat_l2seg->seg_id;
        // Pgm input props entry. Assume clone with have new native l2seg
        ret = l2seg_uplink_pgm_input_properties_tbl(new_nat_l2seg, hal_if_clone);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to program input properties table. ret:{}",
                          ret);
            goto end;
        }
    }
end:
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// PD uplinkpc Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_uplinkpc_delete (pd_if_delete_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_uplinkpc_t    *uplinkpc_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->intf != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->intf->pd_if != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("deleting pd state for uplinkpc {}",
                    args->intf->if_id);
    uplinkpc_pd = (pd_uplinkpc_t *)args->intf->pd_if;

    // deprogram HW
    ret = uplinkpc_pd_deprogram_hw(uplinkpc_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram hw");
    }


    ret = uplinkpc_pd_cleanup(uplinkpc_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed pd uplinkpc delete");
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD Uplinkpc Get
//-----------------------------------------------------------------------------
hal_ret_t
pd_uplinkpc_get (pd_if_get_args_t *args)
{
    hal_ret_t               ret = HAL_RET_OK;
    if_t                    *hal_if = args->hal_if;
    pd_uplinkpc_t           *uplinkpc_pd = (pd_uplinkpc_t *)hal_if->pd_if;
    InterfaceGetResponse    *rsp = args->rsp;;

    auto up_info = rsp->mutable_status()->mutable_uplink_info();
    up_info->set_uplink_lport_id(uplinkpc_pd->uppc_lport_id);
    up_info->set_hw_lif_id(uplinkpc_pd->hw_lif_id);
    up_info->set_uplink_idx(uplinkpc_pd->up_ifpc_id);

    return ret;
}

// ----------------------------------------------------------------------------
// Restoring data post-upgrade
// ----------------------------------------------------------------------------
hal_ret_t
pd_uplinkpc_restore_data (pd_if_restore_args_t *args)
{
    hal_ret_t       ret      = HAL_RET_OK;
    if_t            *hal_if  = args->hal_if;
    pd_uplinkpc_t   *pd_uppc = (pd_uplinkpc_t *)hal_if->pd_if;
    auto up_info             = args->if_status->uplink_info();

    pd_uppc->hw_lif_id      = up_info.hw_lif_id();
    pd_uppc->up_ifpc_id     = up_info.uplink_idx();
    pd_uppc->uppc_lport_id  = up_info.uplink_lport_id();

    return ret;
}

// ----------------------------------------------------------------------------
// Uplink PC Restore
// ----------------------------------------------------------------------------
hal_ret_t
pd_uplinkpc_restore (pd_if_restore_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;;
    pd_uplinkpc_t        *pd_uppc;

    HAL_TRACE_DEBUG("Restoring pd state for if_id: {}",
                    if_get_if_id(args->hal_if));

    // Create Uplink PC
    pd_uppc = uplinkpc_pd_alloc_init();
    if (pd_uppc == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    uplinkpc_link_pi_pd(pd_uppc, args->hal_if);

    // Restore PD info
    ret = pd_uplinkpc_restore_data(args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to restore PD data for IF: {}, err:{}",
                      if_get_if_id(args->hal_if), ret);
        goto end;
    }

    // Program HW
    ret = uplinkpc_pd_program_hw(pd_uppc, true);

end:
    if (ret != HAL_RET_OK) {
        uplinkpc_pd_cleanup(pd_uppc);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD Uplink if
// ----------------------------------------------------------------------------
hal_ret_t
uplinkpc_pd_alloc_res(pd_uplinkpc_t *pd_upif)
{
    hal_ret_t            ret = HAL_RET_OK;
    indexer::status      rs = indexer::SUCCESS;

    // Allocate lif hwid
#if 0
    rs = g_hal_state_pd->lif_hwid_idxr()->alloc((uint32_t *)&pd_upif->hw_lif_id);
    if (rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("failed to alloc hw_lif_id err: {}",
                      rs);
        pd_upif->hw_lif_id = INVALID_INDEXER_INDEX;
        return HAL_RET_NO_RESOURCE;
    }
#endif
    pd_upif->hw_lif_id = if_allocate_hwlif_id();
    if (pd_upif->hw_lif_id == INVALID_INDEXER_INDEX) {
        HAL_TRACE_ERR("failed to alloc hw_lif_id err: {}", rs);
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("if_id:{} allocated hw_lif_id:{}",
                    if_get_if_id((if_t *)pd_upif->pi_if),
                    pd_upif->hw_lif_id);

    // Allocate ifpc id
    rs = g_hal_state_pd->uplinkifpc_hwid_idxr()->
        alloc((uint32_t *)&pd_upif->up_ifpc_id);
    if (rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("failed to alloc uplink_ifpc_id err: {}", rs);
        pd_upif->up_ifpc_id = INVALID_INDEXER_INDEX;
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("if_id:{} allocated uplink_ifpc_id : {}",
                    if_get_if_id((if_t *)pd_upif->pi_if),
                    pd_upif->up_ifpc_id);

    // Allocate lport
    rs = g_hal_state_pd->lport_idxr()->alloc((uint32_t *)&pd_upif->
            uppc_lport_id);
    if (rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("failed to alloc uplink_ifpc_id err: {}", rs);
        pd_upif->uppc_lport_id = INVALID_INDEXER_INDEX;
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("if_id:{} allocated lport_id:{}",
                    if_get_if_id((if_t *)pd_upif->pi_if),
                    pd_upif->uppc_lport_id);
    return ret;
}

//-----------------------------------------------------------------------------
// De-Allocate resources.
//-----------------------------------------------------------------------------
hal_ret_t
uplinkpc_pd_dealloc_res(pd_uplinkpc_t *upif_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs;

    if (upif_pd->hw_lif_id != INVALID_INDEXER_INDEX) {
#if 0
        rs = g_hal_state_pd->lif_hwid_idxr()->free(upif_pd->hw_lif_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("failed to free hw_lif_id err: {}",
                          upif_pd->hw_lif_id);
            ret = HAL_RET_INVALID_OP;
            goto end;
        }
#endif
        // TODO: Have to free up the index from lif manager

        HAL_TRACE_DEBUG("freed hw_lif_id: {}",
                        upif_pd->hw_lif_id);
    }

    if (upif_pd->up_ifpc_id != INVALID_INDEXER_INDEX) {
        rs = g_hal_state_pd->uplinkifpc_hwid_idxr()->free(upif_pd->up_ifpc_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("failed to free uplink_ifpc_id err: {}",
                          upif_pd->up_ifpc_id);
            ret = HAL_RET_INVALID_OP;
            goto end;
        }

        HAL_TRACE_DEBUG("freed uplink_ifpc_id: {}",
                        upif_pd->up_ifpc_id);
    }

    if (upif_pd->uppc_lport_id != INVALID_INDEXER_INDEX) {
        rs = g_hal_state_pd->lport_idxr()->free(upif_pd->uppc_lport_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("failed to free lport_id err: {}",
                          upif_pd->uppc_lport_id);
            ret = HAL_RET_INVALID_OP;
            goto end;
        }

        HAL_TRACE_DEBUG("freed lport_id: {}",
                        upif_pd->uppc_lport_id);
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// PD Uplinkpc Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD If
//  Note:
//      - Just free up whatever PD has.
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
uplinkpc_pd_cleanup(pd_uplinkpc_t *upif_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!upif_pd) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = uplinkpc_pd_dealloc_res(upif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to dealloc res for uplinkpc: {}",
                      ((if_t *)(upif_pd->pi_if))->if_id);
        goto end;
    }

    // Delinking PI<->PD
    uplinkpc_delink_pi_pd(upif_pd, (if_t *)upif_pd->pi_if);

    // Freeing PD
    uplinkpc_pd_free(upif_pd);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram HW
// ----------------------------------------------------------------------------
hal_ret_t
uplinkpc_pd_deprogram_hw (pd_uplinkpc_t *pd_upif)
{
    hal_ret_t            ret = HAL_RET_OK;

#if 0
    // De program TM register
    ret = uplinkpc_pd_depgm_tm_register(pd_upif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram hw");
    }
#endif

    // De-Program Output Mapping Table
    ret = uplinkpc_pd_depgm_output_mapping_tbl(pd_upif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram hw");
    }
    return ret;
}

#if 0
// ----------------------------------------------------------------------------
// De-Program TM Register
// ----------------------------------------------------------------------------
hal_ret_t
uplinkpc_pd_depgm_tm_register(pd_uplinkpc_t *pd_upif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    uint8_t                     tm_oport = 0;

    tm_oport = uplinkpc_get_port_num((if_t *)(pd_upif->pi_if));

    ret = asicpd_tm_uplink_lif_set(tm_oport, 0);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram for if_id: {}",
                      if_get_if_id((if_t *)pd_upif->pi_if));
    } else {
        HAL_TRACE_DEBUG("deprogrammed for if_id: {} "
                        "iport:{} => hwlif: {}",
                        if_get_if_id((if_t *)pd_upif->pi_if),
                        tm_oport, 0);
    }

    return ret;
}
#endif

// ----------------------------------------------------------------------------
// DeProgram output mapping table for cpu tx traffic
// ----------------------------------------------------------------------------
hal_ret_t
uplinkpc_pd_depgm_output_mapping_tbl (pd_uplinkpc_t *pd_upif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    directmap                   *dm_omap = NULL;

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    SDK_ASSERT_RETURN((dm_omap != NULL), HAL_RET_ERR);

    sdk_ret = dm_omap->remove(pd_upif->uppc_lport_id);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram omapping table. index:{}",
                      pd_upif->uppc_lport_id);
    } else {
        HAL_TRACE_DEBUG("deprogrammed omapping table. index:{}",
                      pd_upif->uppc_lport_id);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
uplinkpc_pd_program_hw(pd_uplinkpc_t *pd_upif, bool is_upgrade)
{
    hal_ret_t            ret;
    if_t                 *pi_if = NULL;

    pi_if = (if_t *)(pd_upif->pi_if);

    // Upgrade: There is no table lib for this. Does only HW programming.
    if (!is_upgrade) {
        ret = uplinkpc_pd_pgm_tm_register(pd_upif, true);
        SDK_ASSERT_RETURN(ret == HAL_RET_OK, ret);
    }

    /*
     * Program Output Mapping Table.
     * Upgrade: No need to send the flag. Already doing insert_withid, so lib
     *          will be populated and since we are in upgrade HW writes will
     *          not happen.
     */
    ret = uplinkpc_pd_pgm_output_mapping_tbl(pd_upif,
                                             pi_if->mbr_if_list,
                                             TABLE_OPER_INSERT);
    SDK_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    // TODO: Un-program Output Mapping Table and
    //       tm table program for member port

    return ret;
}

// ----------------------------------------------------------------------------
// ReProgram TM Register
// ----------------------------------------------------------------------------
hal_ret_t
uplinkpc_pd_upd_tm_register (pd_if_update_args_t *args)
{
    hal_ret_t                   ret = HAL_RET_OK;
    if_t                        *pi_up_if = NULL;
    pd_uplinkpc_t               *pd_uppcif = (pd_uplinkpc_t *)args->intf->pd_if;
    hal_handle_t                *p_hdl_id = NULL;

    // Walk the newly added mbr list
    for (const void *ptr : *args->add_mbrlist) {
        p_hdl_id = (hal_handle_t *)ptr;
        pi_up_if = find_if_by_handle(*p_hdl_id);

        ret = uplinkpc_pd_pgm_tm_register_per_upif(pd_uppcif,
                (pd_uplinkif_t *)pi_up_if->pd_if, true);
        if (ret != HAL_RET_OK) {
            continue;
        }
    }

    // Walk the deleted mbr list
    for (const void *ptr : *args->del_mbrlist) {
        p_hdl_id = (hal_handle_t *)ptr;
        pi_up_if = find_if_by_handle(*p_hdl_id);

        ret = uplinkpc_pd_pgm_tm_register_per_upif(pd_uppcif,
                (pd_uplinkif_t *)pi_up_if->pd_if, false);
        if (ret != HAL_RET_OK) {
            continue;
        }
    }

    return ret;
}


// ----------------------------------------------------------------------------
// Program TM Register
// ----------------------------------------------------------------------------
hal_ret_t
uplinkpc_pd_pgm_tm_register(pd_uplinkpc_t *pd_uppcif, bool add)
{
    hal_ret_t                   ret = HAL_RET_OK;
    if_t                        *pi_if = NULL, *pi_up_if = NULL;
    hal_handle_t                *p_hdl_id = NULL;

    pi_if = (if_t *)(pd_uppcif->pi_if);

    // Walk the member ports and set the lif for each uplink
    for (const void *ptr : *pi_if->mbr_if_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        pi_up_if = find_if_by_handle(*p_hdl_id);

        ret = uplinkpc_pd_pgm_tm_register_per_upif(pd_uppcif,
                (pd_uplinkif_t *)pi_up_if->pd_if, add);
        if (ret != HAL_RET_OK) {
            continue;
        }
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Program TM Register per uplink if
// ----------------------------------------------------------------------------
hal_ret_t
uplinkpc_pd_pgm_tm_register_per_upif(pd_uplinkpc_t *pd_uppc,
                                     pd_uplinkif_t *pd_upif, bool add)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    uint8_t                     tm_oport = 0;
    if_t                        *pi_if = NULL;
    uint32_t                    hw_lif_id = 0;

    pi_if = (if_t *)(pd_uppc->pi_if);

    if (add) {
        // Get hw_lif_id from PC
        hw_lif_id = pd_uppc->hw_lif_id;
    } else {
        // Get hw_lif_id from Mbr If
        hw_lif_id = pd_upif->hw_lif_id;
    }
    tm_oport = uplinkif_get_port_num((if_t *)(pd_upif->pi_if));

    sdk_ret = asicpd_tm_uplink_lif_set(tm_oport, hw_lif_id);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("if_id:{} unable to program for if_id",
                      if_get_if_id(pi_if));
    } else {
        HAL_TRACE_DEBUG("if_id:{} programmed "
                        "iport:{} => hwlif: {}",
                        if_get_if_id(pi_if), tm_oport, hw_lif_id);
    }

    return ret;
}

#define om_tmoport data.action_u.output_mapping_set_tm_oport

// ----------------------------------------------------------------------------
// Form info for Output Mapping Table
// ----------------------------------------------------------------------------
hal_ret_t
uplinkpc_pd_form_mbr_info_omap_table (block_list *mbr_list,
                                      output_mapping_actiondata_t &data)
{
    uint8_t                     *tm_oport = NULL;
    if_t                        *pi_up_if = NULL;
    hal_handle_t                *p_hdl_id = NULL;

    tm_oport = &om_tmoport.egress_port1;
    for (const void *ptr : *mbr_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        pi_up_if = find_if_by_handle(*p_hdl_id);
        *tm_oport = uplinkif_get_port_num(pi_up_if);

        om_tmoport.nports++;
        tm_oport++;
    }

    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Program Output Mapping Table
// ----------------------------------------------------------------------------
hal_ret_t
uplinkpc_pd_pgm_output_mapping_tbl(pd_uplinkpc_t *pd_uppcif,
                                   block_list *mbr_list,
                                   table_oper_t oper)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    // uint8_t                     *tm_oport = NULL;
    // if_t                        *pi_if = NULL, *pi_up_if = NULL;
    output_mapping_actiondata_t   data;
    directmap                   *dm_omap = NULL;
    // dllist_ctxt_t               *curr, *next;
    // hal_handle_id_list_entry_t  *entry = NULL;

    memset(&data, 0, sizeof(data));
    // tm_oport = &om_tmoport.egress_port1;

    // pi_if = (if_t *)(pd_uppcif->pi_if);

    data.action_id = OUTPUT_MAPPING_SET_TM_OPORT_ID;
    om_tmoport.nports = 0;
    om_tmoport.egress_mirror_en = 1;

    uplinkpc_pd_form_mbr_info_omap_table(mbr_list, data);
#if 0
    // Walk the member ports and set the lif for each uplink
    dllist_for_each_safe(curr, next, &pi_if->mbr_if_list_head) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        pi_up_if = find_if_by_handle(entry->handle_id);
        *tm_oport = uplinkif_get_port_num(pi_up_if);

        om_tmoport.nports++;
        tm_oport++;
    }
#endif
    om_tmoport.dst_lif = pd_uppcif->hw_lif_id;

    // Program OutputMapping table
    //  - Get vlan_tagid_in_skb from the fwding mode:
    //      - Classic: TRUE
    //      - Switch : FALSE

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    SDK_ASSERT_RETURN((dm_omap != NULL), HAL_RET_ERR);


    if (oper == TABLE_OPER_INSERT) {
        sdk_ret = dm_omap->insert_withid(&data, pd_uppcif->uppc_lport_id);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{} unable to program. ret:{}",
                          oper, ret);
        } else {
            HAL_TRACE_DEBUG("{} programmed "
                            "table:output_mapping index:{}",
                            oper,
                            pd_uppcif->uppc_lport_id);
        }
    } else {
        sdk_ret = dm_omap->update(pd_uppcif->uppc_lport_id, &data);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{} unable to "
                          "program table:output_mapping. ret:{}",
                          oper, ret);
        } else {
            HAL_TRACE_DEBUG("{} programmed table:output_mapping index:{}",
                            oper,
                            pd_uppcif->uppc_lport_id);
        }
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
// pd_uplinkpc_make_clone(if_t *hal_if, if_t *clone)
pd_uplinkpc_make_clone(pd_if_make_clone_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_uplinkpc_t       *pd_uplinkpc_clone = NULL;
    if_t *hal_if = args->hal_if;
    if_t *clone = args->clone;

    pd_uplinkpc_clone = uplinkpc_pd_alloc_init();
    if (pd_uplinkpc_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_uplinkpc_clone, hal_if->pd_if, sizeof(pd_uplinkpc_t));

    uplinkpc_link_pi_pd(pd_uplinkpc_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_uplinkpc_mem_free(pd_if_mem_free_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_uplinkpc_t  *upif_pd;

    upif_pd = (pd_uplinkpc_t *)args->intf->pd_if;
    uplinkpc_pd_mem_free(upif_pd);

    return ret;
}


}    // namespace pd
}    // namespace hal

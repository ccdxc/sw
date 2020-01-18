#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/nw/interface_api.hpp"
#include "nic/hal/pd/iris/nw/if_pd.hpp"
#include "nic/hal/pd/iris/nw/uplinkif_pd.hpp"
#include "platform/capri/capri_tm_rw.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/hal/pd/iris/nw/l2seg_pd.hpp"

using namespace sdk::platform::capri;

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// Uplink If Create
// ----------------------------------------------------------------------------
hal_ret_t
pd_uplinkif_create(pd_if_create_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;;
    pd_uplinkif_t        *pd_upif;

    HAL_TRACE_DEBUG("creating pd state for if_id: {}",
                    if_get_if_id(args->intf));

    // Create lif PD
    pd_upif = uplinkif_pd_alloc_init();
    if (pd_upif == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    uplinkif_link_pi_pd(pd_upif, args->intf);

    // Allocate Resources
    ret = uplinkif_pd_alloc_res(pd_upif);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("unable to alloc. resources for if_id: {}",
                      if_get_if_id(args->intf));
        goto end;
    }

    // Program HW
    ret = uplinkif_pd_program_hw(pd_upif);

end:
    if (ret != HAL_RET_OK) {
        uplinkif_pd_cleanup(pd_upif);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD Uplinkif Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_uplinkif_update (pd_if_update_args_t *args)
{
    hal_ret_t   ret = HAL_RET_OK;
    if_t        *hal_if = args->intf, *hal_if_clone = args->intf_clone;
    l2seg_t     *curr_nat_l2seg = NULL, *new_nat_l2seg = NULL;

    // checking for native_l2seg change
    if (args->native_l2seg_change) {
        curr_nat_l2seg = find_l2seg_by_id(hal_if->native_l2seg);
        new_nat_l2seg = args->native_l2seg;

        if (hal_if->native_l2seg != 0) {
            // Check if if is present in l2seg mbrifs
            if (l2seg_is_mbr_if(curr_nat_l2seg, hal_if->if_id)) {
                // De-pgm inp. props entry
                ret = l2seg_uplink_depgm_input_properties_tbl(curr_nat_l2seg, hal_if);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("Unable to deprogram input properties table. ret:{}",
                                  ret);
                    goto end;
                }
            }
        }
        hal_if_clone->native_l2seg = 0;
        if (new_nat_l2seg) {
            // Update clone with new native l2seg
            hal_if_clone->native_l2seg = new_nat_l2seg->seg_id;
            if (l2seg_is_mbr_if(new_nat_l2seg, hal_if->if_id)) {
                // De-pgm input props entry with old native property
                ret = l2seg_uplink_depgm_input_properties_tbl(new_nat_l2seg, hal_if_clone);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("Unable to deprogram input properties table. ret:{}",
                                  ret);
                    goto end;
                }
                // Pgm input props entry. Assume clone with have new native l2seg
                ret = l2seg_uplink_pgm_input_properties_tbl(new_nat_l2seg, hal_if_clone);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("Unable to program input properties table. ret:{}",
                                  ret);
                    goto end;
                }
            }
        }

#if 0
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
#endif
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// PD Uplinkif Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_uplinkif_delete (pd_if_delete_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_uplinkif_t  *uplinkif_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->intf != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->intf->pd_if != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("deleting pd state for uplinkif {}",
                    args->intf->if_id);
    uplinkif_pd = (pd_uplinkif_t *)args->intf->pd_if;

    // deprogram HW
    ret = uplinkif_pd_deprogram_hw(uplinkif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram hw");
    }

    // pd cleanup
    ret = uplinkif_pd_cleanup(uplinkif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed pd uplinkif delete");
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD Uplinkif Get
//-----------------------------------------------------------------------------
hal_ret_t
pd_uplinkif_get (pd_if_get_args_t *args)
{
    hal_ret_t               ret = HAL_RET_OK;
    if_t                    *hal_if = args->hal_if;
    pd_uplinkif_t           *uplinkif_pd = (pd_uplinkif_t *)hal_if->pd_if;
    InterfaceGetResponse    *rsp = args->rsp;

    auto up_info = rsp->mutable_status()->mutable_uplink_info();
    up_info->set_uplink_lport_id(uplinkif_pd->upif_lport_id);
    up_info->set_hw_lif_id(uplinkif_pd->hw_lif_id);
    up_info->set_uplink_idx(uplinkif_pd->up_ifpc_id);

    return ret;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD Uplink if
// ----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_alloc_res(pd_uplinkif_t *pd_upif)
{
    hal_ret_t            ret = HAL_RET_OK;
    indexer::status      rs = indexer::SUCCESS;

    // Allocate lif hwid
#if 0
    rs = g_hal_state_pd->lif_hwid_idxr()->
        alloc((uint32_t *)&pd_upif->hw_lif_id);
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
    HAL_TRACE_DEBUG("if_id:{} allocated hw_lif_id: {}",
                    if_get_if_id((if_t *)pd_upif->pi_if),
                    pd_upif->hw_lif_id);

    // TODO: The macro of 32 is define in dev.hpp in platform.
    //       Needs to work on a single place to define the id splits.
    // Nicmgr uses hw_lif_ids of above 32 for host lifs.

    // nic/agent jobd is creating lots of lifs without with_hw_lif_id
    // which are being allocated in hal. After those lifs an uplink
    // is being created and it gets a large hw_lif_id.
    // So having this check only in hw.
    if (is_platform_type_haps() || is_platform_type_hw()) {
        SDK_ASSERT_RETURN(pd_upif->hw_lif_id <= 32, HAL_RET_NO_RESOURCE);
    }


    // Allocate ifpc id
    rs = g_hal_state_pd->uplinkifpc_hwid_idxr()->
        alloc((uint32_t *)&pd_upif->up_ifpc_id);
    if (rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("failed to alloc uplink_ifpc_id err: {}", rs);
        pd_upif->up_ifpc_id = INVALID_INDEXER_INDEX;
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("if_id:{} allocated uplink_ifpc_id: {}",
                    if_get_if_id((if_t *)pd_upif->pi_if),
                    pd_upif->up_ifpc_id);

    // Allocate lport
    rs = g_hal_state_pd->lport_idxr()->alloc((uint32_t *)&pd_upif->
            upif_lport_id);
    if (rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("failed to alloc uplink_ifpc_id err: {}", rs);
        pd_upif->upif_lport_id = INVALID_INDEXER_INDEX;
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("if_id:{} allocated lport_id:{}",
                    if_get_if_id((if_t *)pd_upif->pi_if),
                    pd_upif->upif_lport_id);
    return ret;
}

// ----------------------------------------------------------------------------
// Restoring data post-upgrade
// ----------------------------------------------------------------------------
hal_ret_t
pd_uplinkif_restore_data (pd_if_restore_args_t *args)
{
    hal_ret_t       ret      = HAL_RET_OK;
    if_t            *hal_if  = args->hal_if;
    pd_uplinkif_t   *pd_upif = (pd_uplinkif_t *)hal_if->pd_if;
    auto up_info             = args->if_status->uplink_info();

    pd_upif->hw_lif_id      = up_info.hw_lif_id();
    pd_upif->up_ifpc_id     = up_info.uplink_idx();
    pd_upif->upif_lport_id  = up_info.uplink_lport_id();
    hal_if->uplink_port_num = up_info.hw_port_num(); // TODO: Shud it be in PI?

    return ret;
}

// ----------------------------------------------------------------------------
// Uplink If Restore
// ----------------------------------------------------------------------------
hal_ret_t
pd_uplinkif_restore (pd_if_restore_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;
    pd_uplinkif_t        *pd_upif;

    HAL_TRACE_DEBUG("Restoring pd state for if_id: {}",
                    if_get_if_id(args->hal_if));

    // Create Uplink if
    pd_upif = uplinkif_pd_alloc_init();
    if (pd_upif == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    uplinkif_link_pi_pd(pd_upif, args->hal_if);

    // Restore PD info
    ret = pd_uplinkif_restore_data(args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to restore PD data for IF: {}, err:{}",
                      if_get_if_id(args->hal_if), ret);
        goto end;
    }

    // Program HW
    ret = uplinkif_pd_program_hw(pd_upif);

end:
    if (ret != HAL_RET_OK) {
        uplinkif_pd_cleanup(pd_upif);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// De-Allocate resources.
//-----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_dealloc_res(pd_uplinkif_t *upif_pd)
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
        if_free_hwlif_id(upif_pd->hw_lif_id);

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

    if (upif_pd->upif_lport_id != INVALID_INDEXER_INDEX) {
        rs = g_hal_state_pd->lport_idxr()->free(upif_pd->upif_lport_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("failed to free lport_id err: {}",
                          upif_pd->upif_lport_id);
            ret = HAL_RET_INVALID_OP;
            goto end;
        }

        HAL_TRACE_DEBUG("freed lport_id: {}",
                        upif_pd->upif_lport_id);
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// PD Uplinkif Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD If
//  Note:
//      - Just free up whatever PD has.
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_cleanup(pd_uplinkif_t *upif_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!upif_pd) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = uplinkif_pd_dealloc_res(upif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to dealloc res for uplinkif: {}",
                      ((if_t *)(upif_pd->pi_if))->if_id);
        goto end;
    }

    // Delinking PI<->PD
    uplinkif_delink_pi_pd(upif_pd, (if_t *)upif_pd->pi_if);

    // Freeing PD
    uplinkif_pd_free(upif_pd);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram HW
// ----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_deprogram_hw (pd_uplinkif_t *pd_upif)
{
    hal_ret_t            ret = HAL_RET_OK;

    // De program TM register
    ret = uplinkif_pd_depgm_tm_register(pd_upif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram hw");
    }

    // De-Program Output Mapping Table
    ret = uplinkif_pd_depgm_output_mapping_tbl(pd_upif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram hw");
    }
    return ret;
}

// ----------------------------------------------------------------------------
// De-Program TM Register
// ----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_depgm_tm_register(pd_uplinkif_t *pd_upif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    uint8_t                     tm_oport = 0;

    tm_oport = uplinkif_get_port_num((if_t *)(pd_upif->pi_if));

    sdk_ret = capri_tm_uplink_lif_set(tm_oport, 0);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
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

// ----------------------------------------------------------------------------
// DeProgram output mapping table for cpu tx traffic
// ----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_depgm_output_mapping_tbl (pd_uplinkif_t *pd_upif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    directmap                   *dm_omap = NULL;

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    SDK_ASSERT_RETURN((dm_omap != NULL), HAL_RET_ERR);

    sdk_ret = dm_omap->remove(pd_upif->upif_lport_id);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram omapping table at: {}",
                      pd_upif->upif_lport_id);
    } else {
        HAL_TRACE_ERR("deprogrammed omapping table at: {}",
                      pd_upif->upif_lport_id);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_program_hw(pd_uplinkif_t *pd_upif, bool is_upgrade)
{
    hal_ret_t            ret;

    // Upgrade: There is no table lib for this. Does only HW programming.
    if (!is_upgrade) {
        ret = uplinkif_pd_pgm_tm_register(pd_upif);
        SDK_ASSERT_RETURN(ret == HAL_RET_OK, ret);
    }

    /*
     * Program Output Mapping Table.
     * Upgrade: No need to send the flag. Already doing insert_withid, so lib
     *          will be populated and since we are in upgrade HW writes will
     *          not happen.
     */
    ret = uplinkif_pd_pgm_output_mapping_tbl(pd_upif);
    SDK_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    return ret;
}

// ----------------------------------------------------------------------------
// Program TM Register
// ----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_pgm_tm_register(pd_uplinkif_t *pd_upif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    uint8_t                     tm_oport = 0;

    tm_oport = uplinkif_get_port_num((if_t *)(pd_upif->pi_if));

    sdk_ret = capri_tm_uplink_lif_set(tm_oport, pd_upif->hw_lif_id);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to program for if_id: {}",
                      if_get_if_id((if_t *)pd_upif->pi_if));
    } else {
        HAL_TRACE_DEBUG("programmed for if_id: {} "
                        "iport:{} => hwlif: {}",
                        if_get_if_id((if_t *)pd_upif->pi_if),
                        tm_oport, pd_upif->hw_lif_id);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Program Output Mapping Table
// ----------------------------------------------------------------------------
#define om_tmoport data.action_u.output_mapping_set_tm_oport
hal_ret_t
uplinkif_pd_pgm_output_mapping_tbl(pd_uplinkif_t *pd_upif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    uint8_t                     tm_oport = 0;
    output_mapping_actiondata_t   data;
    directmap                   *dm_omap = NULL;
    if_t                        *hal_if = (if_t *)pd_upif->pi_if;

    memset(&data, 0, sizeof(data));

    tm_oport = uplinkif_get_port_num((if_t *)(pd_upif->pi_if));

    data.action_id = OUTPUT_MAPPING_SET_TM_OPORT_ID;
    om_tmoport.nports = 1;
    om_tmoport.egress_mirror_en = 1;
    om_tmoport.egress_port1 = tm_oport;
    om_tmoport.dst_lif = pd_upif->hw_lif_id;
    om_tmoport.nacl_egress_drop_en = hal_if->is_oob_management;

    // Program OutputMapping table
    //  - Get tmoport from PI
    //  - Get vlan_tagid_in_skb from the fwding mode:
    //      - Classic: TRUE
    //      - Switch : FALSE

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    SDK_ASSERT_RETURN((dm_omap != NULL), HAL_RET_ERR);

    sdk_ret = dm_omap->insert_withid(&data, pd_upif->upif_lport_id);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to program for if_id: {}",
                      if_get_if_id((if_t *)pd_upif->pi_if));
    } else {
        HAL_TRACE_DEBUG("programmed for if_id: {} at {}",
                        if_get_if_id((if_t *)pd_upif->pi_if),
                        pd_upif->upif_lport_id);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_uplinkif_make_clone(pd_if_make_clone_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_uplinkif_t       *pd_uplinkif_clone = NULL;
    if_t *hal_if = args->hal_if;
    if_t *clone = args->clone;

    pd_uplinkif_clone = uplinkif_pd_alloc_init();
    if (pd_uplinkif_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_uplinkif_clone, hal_if->pd_if, sizeof(pd_uplinkif_t));

    uplinkif_link_pi_pd(pd_uplinkif_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_uplinkif_mem_free(pd_if_mem_free_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_uplinkif_t  *upif_pd;

    upif_pd = (pd_uplinkif_t *)args->intf->pd_if;
    uplinkif_pd_mem_free(upif_pd);

    return ret;
}

uint32_t
pd_uplinkif_pdif_label (pd_uplinkif_t *upif)
{
    return upif->up_ifpc_id + 1;
}

uint32_t
pd_uplinkif_if_label (if_t *hal_if)
{
    pd_uplinkif_t *uplinkif_pd = NULL;
        
    if (!hal_if) {
        goto end;
    }
    uplinkif_pd = (pd_uplinkif_t *)hal_if->pd_if;

    if (hal::intf_get_if_type(hal_if) == intf::IF_TYPE_UPLINK) {
        return pd_uplinkif_pdif_label(uplinkif_pd);
    }
end:
    return 0;
}


}    // namespace pd
}    // namespace hal

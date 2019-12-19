// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/mcast/multicast_pd.hpp"
#include "nic/hal/pd/iris/nw/vrf_pd.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"
#include "nic/hal/src/internal/proxy.hpp"

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void
mc_entry_link_pi_pd(pd_mc_entry_t *pd_mc_entry, mc_entry_t *pi_mc_entry)
{
    pd_mc_entry->mc_entry = pi_mc_entry;
    pi_mc_entry->pd = pd_mc_entry;
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void
mc_entry_delink_pi_pd(pd_mc_entry_t *pd_mc_entry, mc_entry_t *pi_mc_entry)
{
    if (pd_mc_entry) {
        pd_mc_entry->mc_entry = NULL;
    }
    if (pi_mc_entry) {
        pi_mc_entry->pd = NULL;
    }
}

#define registered_mac_data data.action_u.registered_macs_registered_macs

hal_ret_t
pd_mc_entry_pgm_registered_mac(pd_mc_entry_t *pd_mc_entry, table_oper_t oper)
{
    hal_ret_t                       ret  = HAL_RET_OK;
    sdk_ret_t                       sdk_ret;
    registered_macs_swkey_t         key  = { 0 };
    registered_macs_actiondata_t      data = { 0 };
    sdk_hash                        *reg_mac_tbl = NULL;
    mc_entry_t                      *pi_mc_entry = (mc_entry_t *)pd_mc_entry->mc_entry;
    l2seg_t                         *l2seg = NULL;
    uint32_t                        hash_idx = INVALID_INDEXER_INDEX;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    reg_mac_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_REGISTERED_MACS);
    SDK_ASSERT_RETURN((reg_mac_tbl != NULL), HAL_RET_ERR);

    // lkp_vrf
    l2seg = l2seg_lookup_by_handle(pi_mc_entry->key.l2seg_handle);
    SDK_ASSERT_RETURN(l2seg != NULL, HAL_RET_L2SEG_NOT_FOUND);
    key.flow_lkp_metadata_lkp_reg_mac_vrf = 
        ((pd_l2seg_t *)(l2seg->pd))->l2seg_fl_lkup_id;

    // lkp_mac
    memcpy(key.flow_lkp_metadata_lkp_dstMacAddr, pi_mc_entry->key.u.mac, 6);
    memrev(key.flow_lkp_metadata_lkp_dstMacAddr, 6);

    // dst_lport
    data.action_id = REGISTERED_MACS_REGISTERED_MACS_ID;
    registered_mac_data.multicast_en = (uint8_t)TRUE;
    registered_mac_data.dst_lport = (uint16_t) pi_mc_entry->oif_list;

    if (oper == TABLE_OPER_INSERT) {
        // Insert
        sdk_ret = reg_mac_tbl->insert(&key, &data, &hash_idx);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to program for :{}",
                          mc_key_to_string(&pi_mc_entry->key));
            goto end;
        } else {
            HAL_TRACE_DEBUG("Programmed {} at hash_idx:{}",
                            mc_key_to_string(&pi_mc_entry->key), hash_idx);
        }

        pd_mc_entry->reg_mac_tbl_idx = hash_idx;
    } else {
        hash_idx = pd_mc_entry->reg_mac_tbl_idx;
        // Update
        sdk_ret = reg_mac_tbl->update(hash_idx, &data);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to reprogram for {} at hash_idx:{}",
                          mc_key_to_string(&pi_mc_entry->key), hash_idx);
            goto end;
        } else {
            HAL_TRACE_DEBUG("Reprogrammed for {} at hash_idx:{}",
                            mc_key_to_string(&pi_mc_entry->key), hash_idx);
        }
    }

end:

    return ret;
}

// ----------------------------------------------------------------------------
// De-program Registered Mac table
// ----------------------------------------------------------------------------
hal_ret_t
pd_mc_entry_depgm_registered_mac (pd_mc_entry_t *mc_entry_pd)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    sdk_hash                    *reg_mac_tbl = NULL;
    mc_entry_t                  *mc_entry = (mc_entry_t*) mc_entry_pd->mc_entry;

    reg_mac_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_REGISTERED_MACS);
    SDK_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    sdk_ret = reg_mac_tbl->remove(mc_entry_pd->reg_mac_tbl_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to deprogram from cpu entry for {}",
                      mc_key_to_string(&mc_entry->key));
    } else {
        HAL_TRACE_DEBUG("Deprogrammed from_cpu_entry for {}",
                        mc_key_to_string(&mc_entry->key));
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
mc_entry_pd_program_hw (pd_mc_entry_t *mc_entry_pd)
{
    hal_ret_t ret;

    // Program Registered Mac Table
    ret = pd_mc_entry_pgm_registered_mac(mc_entry_pd, TABLE_OPER_INSERT);
    SDK_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    return ret;
}

// ----------------------------------------------------------------------------
// Update HW
// ----------------------------------------------------------------------------
hal_ret_t
mc_entry_pd_update_hw (pd_mc_entry_t *mc_entry_pd)
{
    hal_ret_t ret;

    // Program Registered Mac Table
    ret = pd_mc_entry_pgm_registered_mac(mc_entry_pd, TABLE_OPER_UPDATE);
    SDK_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    return ret;
}

//-----------------------------------------------------------------------------
// PD mc entry Cleanup
//-----------------------------------------------------------------------------
hal_ret_t
mc_entry_pd_cleanup(pd_mc_entry_t *mc_entry_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!mc_entry_pd) {
        // Nothing to do
        goto end;
    }

    // Delinking PI<->PD
    mc_entry_delink_pi_pd(mc_entry_pd, (mc_entry_t *)mc_entry_pd->mc_entry);

    // Freeing PD
    mc_entry_pd_free(mc_entry_pd);

end:
    return ret;
}

//------------------------------------------------------------------------------
// pd mc_entry get
//------------------------------------------------------------------------------
hal_ret_t
pd_mc_entry_get (pd_func_args_t *pd_func_args)
{
    pd_mc_entry_get_args_t *args = pd_func_args->pd_mc_entry_get;
    pd_mc_entry_t           *mc_entry_pd = NULL;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->rsp != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->mc_entry != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->mc_entry->pd != NULL), HAL_RET_INVALID_ARG);

    HAL_TRACE_VERBOSE("Getting pd state for mc_entry {}",
                    mc_key_to_string(&args->mc_entry->key));

    mc_entry_pd = (pd_mc_entry_t *)args->mc_entry->pd;
    args->rsp->mutable_status()->mutable_epd_info()->set_reg_mac_tbl_idx(
            mc_entry_pd->reg_mac_tbl_idx);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// pd mc_entry create
//------------------------------------------------------------------------------
hal_ret_t
pd_mc_entry_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_mc_entry_create_args_t *args = pd_func_args->pd_mc_entry_create;
    pd_mc_entry_t           *mc_entry_pd = NULL;

    HAL_TRACE_DEBUG("Creating pd state for mc_entry {}",
                    mc_key_to_string(&args->mc_entry->key));

    // create mc entry PD
    mc_entry_pd = mc_entry_pd_alloc_init();
    if (mc_entry_pd == NULL) {
        return HAL_RET_OOM;
    }

    // link PI<->PD
    mc_entry_link_pi_pd(mc_entry_pd, args->mc_entry);

    // Program HW
    ret = mc_entry_pd_program_hw(mc_entry_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program hw for mc_entry {}",
                      mc_key_to_string(&args->mc_entry->key));
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        mc_entry_pd_cleanup(mc_entry_pd);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD mc_entry Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_mc_entry_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_mc_entry_update_args_t *args = pd_func_args->pd_mc_entry_update;
    pd_mc_entry_t           *mc_entry_pd = NULL;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->mc_entry != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->upd_entry != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->mc_entry->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("Updating pd state for mc_entry {}",
                    mc_key_to_string(&args->mc_entry->key));

    mc_entry_pd = (pd_mc_entry_t *)args->mc_entry->pd;

    // link PI<->PD
    mc_entry_link_pi_pd(mc_entry_pd, args->upd_entry);

    ret = mc_entry_pd_update_hw(mc_entry_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update hw for mc_entry {}",
                      mc_key_to_string(&args->mc_entry->key));
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD mc_entry Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_mc_entry_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_mc_entry_delete_args_t *args = pd_func_args->pd_mc_entry_delete;
    pd_mc_entry_t     *mc_entry_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->mc_entry != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->mc_entry->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("Deleting pd state for mc_entry {}",
                    mc_key_to_string(&args->mc_entry->key));

    mc_entry_pd = (pd_mc_entry_t *)args->mc_entry->pd;

    // deprogram HW
    ret = mc_entry_pd_deprogram_hw(mc_entry_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to deprogram hw for mc_entry {}",
                      mc_key_to_string(&args->mc_entry->key));
    }

    // dealloc resources and free
    ret = mc_entry_pd_cleanup(mc_entry_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed pd mc_entry delete");
    }

    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram HW
// ----------------------------------------------------------------------------
hal_ret_t
mc_entry_pd_deprogram_hw (pd_mc_entry_t *mc_entry_pd)
{
    hal_ret_t            ret = HAL_RET_OK;

    // De-program registered Mac Table
    ret = pd_mc_entry_depgm_registered_mac(mc_entry_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to deprogram hw");
    }

    return ret;
}

}    // namespace pd
}    // namespace hal

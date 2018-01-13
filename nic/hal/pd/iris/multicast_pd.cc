// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/multicast_pd.hpp"
#include "nic/hal/pd/iris/vrf_pd.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/if_pd_utils.hpp"
#include "nic/hal/src/proxy.hpp"

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

#define registered_mac_data data.registered_macs_action_u.registered_macs_registered_macs

hal_ret_t
pd_mc_entry_pgm_registered_mac(pd_mc_entry_t *pd_mc_entry, table_oper_t oper)
{
    hal_ret_t                       ret  = HAL_RET_OK;
    sdk_ret_t                       sdk_ret;
    registered_macs_swkey_t         key  = { 0 };
    registered_macs_actiondata      data = { 0 };
    sdk_hash                        *reg_mac_tbl = NULL;
    mc_entry_t                      *pi_mc_entry = (mc_entry_t *)pd_mc_entry->mc_entry;
    l2seg_t                         *l2seg = NULL;
    uint32_t                        hash_idx = INVALID_INDEXER_INDEX;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    reg_mac_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_REGISTERED_MACS);
    HAL_ASSERT_RETURN((reg_mac_tbl != NULL), HAL_RET_ERR);

    // lkp_vrf
    l2seg = find_l2seg_by_handle(pi_mc_entry->key.l2seg_handle);
    HAL_ASSERT_RETURN(l2seg != NULL, HAL_RET_L2SEG_NOT_FOUND);
    key.flow_lkp_metadata_lkp_vrf = ((pd_l2seg_t *)(l2seg->pd))->l2seg_fl_lkup_id;

    // lkp_mac
    memcpy(key.flow_lkp_metadata_lkp_dstMacAddr, pi_mc_entry->key.u.mac, 6);
    memrev(key.flow_lkp_metadata_lkp_dstMacAddr, 6);

    // dst_lport
    data.actionid = REGISTERED_MACS_REGISTERED_MACS_ID;
    registered_mac_data.multicast_en = (uint8_t)TRUE;
    registered_mac_data.dst_lport = (uint16_t) pi_mc_entry->oif_list;

    if (oper == TABLE_OPER_INSERT) {
        // Insert
        sdk_ret = reg_mac_tbl->insert(&key, &data, &hash_idx);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-mc_entry:{}:classic: unable to program for :{}",
                          __FUNCTION__, mc_key_to_string(&pi_mc_entry->key));
            goto end;
        } else {
            HAL_TRACE_DEBUG("pd-ep:{}:classic: programmed for ep:{} at hash_idx:{}",
                            __FUNCTION__, mc_key_to_string(&pi_mc_entry->key), hash_idx);
        }

        pd_mc_entry->reg_mac_tbl_idx = hash_idx;
    } else {
        hash_idx = pd_mc_entry->reg_mac_tbl_idx;
        // Update
        sdk_ret = reg_mac_tbl->update(hash_idx, &data);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-mc_entry:{}:classic: unable to reprogram for "
                          "mc_entry:{} at: {}",
                          __FUNCTION__, mc_key_to_string(&pi_mc_entry->key), hash_idx);
            goto end;
        } else {
            HAL_TRACE_DEBUG("pd-mc_entry:{}:classic: reprogrammed for "
                            "mc_entry:{} at: {}", __FUNCTION__,
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
    sdk_hash                    *inp_prop_tbl = NULL;
    mc_entry_t                  *mc_entry = (mc_entry_t*) mc_entry_pd->mc_entry;

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_REGISTERED_MACS);
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    sdk_ret = inp_prop_tbl->remove(mc_entry_pd->reg_mac_tbl_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-mc_entry::{}:unable to deprogram from cpu entry "
                              "input properties for seg_id:{}",
                      __FUNCTION__, mc_key_to_string(&mc_entry->key));
    } else {
        HAL_TRACE_DEBUG("pd-mc_entry::{}:deprogrammed from_cpu_entry "
                                "input properties for seg_id:{}",
                        __FUNCTION__, mc_key_to_string(&mc_entry->key));
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
mc_entry_pd_program_hw (pd_mc_entry_t *mc_entry_pd)
{
    hal_ret_t            ret;

    // Program Registered Mac Table
    ret = pd_mc_entry_pgm_registered_mac(mc_entry_pd, TABLE_OPER_INSERT);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

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
// pd mc_entry create
//------------------------------------------------------------------------------
hal_ret_t
pd_mc_entry_create (pd_mc_entry_args_t *args)
{
    hal_ret_t               ret;
    pd_mc_entry_t           *mc_entry_pd = NULL;

    HAL_TRACE_DEBUG("pd-mc_entry:{}:creating pd state for mc_entry",
                    __FUNCTION__, mc_key_to_string(&args->mc_entry->key));

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
        HAL_TRACE_ERR("pd-mc_entry:{}:failed to program hw", __FUNCTION__);
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        mc_entry_pd_cleanup(mc_entry_pd);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD mc_entry Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_mc_entry_delete (pd_mc_entry_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_mc_entry_t     *mc_entry_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->mc_entry != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->mc_entry->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("pd-mc_entry:{}:deleting pd state for mc_entry {}",
                    __FUNCTION__, mc_key_to_string(&args->mc_entry->key));

    mc_entry_pd = (pd_mc_entry_t *)args->mc_entry->pd;

    // deprogram HW
    ret = mc_entry_pd_deprogram_hw(mc_entry_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-mc_entry:{}:unable to deprogram hw", __FUNCTION__);
    }

    // dealloc resources and free
    ret = mc_entry_pd_cleanup(mc_entry_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed pd mc_entry delete");
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
        HAL_TRACE_ERR("pd-mc_entry:{}:unable to deprogram hw", __FUNCTION__);
    }

    return ret;
}

}    // namespace pd
}    // namespace hal

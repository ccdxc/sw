#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/hal/pd/iris/if_pd.hpp"
#include "nic/hal/pd/iris/uplinkif_pd.hpp"
#include "nic/hal/pd/capri/capri_tm_rw.hpp"
#include "nic/hal/pd/p4pd_api.hpp"

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// Uplink If Create
// ----------------------------------------------------------------------------
hal_ret_t 
pd_uplinkif_create(pd_if_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;; 
    pd_uplinkif_t        *pd_upif;

    HAL_TRACE_DEBUG("PD-Uplinkif:{}: Creating pd state for if_id: {}", 
                    __FUNCTION__, if_get_if_id(args->intf));

    // Create lif PD
    pd_upif = uplinkif_pd_alloc_init();
    if (pd_upif == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    link_pi_pd(pd_upif, args->intf);

    // Allocate Resources
    ret = uplinkif_pd_alloc_res(pd_upif);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-Uplinkif::{}: Unable to alloc. resources for if_id: {}",
                      __FUNCTION__, if_get_if_id(args->intf));
        goto end;
    }

    // Program HW
    ret = uplinkif_pd_program_hw(pd_upif);

end:
    if (ret != HAL_RET_OK) {
        unlink_pi_pd(pd_upif, args->intf);
        uplinkif_pd_free(pd_upif);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize Uplinkif PD Instance
// ----------------------------------------------------------------------------
pd_uplinkif_t *
uplinkif_pd_alloc_init(void)
{
    return uplinkif_pd_init(uplinkif_pd_alloc());
}

// ----------------------------------------------------------------------------
// Allocate Uplink IF Instance
// ----------------------------------------------------------------------------
pd_uplinkif_t *
uplinkif_pd_alloc (void)
{
    pd_uplinkif_t    *upif;

    upif = (pd_uplinkif_t *)g_hal_state_pd->uplinkif_pd_slab()->alloc();
    if (upif == NULL) {
        return NULL;
    }
    return upif;
}

// ----------------------------------------------------------------------------
// Initialize Uplink IF PD instance
// ----------------------------------------------------------------------------
pd_uplinkif_t *
uplinkif_pd_init (pd_uplinkif_t *upif)
{
    // Nothing to do currently
    if (!upif) {
        return NULL;
    }

    // Set here if you want to initialize any fields

    return upif;
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
    rs = g_hal_state_pd->lif_hwid_idxr()->
        alloc((uint32_t *)&pd_upif->hw_lif_id);
    if (rs != indexer::SUCCESS) {
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("PD-Uplinkif:{}: if_id:{} Allocated hw_lif_id: {}", 
                    __FUNCTION__, 
                    if_get_if_id((if_t *)pd_upif->pi_if),
                    pd_upif->hw_lif_id);

    // Allocate ifpc id
    rs = g_hal_state_pd->uplinkifpc_hwid_idxr()->
        alloc((uint32_t *)&pd_upif->up_ifpc_id);
    if (rs != indexer::SUCCESS) {
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("PD-Uplinkif:{}: if_id:{} Allocated Uplink ifpc_id: {}", 
                    __FUNCTION__, 
                    if_get_if_id((if_t *)pd_upif->pi_if),
                    pd_upif->up_ifpc_id);

    // Allocate lport
    rs = g_hal_state_pd->lport_idxr()->alloc((uint32_t *)&pd_upif->
            upif_lport_id);
    if (rs != indexer::SUCCESS) {
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("PD-Uplinkif:{}: if_id:{} Allocated lport_id:{}", 
                    __FUNCTION__, 
                    if_get_if_id((if_t *)pd_upif->pi_if),
                    pd_upif->upif_lport_id);
    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_program_hw(pd_uplinkif_t *pd_upif)
{
    hal_ret_t            ret;

    // TODO: Program TM table port_num -> lif_hw_id
    ret = uplinkif_pd_pgm_tm_register(pd_upif);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    // Program Output Mapping Table
    ret = uplinkif_pd_pgm_output_mapping_tbl(pd_upif);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    return ret;
}

// ----------------------------------------------------------------------------
// Program TM Register
// ----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_pgm_tm_register(pd_uplinkif_t *pd_upif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    uint8_t                     tm_oport = 0;

    tm_oport = uplinkif_get_port_num((if_t *)(pd_upif->pi_if)); 

    ret = capri_tm_uplink_lif_set(tm_oport, pd_upif->hw_lif_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-Uplinkif::{}: Unable to program for if_id: {}",
                __FUNCTION__, if_get_if_id((if_t *)pd_upif->pi_if));
    } else {
        HAL_TRACE_DEBUG("PD-Uplinkif::{}: Programmed for if_id: {} "
                        "iport:{} => hwlif: {}",
                        __FUNCTION__, if_get_if_id((if_t *)pd_upif->pi_if),
                        tm_oport, pd_upif->hw_lif_id);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Program Output Mapping Table
// ----------------------------------------------------------------------------
#define om_tmoport data.output_mapping_action_u.output_mapping_set_tm_oport
hal_ret_t
uplinkif_pd_pgm_output_mapping_tbl(pd_uplinkif_t *pd_upif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    uint8_t                     tm_oport = 0;
    output_mapping_actiondata   data;
    DirectMap                   *dm_omap = NULL;
    char                        buff[4096] = {0};
    p4pd_error_t                p4_err;

    memset(&data, 0, sizeof(data));

    tm_oport = uplinkif_get_port_num((if_t *)(pd_upif->pi_if)); 

    data.actionid = OUTPUT_MAPPING_SET_TM_OPORT_ID;
    om_tmoport.nports = 1;
    om_tmoport.egress_mirror_en = 1;
    om_tmoport.egress_port1 = tm_oport;
    om_tmoport.dst_lif = pd_upif->hw_lif_id;

    
    // Program OutputMapping table
    //  - Get tmoport from PI
    //  - Get vlan_tagid_in_skb from the fwding mode:
    //      - Classic: TRUE
    //      - Switch : FALSE

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    HAL_ASSERT_RETURN((dm_omap != NULL), HAL_RET_ERR);

    ret = dm_omap->insert_withid(&data, pd_upif->upif_lport_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-Uplinkif::{}: Unable to program for if_id: {}",
                __FUNCTION__, if_get_if_id((if_t *)pd_upif->pi_if));
    } else {
        HAL_TRACE_DEBUG("PD-Uplinkif::{}: Programmed for if_id: {} at {}",
                __FUNCTION__, if_get_if_id((if_t *)pd_upif->pi_if),
                pd_upif->upif_lport_id);
        p4_err = p4pd_table_ds_decoded_string_get(P4TBL_ID_OUTPUT_MAPPING, 
                                                NULL, NULL, &data, buff, 
                                                sizeof(buff));
        HAL_ASSERT(p4_err == P4PD_SUCCESS);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Freeing UPLINKIF PD
// ----------------------------------------------------------------------------
hal_ret_t
uplinkif_pd_free (pd_uplinkif_t *upif)
{
    g_hal_state_pd->uplinkif_pd_slab()->free(upif);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
link_pi_pd(pd_uplinkif_t *pd_upif, if_t *pi_if)
{
    pd_upif->pi_if = pi_if;
    if_set_pd_if(pi_if, pd_upif);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
unlink_pi_pd(pd_uplinkif_t *pd_upif, if_t *pi_if)
{
    pd_upif->pi_if = NULL;
    if_set_pd_if(pi_if, NULL);
}



}    // namespace pd
}    // namespace hal

#include <hal_lock.hpp>
#include <hal_state_pd.hpp>
#include <lif_pd.hpp>
#include <pd_api.hpp>
#include <interface_api.hpp>
#include <p4pd.h>
#include <defines.h>

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// Lif Create in PD
// ----------------------------------------------------------------------------
hal_ret_t
pd_lif_create (pd_lif_args_t *args)
{
    hal_ret_t            ret;
    pd_lif_t             *pd_lif;

    HAL_TRACE_DEBUG("PD-LIF::{}: lif_id:{} Creating pd state for Lif", 
            __FUNCTION__, lif_get_lif_id(args->lif));

    // Create lif PD
    pd_lif = lif_pd_alloc_init();
    if (pd_lif == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    link_pi_pd(pd_lif, args->lif);

    // Allocate Resources
    ret = lif_pd_alloc_res(pd_lif, args);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-LIF::{}: lif_id:{} Unable to alloc. resources for Lif",
                __FUNCTION__, lif_get_lif_id(args->lif));
        goto end;
    }

    // Program HW
    ret = lif_pd_program_hw(pd_lif);

end:
    if (ret != HAL_RET_OK) {
        unlink_pi_pd(pd_lif, args->lif);
        lif_pd_free(pd_lif);
    }
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize Lif PD Instance
// ----------------------------------------------------------------------------
inline pd_lif_t *
lif_pd_alloc_init (void)
{
    return lif_pd_init(lif_pd_alloc());
}

// ----------------------------------------------------------------------------
// Allocate LIF Instance
// ----------------------------------------------------------------------------
inline pd_lif_t *
lif_pd_alloc (void)
{
    pd_lif_t    *lif;

    lif = (pd_lif_t *)g_hal_state_pd->lif_pd_slab()->alloc();
    if (lif == NULL) {
        return NULL;
    }
    return lif;
}

// ----------------------------------------------------------------------------
// Initialize LIF PD instance
// ----------------------------------------------------------------------------
inline pd_lif_t *
lif_pd_init (pd_lif_t *lif)
{
    // Nothing to do currently
    if (!lif) {
        return NULL;
    }

    // Set here if you want to initialize any fields

    return lif;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD Lif
// ----------------------------------------------------------------------------
hal_ret_t 
lif_pd_alloc_res(pd_lif_t *pd_lif, pd_lif_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;
    indexer::status      rs = indexer::SUCCESS;

    if(((lif_t *)pd_lif->pi_lif)->lif_id == 1001) {
        pd_lif->hw_lif_id = 1001;
        HAL_TRACE_DEBUG("HACK ALERT: SETTING HW LIF ID to {}", pd_lif->hw_lif_id);
        return ret;
    }

    if (args->with_hw_lif_id) {
        pd_lif->hw_lif_id = args->hw_lif_id;
    } else {
        // Allocate lif hwid
        rs = g_hal_state_pd->lif_hwid_idxr()->alloc((uint32_t *)&pd_lif->hw_lif_id);
        if (rs != indexer::SUCCESS) {
            return HAL_RET_NO_RESOURCE;
        }
    }
    HAL_TRACE_DEBUG("PD-LIF:{}: lif_id:{} Allocated hw_lif_id:{}", 
                    __FUNCTION__, 
                    lif_get_lif_id((lif_t *)pd_lif->pi_lif),
                    pd_lif->hw_lif_id);

    // Allocate lport
    rs = g_hal_state_pd->lport_idxr()->alloc((uint32_t *)&pd_lif->
            lif_lport_id);
    if (rs != indexer::SUCCESS) {
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("PD-LIF:{}: lif_id:{} Allocated lport_id:{}", 
                    __FUNCTION__, 
                    lif_get_lif_id((lif_t *)pd_lif->pi_lif),
                    pd_lif->lif_lport_id);
    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
lif_pd_program_hw(pd_lif_t *pd_lif)
{
    hal_ret_t            ret;
    ret = lif_pd_pgm_output_mapping_tbl(pd_lif);

    return ret;
}

// ----------------------------------------------------------------------------
// Program Output Mapping Table
// ----------------------------------------------------------------------------
#define om_tmoport data.output_mapping_action_u.output_mapping_set_tm_oport
hal_ret_t
lif_pd_pgm_output_mapping_tbl(pd_lif_t *pd_lif)
{
    hal_ret_t                   ret = HAL_RET_OK;
    uint8_t                     tm_oport = 0;
    uint8_t                     p4plus_app_id = 0;
    output_mapping_actiondata   data;
    DirectMap                   *dm_omap = NULL;

    memset(&data, 0, sizeof(data));

    if(((lif_t *)pd_lif->pi_lif)->lif_id != 1001) {
        tm_oport = lif_get_port_num((lif_t *)(pd_lif->pi_lif));
        tm_oport = TM_PORT_DMA;
    } else {
        pd_lif->lif_lport_id = 1001;
        pd_lif->hw_lif_id = 1001;
        tm_oport = 9;
        p4plus_app_id = 3;
        HAL_TRACE_ERR("xxx: setting tm_port = 9");
    }

    data.actionid = OUTPUT_MAPPING_SET_TM_OPORT_ID;
    om_tmoport.nports = 1;
    om_tmoport.egress_port1 = tm_oport;
    om_tmoport.p4plus_app_id = p4plus_app_id;
    om_tmoport.rdma_enabled = lif_get_enable_rdma((lif_t *)pd_lif->pi_lif);
    om_tmoport.dst_lif = pd_lif->hw_lif_id;

    // Program OutputMapping table
    //  - Get tmoport from PI
    //  - Get vlan_tagid_in_skb from the fwding mode:
    //      - Classic: TRUE
    //      - Switch : FALSE

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    ret = dm_omap->insert_withid(&data, pd_lif->lif_lport_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-LIF::{}: lif_id:{} Unable to program",
                __FUNCTION__, lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    } else {
        HAL_TRACE_DEBUG("PD-LIF::{}: lif_id:{} Success",
                __FUNCTION__, lif_get_lif_id((lif_t *)pd_lif->pi_lif));
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Freeing LIF PD
// ----------------------------------------------------------------------------
hal_ret_t
lif_pd_free (pd_lif_t *lif)
{
    g_hal_state_pd->lif_pd_slab()->free(lif);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
link_pi_pd(pd_lif_t *pd_lif, lif_t *pi_lif)
{
    pd_lif->pi_lif = pi_lif;
    lif_set_pd_lif(pi_lif, pd_lif);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
unlink_pi_pd(pd_lif_t *pd_lif, lif_t *pi_lif)
{
    pd_lif->pi_lif = NULL;
    lif_set_pd_lif(pi_lif, NULL);
}
}    // namespace pd
}    // namespace hal

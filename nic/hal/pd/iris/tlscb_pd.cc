#include <hal_lock.hpp>
#include <proxy_api.hpp>
#include <pd_api.hpp>
#include <tlscb_pd.hpp>
#include <pd.hpp>
#include <hal_state_pd.hpp>

namespace hal {
namespace pd {

void *
tlscb_pd_get_hw_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((pd_tlscb_t *)entry)->hw_id);
}

uint32_t
tlscb_pd_compute_hw_hash_func (void *key, uint32_t ht_size)
{
    return hal::utils::hash_algo::fnv_hash(key, sizeof(tlscb_hw_id_t)) % ht_size;
}

bool
tlscb_pd_compare_hw_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(tlscb_hw_id_t *)key1 == *(tlscb_hw_id_t *)key2) {
        return true;
    }
    return false;
}

hal_ret_t
p4pd_add_tlscb_entry(pd_tlscb_t* tlscb_pd) 
{
    hal_ret_t                   ret = HAL_RET_OK;
    /*
    tlscb_tbl_actiondata        data;
    p4pd_error_t                pd_err = P4PD_SUCCESS;

    // program tlscb in hbm
    data.actionid = TLSCB_TBL_TLS_RX_SERQ_ACTION_ID;
    data.tlscb_tbl_action_u.tlscb_tbl_tls_rx_serq_action.salt = 0;
    data.tlscb_tbl_action_u.tlscb_tbl_tls_rx_serq_action.cipher_type = tlscb_pd->tlscb->cipher_type;
    
    pd_err = proxypd_entry_write(P4TBL_ID_TLSCB_TBL, 0, NULL, NULL, &data);
    if(pd_err != P4PD_SUCCESS) {
        ret = HAL_RET_HW_FAIL;
        HAL_ASSERT(0);
    }
    */
    return ret;
}

tlscb_hw_id_t
pd_tlscb_get_base_hw_index(pd_tlscb_t* tlscb_pd)
{
    HAL_ASSERT(NULL != tlscb_pd);
    HAL_ASSERT(NULL != tlscb_pd->tlscb);
    
    /*
    char tcpcb_reg[10] = "tcpcb";
    return get_start_offset(tcpcb_reg) + \
        (tcpcb_pd->tcpcb->cb_id * P4PD_HBM_TCP_CB_ENTRY_SIZE);
    */
    return 0xbbbb + \
        (tlscb_pd->tlscb->cb_id * P4PD_HBM_TLS_CB_ENTRY_SIZE);

}



hal_ret_t
pd_tlscb_create (pd_tlscb_args_t *args)
{
    hal_ret_t               ret;
    pd_tlscb_s              *tlscb_pd;

    HAL_TRACE_DEBUG("Creating pd state for TLS CB");

    // allocate PD tlscb state
    tlscb_pd = tlscb_pd_alloc_init();
    if (tlscb_pd == NULL) {
        return HAL_RET_OOM;
    }
    HAL_TRACE_DEBUG("Alloc done");
    tlscb_pd->tlscb = args->tlscb;

    // get hw-id for this TLSCB
    tlscb_pd->hw_id = pd_tlscb_get_base_hw_index(tlscb_pd);
    HAL_TRACE_DEBUG("Received hw-id");

    // program tlscb
    ret = p4pd_add_tlscb_entry(tlscb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;    
    }

    // add to db
    ret = add_tlscb_pd_to_db(tlscb_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    HAL_TRACE_DEBUG("DB add done");
    args->tlscb->pd = tlscb_pd;

    return HAL_RET_OK;

cleanup:

    if (tlscb_pd) {
        tlscb_pd_free(tlscb_pd);
    }
    return ret;
}

}    // namespace pd
}    // namespace hal

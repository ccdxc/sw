#include <base.h>
#include <hal_lock.hpp>
#include <pd_api.hpp>
#include <tcpcb_pd.hpp>
#include <p4pd_tcp_proxy_api.h>
#include <capri_loader.h>
#include <capri_hbm.hpp>
#include <capri_lif.hpp>
#include <wring_pd.hpp>

namespace hal {
namespace pd {

void *
tcpcb_pd_get_hw_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((pd_tcpcb_t *)entry)->hw_id);
}

uint32_t
tcpcb_pd_compute_hw_hash_func (void *key, uint32_t ht_size)
{
    return hal::utils::hash_algo::fnv_hash(key, sizeof(tcpcb_hw_id_t)) % ht_size;
}

bool
tcpcb_pd_compare_hw_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(tcpcb_hw_id_t *)key1 == *(tcpcb_hw_id_t *)key2) {
        return true;
    }
    return false;
}

/********************************************
 * RxDMA
 * ******************************************/

hal_ret_t
p4pd_get_stage0_prog_addr(uint64_t* offset)
{
    char progname[] = "rxdma_stage0.bin";
    char labelname[]= "tcp_rx_stage0";

    int ret = capri_program_label_to_offset("p4plus",
                                            progname,
                                            labelname,
                                            offset);
    if(ret < 0) {
        return HAL_RET_HW_FAIL;
    }
    return HAL_RET_OK;
}

static hal_ret_t 
p4pd_add_or_del_tcp_rx_read_tx2rx_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_read_tx2rx_actiondata_d  data = {0};
    hal_ret_t                       ret = HAL_RET_OK;
    uint64_t                        pc_offset;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_READ_TX2RX);

    if(!del) {
        // get pc address
        if(p4pd_get_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
            //ret = HAL_RET_HW_FAIL;
        }
        HAL_TRACE_DEBUG("Received pc address", pc_offset);
        data.pc = pc_offset;
    }

    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
        HAL_TRACE_ERR("Failed to create rx: read_tx2rx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_tcp_rx_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_tcp_rx_d             data = {0};
    hal_ret_t                   ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_RX);

    if(!del) {
       // data.u.tcp_rx_d.rcv_nxt = tcpcb_pd->tcpcb->rcv_nxt;
        //data.u.tcp_rx_d.snd_una = tcpcb_pd->tcpcb->snd_una;
        data.u.tcp_rx_d.rcv_nxt = 0xBABABABA;
        data.u.tcp_rx_d.snd_una = 0xEFEFEFEF;
        data.u.tcp_rx_d.rcv_tsval = 0xFAFAFAFA;
        data.u.tcp_rx_d.ts_recent = 0xFAFAFAF0;
        
        // Get Serq address
        wring_hw_id_t  serq_base;
        ret = wring_pd_get_base_addr(types::WRING_TYPE_SERQ,
                                     tcpcb_pd->tcpcb->cb_id,
                                     &serq_base);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to receive serq base for tcp cb: {}", 
                        tcpcb_pd->tcpcb->cb_id);
        } else {
            HAL_TRACE_DEBUG("Serq base: 0x{0:x}", serq_base);
            data.u.tcp_rx_d.serq_base = serq_base;    
        }
    }
    int size = sizeof(tcp_rx_tcp_rx_d);
    
    if(!p4plus_hbm_write(hwid, (uint8_t *)&data, size)) {
        HAL_TRACE_ERR("Failed to create rx: tcp_rx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_tcp_rtt_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_tcp_rtt_d   data = {0};
    hal_ret_t          ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_RTT_ID);

    if(!del) {
        data.u.tcp_rtt_d.rto = 0x30;
        data.u.tcp_rtt_d.srtt_us = 0x80;
        data.u.tcp_rtt_d.seq_rtt_us = 0x10;
        data.u.tcp_rtt_d.ca_rtt_us = 0x10;
        data.u.tcp_rtt_d.curr_ts = 0xf0;
        data.u.tcp_rtt_d.rtt_min = 0x1;
        data.u.tcp_rtt_d.rttvar_us = 0x20;
        data.u.tcp_rtt_d.mdev_us = 0x20;
        data.u.tcp_rtt_d.mdev_max_us = 0;
        data.u.tcp_rtt_d.rtt_seq = 0x20;
    }
    
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
        HAL_TRACE_ERR("Failed to create rx: tcp_rtt entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_read_rnmdr_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_read_rnmdr_d    data = {0};
    hal_ret_t                       ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_READ_RNMDR_ID);

    if(!del) {
    }
    
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
        HAL_TRACE_ERR("Failed to create rx: read_tx2rx entry for read rnmdr TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_read_rnmpr_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_read_rnmpr_d    data = {0};
    hal_ret_t                       ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_READ_RNMPR_ID);

    if(!del) {
    }
    
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
        HAL_TRACE_ERR("Failed to create rx: read_tx2rx entry for read rnmdr TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_read_serq_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_read_serq_d     data = {0};
    hal_ret_t                       ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_READ_SERQ);

    if(!del) {
    }
    
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
        HAL_TRACE_ERR("Failed to create rx: read_tx2rx entry for read rnmdr TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_tcp_fra_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_tcp_fra_d   data = {0};
    hal_ret_t                   ret = HAL_RET_OK;
    
    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_FRA);

    if(!del) {
        data.u.tcp_fra_d.ca_state = 0x2;
        data.u.tcp_fra_d.high_seq = 0xEFEFEFEF;
    }
    
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
        HAL_TRACE_ERR("Failed to create rx: tcp_fra entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_rdesc_alloc_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_rdesc_alloc_d   data = {0};
    hal_ret_t                       ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_RDESC_ALLOC);

    if(!del) {
    }
    
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
        HAL_TRACE_ERR("Failed to create rx: read_tx2rx entry for read rnmdr TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_rpage_alloc_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_rpage_alloc_d   data = {0};
    hal_ret_t                       ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_RPAGE_ALLOC);

    if(!del) {
    }
    
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
        HAL_TRACE_ERR("Failed to create rx: read_tx2rx entry for read rnmdr TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}



hal_ret_t 
p4pd_add_or_del_tcp_rx_tcp_cc_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_tcp_cc_d   data = {0};
    hal_ret_t                  ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_CC);

    if(!del) {
        data.u.tcp_cc_d.snd_cwnd = 0x10;
        data.u.tcp_cc_d.max_packets_out = 0x07;
        data.u.tcp_cc_d.is_cwnd_limited = 0x00;
        data.u.tcp_cc_d.last_max_cwnd = 0x16;
    }
    
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
        HAL_TRACE_ERR("Failed to create rx: tcp_cc entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_write_serq_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_write_serq_d    data = {0};
    hal_ret_t                       ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_WRITE_SERQ);

    if(!del) {
    }
    
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
        HAL_TRACE_ERR("Failed to create rx: read_tx2rx entry for read rnmdr TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}


hal_ret_t 
p4pd_add_or_del_tcpcb_rxdma_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_tcp_rx_read_tx2rx_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tcp_rx_tcp_rx_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tcp_rx_tcp_rtt_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    
    ret = p4pd_add_or_del_tcp_rx_read_rnmdr_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tcp_rx_read_rnmpr_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tcp_rx_read_serq_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tcp_rx_tcp_fra_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
 
    ret = p4pd_add_or_del_tcp_rx_rdesc_alloc_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

   ret = p4pd_add_or_del_tcp_rx_rpage_alloc_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tcp_rx_tcp_cc_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tcp_rx_write_serq_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }


    return HAL_RET_OK;
cleanup:
    /* TODO: CLEANUP */
    return ret;
}

hal_ret_t 
p4pd_get_tcp_rx_tcp_rx_entry(pd_tcpcb_t* tcpcb_pd)
{
    tcp_rx_tcp_rx_d    data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_RX);

    if(!p4plus_hbm_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create rx: tcp_rx entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    tcpcb_pd->tcpcb->rcv_nxt = data.u.tcp_rx_d.rcv_nxt;
    tcpcb_pd->tcpcb->snd_una = data.u.tcp_rx_d.snd_una;
    tcpcb_pd->tcpcb->rcv_tsval = data.u.tcp_rx_d.rcv_tsval;
    tcpcb_pd->tcpcb->ts_recent = data.u.tcp_rx_d.ts_recent;
    tcpcb_pd->tcpcb->serq_base = data.u.tcp_rx_d.serq_base;

    HAL_TRACE_DEBUG("Received serq_base: 0x{0:x}", tcpcb_pd->tcpcb->serq_base);

    return HAL_RET_OK;
}

hal_ret_t 
p4pd_get_tcpcb_rxdma_entry(pd_tcpcb_t* tcpcb_pd)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_get_tcp_rx_tcp_rx_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get tcp_rx entry");
        goto cleanup;
    }
    return HAL_RET_OK;
cleanup:
    /* TODO: CLEANUP */
    return ret;
}

/********************************************
 * TxDMA
 * ******************************************/

hal_ret_t 
p4pd_add_or_del_tcp_tx_read_rx2tx_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_tx_read_rx2tx_d   data = {0};
    hal_ret_t                      ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_READ_RX2TX);
    
    if(!del) {
    }
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
        HAL_TRACE_ERR("Failed to create tx: read_rx2tx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_tx_read_rx2tx_extra_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_tx_read_rx2tx_extra_d      data = {0};
    hal_ret_t                               ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_READ_RX2TX_EXTRA);
    
    if(!del) {
    }
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
        HAL_TRACE_ERR("Failed to create tx: read_rx2tx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}


hal_ret_t 
p4pd_add_or_del_tcp_tx_read_sesq_ci_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_tx_read_sesq_ci_d      data = {0};
    hal_ret_t                           ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_READ_SESQ_CI);
    
    if(!del) {
    }
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
        HAL_TRACE_ERR("Failed to create tx: read_rx2tx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}


hal_ret_t 
p4pd_add_or_del_tcp_tx_read_sesq_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_tx_read_sesq_d    data = {0};
    hal_ret_t               ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_READ_SESQ);
    
    if(!del) {
    }
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
        HAL_TRACE_ERR("Failed to create tx: read_rx2tx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}


hal_ret_t 
p4pd_add_or_del_tcp_tx_sesq_consume_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_tx_sesq_consume_d     data = {0};
    hal_ret_t                          ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_SESQ_CONSUME);
    
    if(!del) {
    }
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
        HAL_TRACE_ERR("Failed to create tx: read_rx2tx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}


hal_ret_t 
p4pd_add_or_del_tcpcb_txdma_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_tcp_tx_read_rx2tx_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    
    ret = p4pd_add_or_del_tcp_tx_read_rx2tx_extra_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    ret = p4pd_add_or_del_tcp_tx_read_sesq_ci_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    ret = p4pd_add_or_del_tcp_tx_read_sesq_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
 
    ret = p4pd_add_or_del_tcp_tx_sesq_consume_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
 
    return HAL_RET_OK;

cleanup:

    /* TODO: Cleanup */
    return ret;
}

hal_ret_t 
p4pd_get_tcpcb_txdma_entry(pd_tcpcb_t* tcpcb_pd)
{
    /* TODO */
    return HAL_RET_OK;
}

/**************************/

tcpcb_hw_id_t
pd_tcpcb_get_base_hw_index(pd_tcpcb_t* tcpcb_pd)
{
    HAL_ASSERT(NULL != tcpcb_pd);
    HAL_ASSERT(NULL != tcpcb_pd->tcpcb);
    
    
    char tcpcb_reg[10] = "tcpcb";
    uint64_t offset = get_start_offset(tcpcb_reg);
    HAL_TRACE_DEBUG("received offset ", offset);
    return offset + \
        (tcpcb_pd->tcpcb->cb_id * P4PD_HBM_TCP_CB_ENTRY_SIZE);
}

hal_ret_t
p4pd_add_or_del_tcpcb_entry(pd_tcpcb_t* tcpcb_pd, bool del) 
{
    hal_ret_t                   ret = HAL_RET_OK;
 
    ret = p4pd_add_or_del_tcpcb_rxdma_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto err;    
    }
   
    ret = p4pd_add_or_del_tcpcb_txdma_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto err;    
    }
    ret = capri_lif_qstate_create(1001);

err:
    /*TODO: cleanup */
    return ret;
}

static
hal_ret_t
p4pd_get_tcpcb_entry(pd_tcpcb_t* tcpcb_pd) 
{
    hal_ret_t                   ret = HAL_RET_OK;
 
    ret = p4pd_get_tcpcb_rxdma_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        goto err;    
    }
   
    ret = p4pd_get_tcpcb_txdma_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        goto err;    
    }

err:
    /*TODO: cleanup */
    return ret;
}

/********************************************
 * APIs
 *******************************************/

hal_ret_t
pd_tcpcb_create (pd_tcpcb_args_t *args)
{
    hal_ret_t               ret;
    pd_tcpcb_s              *tcpcb_pd;

    HAL_TRACE_DEBUG("Creating pd state for TCP CB.");

    // allocate PD tcpcb state
    tcpcb_pd = tcpcb_pd_alloc_init();
    if (tcpcb_pd == NULL) {
        return HAL_RET_OOM;
    }
    HAL_TRACE_DEBUG("Alloc done");
    tcpcb_pd->tcpcb = args->tcpcb;
    // get hw-id for this TCPCB
    tcpcb_pd->hw_id = pd_tcpcb_get_base_hw_index(tcpcb_pd);
    printf("Received hw-id: 0x%lx ", tcpcb_pd->hw_id);
    
    // program tcpcb
    ret = p4pd_add_or_del_tcpcb_entry(tcpcb_pd, false);
    if(ret != HAL_RET_OK) {
        goto cleanup;    
    }
    HAL_TRACE_DEBUG("Programming done");
    // add to db
    ret = add_tcpcb_pd_to_db(tcpcb_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    HAL_TRACE_DEBUG("DB add done");
    args->tcpcb->pd = tcpcb_pd;

    return HAL_RET_OK;

cleanup:

    if (tcpcb_pd) {
        tcpcb_pd_free(tcpcb_pd);
    }
    return ret;
}

hal_ret_t
pd_tcpcb_update (pd_tcpcb_args_t *args)
{
    hal_ret_t               ret;
    
    if(!args) {
       return HAL_RET_INVALID_ARG; 
    }

    tcpcb_t*                tcpcb = args->tcpcb;
    pd_tcpcb_t*             tcpcb_pd = (pd_tcpcb_t*)tcpcb->pd;

    HAL_TRACE_DEBUG("TCPCB pd update");
    
    // program tcpcb
    ret = p4pd_add_or_del_tcpcb_entry(tcpcb_pd, false);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update tcpcb");
    }
    return ret;
}

hal_ret_t
pd_tcpcb_delete (pd_tcpcb_args_t *args)
{
    hal_ret_t               ret;
    
    if(!args) {
       return HAL_RET_INVALID_ARG; 
    }

    tcpcb_t*                tcpcb = args->tcpcb;
    pd_tcpcb_t*             tcpcb_pd = (pd_tcpcb_t*)tcpcb->pd;

    HAL_TRACE_DEBUG("TCPCB pd delete");
    
    // program tcpcb
    ret = p4pd_add_or_del_tcpcb_entry(tcpcb_pd, true);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete tcpcb entry"); 
    }
    
    del_tcpcb_pd_from_db(tcpcb_pd);

    tcpcb_pd_free(tcpcb_pd);

    return ret;
}

hal_ret_t
pd_tcpcb_get (pd_tcpcb_args_t *args)
{
    hal_ret_t               ret;
    pd_tcpcb_t              tcpcb_pd;

    HAL_TRACE_DEBUG("TCPCB pd get for id: {}", args->tcpcb->cb_id);

    // allocate PD tcpcb state
    tcpcb_pd_init(&tcpcb_pd);

    HAL_TRACE_DEBUG("Alloc done");
    tcpcb_pd.tcpcb = args->tcpcb;

    // get hw-id for this TCPCB
    tcpcb_pd.hw_id = pd_tcpcb_get_base_hw_index(&tcpcb_pd);
    HAL_TRACE_DEBUG("Received hw-id 0x{0:x}", tcpcb_pd.hw_id);

    // get hw tcpcb entry
    ret = p4pd_get_tcpcb_entry(&tcpcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for id: 0x{0:x}", tcpcb_pd.tcpcb->cb_id);
    }
    return ret;
}

}    // namespace pd
}    // namespace hal

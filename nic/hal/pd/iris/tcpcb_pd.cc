#include <base.h>
#include <hal_lock.hpp>
#include <pd_api.hpp>
#include <tcpcb_pd.hpp>
#include <p4pd_tcp_proxy_api.h>
#include <capri_loader.h>
#include <capri_hbm.hpp>

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
    char progname[] = "abc";
    char labelname[]= "xyz";

    int ret = capri_program_label_to_offset(progname,
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
    tcp_rx_read_tx2rx_actiondata     data = {0};
    p4pd_error_t                     pd_err = P4PD_SUCCESS;
    hal_ret_t                        ret = HAL_RET_OK;
    uint64_t                         pc_offset;
    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_READ_TX2RX);

    if(!del) {
        data.actionid = TCP_RX_READ_TX2RX_READ_TX2RX_ID;
        
        // get pc address
        if(p4pd_get_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
            ret = HAL_RET_HW_FAIL;
        }
    }
    pd_err = p4pd_tcp_proxy_rxdma_entry_write(P4_TCP_PROXY_RXDMA_TBL_ID_TCP_RX_READ_TX2RX, 
                                              hwid, 
                                              NULL, 
                                              NULL, 
                                              &data);
    if(pd_err != P4PD_SUCCESS) {
        HAL_TRACE_ERR("Failed to create rx: read_tx2rx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_tcp_rx_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_tcp_rx_actiondata    data = {0};
    p4pd_error_t                pd_err = P4PD_SUCCESS;
    hal_ret_t                   ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_RX);

    if(!del) {
        data.actionid = TCP_RX_TCP_RX_TCP_RX_ID;
        data.tcp_rx_tcp_rx_action_u.tcp_rx_tcp_rx_tcp_rx.rcv_nxt = 0x10;
        data.tcp_rx_tcp_rx_action_u.tcp_rx_tcp_rx_tcp_rx.rcv_tsval = 0xFA;
        data.tcp_rx_tcp_rx_action_u.tcp_rx_tcp_rx_tcp_rx.ts_recent = 0xF0;
    }

    pd_err = p4pd_tcp_proxy_rxdma_entry_write(P4_TCP_PROXY_RXDMA_TBL_ID_TCP_RX_READ_TX2RX, 
                                              hwid, 
                                              NULL, 
                                              NULL, 
                                              &data);
    if(pd_err != P4PD_SUCCESS) {
        HAL_TRACE_ERR("Failed to create rx: tcp_rx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_tcp_rtt_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_tcp_rtt_actiondata   data = {0};
    p4pd_error_t                pd_err = P4PD_SUCCESS;
    hal_ret_t                   ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_RTT_ID);

    if(!del) {
        data.actionid = TCP_RX_TCP_RTT_TCP_RTT_ID;
        data.tcp_rx_tcp_rtt_action_u.tcp_rx_tcp_rtt_tcp_rtt.rto = 0x30;
        data.tcp_rx_tcp_rtt_action_u.tcp_rx_tcp_rtt_tcp_rtt.srtt_us = 0x80;
        data.tcp_rx_tcp_rtt_action_u.tcp_rx_tcp_rtt_tcp_rtt.seq_rtt_us = 0x10;
        data.tcp_rx_tcp_rtt_action_u.tcp_rx_tcp_rtt_tcp_rtt.ca_rtt_us = 0x10;
        data.tcp_rx_tcp_rtt_action_u.tcp_rx_tcp_rtt_tcp_rtt.curr_ts = 0xf0;
        data.tcp_rx_tcp_rtt_action_u.tcp_rx_tcp_rtt_tcp_rtt.rtt_min = 0x1;
        data.tcp_rx_tcp_rtt_action_u.tcp_rx_tcp_rtt_tcp_rtt.rttvar_us = 0x20;
        data.tcp_rx_tcp_rtt_action_u.tcp_rx_tcp_rtt_tcp_rtt.mdev_us = 0x20;
        data.tcp_rx_tcp_rtt_action_u.tcp_rx_tcp_rtt_tcp_rtt.mdev_max_us = 0;
        data.tcp_rx_tcp_rtt_action_u.tcp_rx_tcp_rtt_tcp_rtt.rtt_seq = 0x20;
    }

    pd_err = p4pd_tcp_proxy_rxdma_entry_write(P4_TCP_PROXY_RXDMA_TBL_ID_TCP_RX_TCP_RTT, 
                                              hwid, 
                                              NULL, 
                                              NULL, 
                                              &data);
    if(pd_err != P4PD_SUCCESS) {
        HAL_TRACE_ERR("Failed to create rx: tcp_rtt entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_read_rnmdr_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_read_rnmdr_actiondata    data = {0};
    p4pd_error_t                    pd_err = P4PD_SUCCESS;
    hal_ret_t                       ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_READ_RNMDR_ID);

    if(!del) {
        data.actionid = TCP_RX_READ_RNMDR_READ_RNMDR_ID;
    }
    pd_err = p4pd_tcp_proxy_rxdma_entry_write(P4_TCP_PROXY_RXDMA_TBL_ID_TCP_RX_READ_RNMDR, 
                                              hwid, 
                                              NULL, 
                                              NULL, 
                                              &data);
    if(pd_err != P4PD_SUCCESS) {
        HAL_TRACE_ERR("Failed to create rx: read_tx2rx entry for read rnmdr TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_read_rnmpr_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_read_rnmpr_actiondata    data = {0};
    p4pd_error_t                    pd_err = P4PD_SUCCESS;
    hal_ret_t                       ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_READ_RNMPR_ID);

    if(!del) {
        data.actionid = TCP_RX_READ_RNMPR_READ_RNMPR_ID;
    }
    pd_err = p4pd_tcp_proxy_rxdma_entry_write(P4_TCP_PROXY_RXDMA_TBL_ID_TCP_RX_READ_RNMPR, 
                                              hwid, 
                                              NULL, 
                                              NULL, 
                                              &data);
    if(pd_err != P4PD_SUCCESS) {
        HAL_TRACE_ERR("Failed to create rx: read_tx2rx entry for read rnmdr TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_read_serq_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_read_serq_actiondata     data = {0};
    p4pd_error_t                    pd_err = P4PD_SUCCESS;
    hal_ret_t                       ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_READ_SERQ);

    if(!del) {
        data.actionid = TCP_RX_READ_SERQ_READ_SERQ_ID;;
    }
    pd_err = p4pd_tcp_proxy_rxdma_entry_write(P4_TCP_PROXY_RXDMA_TBL_ID_TCP_RX_READ_SERQ, 
                                              hwid, 
                                              NULL, 
                                              NULL, 
                                              &data);
    if(pd_err != P4PD_SUCCESS) {
        HAL_TRACE_ERR("Failed to create rx: read_tx2rx entry for read rnmdr TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_tcp_fra_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_tcp_fra_actiondata   data = {0};
    p4pd_error_t                pd_err = P4PD_SUCCESS;
    hal_ret_t                   ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_FRA);

    if(!del) {
        data.actionid = TCP_RX_TCP_FRA_TCP_FRA_ID;
        data.tcp_rx_tcp_fra_action_u.tcp_rx_tcp_fra_tcp_fra.ca_state = 0x2;
        data.tcp_rx_tcp_fra_action_u.tcp_rx_tcp_fra_tcp_fra.high_seq = 0x10;
    }
    pd_err = p4pd_tcp_proxy_rxdma_entry_write(P4_TCP_PROXY_RXDMA_TBL_ID_TCP_RX_TCP_FRA, 
                                              hwid, 
                                              NULL, 
                                              NULL, 
                                              &data);
    if(pd_err != P4PD_SUCCESS) {
        HAL_TRACE_ERR("Failed to create rx: tcp_fra entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_rdesc_alloc_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_rdesc_alloc_actiondata   data = {0};
    p4pd_error_t                    pd_err = P4PD_SUCCESS;
    hal_ret_t                       ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_RDESC_ALLOC);

    if(!del) {
        data.actionid = TCP_RX_RDESC_ALLOC_RDESC_ALLOC_ID;
    }
    pd_err = p4pd_tcp_proxy_rxdma_entry_write(P4_TCP_PROXY_RXDMA_TBL_ID_TCP_RX_RDESC_ALLOC, 
                                              hwid, 
                                              NULL, 
                                              NULL, 
                                              &data);
    if(pd_err != P4PD_SUCCESS) {
        HAL_TRACE_ERR("Failed to create rx: read_tx2rx entry for read rnmdr TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_rpage_alloc_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_rpage_alloc_actiondata   data = {0};
    p4pd_error_t                    pd_err = P4PD_SUCCESS;
    hal_ret_t                       ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_RPAGE_ALLOC);

    if(!del) {
        data.actionid = TCP_RX_RPAGE_ALLOC_RPAGE_ALLOC_ID ;
    }
    pd_err = p4pd_tcp_proxy_rxdma_entry_write(P4_TCP_PROXY_RXDMA_TBL_ID_TCP_RX_RPAGE_ALLOC, 
                                              hwid, 
                                              NULL, 
                                              NULL, 
                                              &data);
    if(pd_err != P4PD_SUCCESS) {
        HAL_TRACE_ERR("Failed to create rx: read_tx2rx entry for read rnmdr TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}



hal_ret_t 
p4pd_add_or_del_tcp_rx_tcp_cc_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_tcp_cc_actiondata   data = {0};
    p4pd_error_t               pd_err = P4PD_SUCCESS;
    hal_ret_t                  ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_CC);

    if(!del) {
        data.actionid = TCP_RX_TCP_CC_TCP_CC_ID;
        data.tcp_rx_tcp_cc_action_u.tcp_rx_tcp_cc_tcp_cc.snd_cwnd = 0x10;
        data.tcp_rx_tcp_cc_action_u.tcp_rx_tcp_cc_tcp_cc.max_packets_out = 0x07;
        data.tcp_rx_tcp_cc_action_u.tcp_rx_tcp_cc_tcp_cc.is_cwnd_limited = 0x00;
        data.tcp_rx_tcp_cc_action_u.tcp_rx_tcp_cc_tcp_cc.last_max_cwnd = 0x16;
    }
    pd_err = p4pd_tcp_proxy_rxdma_entry_write(P4_TCP_PROXY_RXDMA_TBL_ID_TCP_RX_TCP_CC, 
                                              hwid, 
                                              NULL, 
                                              NULL, 
                                              &data);
    if(pd_err != P4PD_SUCCESS) {
        HAL_TRACE_ERR("Failed to create rx: tcp_cc entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_write_serq_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_rx_write_serq_actiondata    data = {0};
    p4pd_error_t                    pd_err = P4PD_SUCCESS;
    hal_ret_t                       ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_WRITE_SERQ);

    if(!del) {
        data.actionid = TCP_RX_WRITE_SERQ_WRITE_SERQ_ID;
    }
    pd_err = p4pd_tcp_proxy_rxdma_entry_write(P4_TCP_PROXY_RXDMA_TBL_ID_TCP_RX_WRITE_SERQ, 
                                              hwid, 
                                              NULL, 
                                              NULL, 
                                              &data);
    if(pd_err != P4PD_SUCCESS) {
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


/********************************************
 * TxDMA
 * ******************************************/

hal_ret_t 
p4pd_add_or_del_tcp_tx_read_rx2tx_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_tx_read_rx2tx_actiondata   data = {0};
    p4pd_error_t                   pd_err = P4PD_SUCCESS;
    hal_ret_t                      ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_READ_RX2TX);
    
    if(!del) {
        data.actionid = TCP_TX_READ_RX2TX_READ_RX2TX_ID;
    }
    pd_err = p4pd_tcp_proxy_txdma_entry_write(P4_TCP_PROXY_TXDMA_TBL_ID_TCP_TX_READ_RX2TX, 
                                              hwid, 
                                              NULL, 
                                              NULL, 
                                              &data);
    if(pd_err != P4PD_SUCCESS) {
        HAL_TRACE_ERR("Failed to create tx: read_rx2tx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_tx_read_rx2tx_extra_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_tx_read_rx2tx_extra_actiondata      data = {0};
    p4pd_error_t                            pd_err = P4PD_SUCCESS;
    hal_ret_t                               ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_READ_RX2TX_EXTRA);
    
    if(!del) {
        data.actionid = TCP_TX_READ_RX2TX_EXTRA_READ_RX2TX_EXTRA_ID;
    }
    pd_err = p4pd_tcp_proxy_txdma_entry_write(P4_TCP_PROXY_TXDMA_TBL_ID_TCP_TX_READ_RX2TX_EXTRA, 
                                              hwid, 
                                              NULL, 
                                              NULL, 
                                              &data);
    if(pd_err != P4PD_SUCCESS) {
        HAL_TRACE_ERR("Failed to create tx: read_rx2tx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}


hal_ret_t 
p4pd_add_or_del_tcp_tx_read_sesq_ci_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_tx_read_sesq_ci_actiondata      data = {0};
    p4pd_error_t                        pd_err = P4PD_SUCCESS;
    hal_ret_t                           ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_READ_SESQ_CI);
    
    if(!del) {
        data.actionid = TCP_TX_READ_SESQ_CI_READ_SESQ_CI_ID;
    }
    pd_err = p4pd_tcp_proxy_txdma_entry_write(P4_TCP_PROXY_TXDMA_TBL_ID_TCP_TX_READ_SESQ_CI, 
                                              hwid, 
                                              NULL, 
                                              NULL, 
                                              &data);
    if(pd_err != P4PD_SUCCESS) {
        HAL_TRACE_ERR("Failed to create tx: read_rx2tx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}


hal_ret_t 
p4pd_add_or_del_tcp_tx_read_sesq_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_tx_read_sesq_actiondata    data = {0};
    p4pd_error_t            pd_err = P4PD_SUCCESS;
    hal_ret_t               ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_READ_SESQ);
    
    if(!del) {
        data.actionid = TCP_TX_READ_SESQ_READ_SESQ_ID;
    }
    pd_err = p4pd_tcp_proxy_txdma_entry_write(P4_TCP_PROXY_TXDMA_TBL_ID_TCP_TX_READ_SESQ, 
                                              hwid, 
                                              NULL, 
                                              NULL, 
                                              &data);
    if(pd_err != P4PD_SUCCESS) {
        HAL_TRACE_ERR("Failed to create tx: read_rx2tx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}


hal_ret_t 
p4pd_add_or_del_tcp_tx_sesq_consume_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    tcp_tx_sesq_consume_actiondata     data = {0};
    p4pd_error_t                       pd_err = P4PD_SUCCESS;
    hal_ret_t                          ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_SESQ_CONSUME);
    
    if(!del) {
        data.actionid = TCP_TX_SESQ_CONSUME_SESQ_CONSUME_ID;
    }
    pd_err = p4pd_tcp_proxy_txdma_entry_write(P4_TCP_PROXY_TXDMA_TBL_ID_TCP_TX_SESQ_CONSUME, 
                                              hwid, 
                                              NULL, 
                                              NULL, 
                                              &data);
    if(pd_err != P4PD_SUCCESS) {
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

/**************************/

tcpcb_hw_id_t
pd_tcpcb_get_base_hw_index(pd_tcpcb_t* tcpcb_pd)
{
    HAL_ASSERT(NULL != tcpcb_pd);
    HAL_ASSERT(NULL != tcpcb_pd->tcpcb);
    
    char tcpcb_reg[] = "tcpcb";
    return get_start_offset(tcpcb_reg) + \
        (tcpcb_pd->tcpcb->cb_id * P4PD_HBM_TCP_CB_ENTRY_SIZE);
}

static
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

err:
    /*TODO: cleanup */
    return ret;
}


hal_ret_t
pd_tcpcb_create (pd_tcpcb_args_t *args)
{
    hal_ret_t               ret;
    pd_tcpcb_s              *tcpcb_pd;

    HAL_TRACE_DEBUG("Creating pd state for TCP CB");

    // allocate PD tcpcb state
    tcpcb_pd = tcpcb_pd_alloc_init();
    if (tcpcb_pd == NULL) {
        return HAL_RET_OOM;
    }
    tcpcb_pd->tcpcb = args->tcpcb;

    // get hw-id for this TCPCB
    tcpcb_pd->hw_id = pd_tcpcb_get_base_hw_index(tcpcb_pd);

    // program tcpcb
    ret = p4pd_add_or_del_tcpcb_entry(tcpcb_pd, false);
    if(ret != HAL_RET_OK) {
        goto cleanup;    
    }

    // add to db
    ret = add_tcpcb_pd_to_db(tcpcb_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    args->tcpcb->pd = tcpcb_pd;

    return HAL_RET_OK;

cleanup:

    if (tcpcb_pd) {
        tcpcb_pd_free(tcpcb_pd);
    }
    return ret;
}

}    // namespace pd
}    // namespace hal

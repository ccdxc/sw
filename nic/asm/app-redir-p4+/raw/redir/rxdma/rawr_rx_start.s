#include "app_redir_common.h"

struct phv_                                     p;
struct common_p4plus_stage0_app_header_table_k  k;
struct common_p4plus_stage0_app_header_table_d  d;

%%
    .param          rawr_s1_desc_sem_pindex_post_update
    .param          rawr_s1_ppage_sem_pindex_post_update
    .param          rawr_s1_mpage_sem_pindex_post_update
    
    .align

rawr_s0_rx_start:
        
    /*
     * Stage 0 table engine always fetches the qstate (in d-vector)
     * for processing by the stage 0 MPUs.
     * 
     * In addition, 
     * R1 = table lookup hash value
     * R2 = packet size
     * R3 = random number
     * R4 = current time
     * R5 = programmable table constant
     * R6 = phv timestamp
     * R7 = qstate ring_not_empty
     */
     
    CAPRI_CLEAR_TABLE0_VALID
    
    /*
     * qid is our flow ID context:
     */
    phvwr       p.pen_raw_redir_hdr_v1_flow_id, CAPRI_RXDMA_INTRINSIC_QID
    
    /*
     * temporarily remove access to tm_instance_type until issue of qid..qtype
     * range is solved in RDMA
    seq         c1, CAPRI_INTRINSIC_TM_INSTANCE_TYPE, TM_INSTANCE_TYPE_SPAN
    phvwr.c1    p.common_phv_redir_span_instance, TRUE
     */

    /*
     * For a given flow, one of 2 types of redirection applies:
     *   1) Redirect to ARM CPU RxQ, or
     *   2) Redirect to a P4+ TxQ
     */
    add         r3, r0, d.u.rawr_rx_start_d.chain_rxq_base
    add         r4, r0, d.u.rawr_rx_start_d.chain_txq_base
    sne         c1, r3, r0
    sne         c2, r4, r0
    bcf         [c1 & c2],   qstate_cfg_err_discard
    nop
    bcf         [!c1 & !c2], qstate_cfg_err_discard
    phvwr.c1    p.common_phv_chain_to_rxq, TRUE         // delay slot
    phvwr.c1    p.common_phv_chain_ring_base, r3
    phvwr.c1    p.common_phv_chain_ring_size_shift, d.u.rawr_rx_start_d.chain_rxq_ring_size_shift
    phvwr.c1    p.common_phv_chain_entry_size_shift, d.u.rawr_rx_start_d.chain_rxq_entry_size_shift
    phvwr.c1    p.common_phv_chain_ring_index_select, d.u.rawr_rx_start_d.chain_rxq_ring_index_select
    phvwr.c1    p.to_s5_chain_ring_indices_addr, d.u.rawr_rx_start_d.chain_rxq_ring_indices_addr
    
    phvwr.!c1   p.common_phv_chain_ring_base, r4
    phvwr.!c1   p.common_phv_chain_ring_size_shift, d.u.rawr_rx_start_d.chain_txq_ring_size_shift
    phvwr.!c1   p.common_phv_chain_entry_size_shift, d.u.rawr_rx_start_d.chain_txq_entry_size_shift
    phvwr.!c1   p.common_phv_chain_lif, d.u.rawr_rx_start_d.chain_txq_lif
    phvwr.!c1   p.common_phv_chain_qtype, d.u.rawr_rx_start_d.chain_txq_qtype
    phvwr.!c1   p.common_phv_chain_qid, d.u.rawr_rx_start_d.chain_txq_qid
    phvwr.!c1   p.common_phv_chain_ring_index_select, d.u.rawr_rx_start_d.chain_txq_ring_index_select
    phvwr.!c1   p.to_s5_chain_ring_indices_addr, d.u.rawr_rx_start_d.chain_txq_ring_indices_addr
    
    phvwr       p.common_phv_chain_doorbell_no_sched, d.u.rawr_rx_start_d.chain_txq_doorbell_no_sched
    phvwr       p.common_phv_desc_valid_bit_req, d.u.rawr_rx_start_d.desc_valid_bit_req
    phvwr       p.common_phv_redir_pipeline_lpbk_enable, d.u.rawr_rx_start_d.redir_pipeline_lpbk_enable
    
    /*
     * Packet_len field contains
     *   sizeof(p4_to_p4plus_cpu_pkt_t) + complete packet length
     * to which we will add an app header of size P4PLUS_RAW_REDIR_HDR_SZ
     */
    add         r3, r0, k.rawr_app_header_packet_len
    ble.s       r3, r0, packet_len_err_discard
    addi        r3, r3, P4PLUS_RAW_REDIR_HDR_SZ         // delay slot
    phvwr       p.common_phv_packet_len, r3

    /*
     * Allocate either one meta page, or one packet page, or both.
     */
    blei        r3, APP_REDIR_MPAGE_SIZE, mpage_sem_pindex_fetch_update
    nop
    
ppage_sem_pindex_fetch_update:

    /*
     * Fetch/update memory page pindex for storing packet
     */
    APP_REDIR_ALLOC_SEM_PINDEX_FETCH_UPDATE(1, r4,
                                            APP_REDIR_RNMPR_ALLOC_IDX,
                                            rawr_s1_ppage_sem_pindex_post_update)
                                       
    blei        r3, APP_REDIR_PPAGE_SIZE, desc_sem_pindex_fetch_update
    nop
    
mpage_sem_pindex_fetch_update:

    /*
     * Fetch/update memory page pindex for storing metaheader
     */
    APP_REDIR_ALLOC_SEM_PINDEX_FETCH_UPDATE(2, r4,
                                            APP_REDIR_RNMPR_SMALL_ALLOC_IDX,
                                            rawr_s1_mpage_sem_pindex_post_update)
desc_sem_pindex_fetch_update:

    /*
     * Fetch/update buffer descriptor pindex for queuing to chain ring
     */
    APP_REDIR_ALLOC_SEM_PINDEX_FETCH_UPDATE(0, r4,
                                            APP_REDIR_RNMDR_ALLOC_IDX,
                                            rawr_s1_desc_sem_pindex_post_update)
    nop.e
    nop

packet_len_err_discard:

    /*
     * Discard packet due to invalid packet length;
     * TODO: add stats here
     */
    phvwr.e     p.p4_intr_global_drop, 1
    nop    

    
qstate_cfg_err_discard:

    /*
     * Discard packet due to error in qstate configuration
     * TODO: add stats here
     */
    phvwr.e     p.p4_intr_global_drop, 1
    nop


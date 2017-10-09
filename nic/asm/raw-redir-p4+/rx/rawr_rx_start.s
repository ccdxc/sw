#include "rawr-defines.h"

struct phv_                                     p;
struct common_p4plus_stage0_app_header_table_k  k;
struct common_p4plus_stage0_app_header_table_rawr_rx_start_d d;

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
     * qid is our flow ID context
     */
    CAPRI_OPERAND_DEBUG(k.p4_rxdma_intr_qid)
    phvwr       p.common_phv_flow_id, k.p4_rxdma_intr_qid

    phvwr       p.common_phv_chain_rxq_base, d.chain_rxq_base
    phvwr       p.common_phv_chain_qidxr_pi, d.chain_qidxr_pi
    phvwr       p.common_phv_chain_rxq_ring_size_shift, d.chain_rxq_ring_size_shift
    phvwr       p.common_phv_chain_rxq_entry_size_shift, d.chain_rxq_entry_size_shift
    phvwr       p.common_phv_desc_valid_bit_req, d.desc_valid_bit_req
    
    phvwr       p.to_s5_chain_rxq_sem_alloc_idx, d.chain_rxq_sem_alloc_idx
    phvwr       p.to_s5_chain_rxq_qidxr_base, d.chain_rxq_qidxr_base
    
    /*
     * Packet_len field contains
     *   sizeof(p4_to_p4plus_cpu_pkt_t) + complete packet length
     */
    add         r3, r0, k.rawr_app_header_packet_len
    ble.s       r3, r0, packet_len_err_discard
    phvwr       p.common_phv_packet_len, r3             // delay slot

    /*
     * Allocate either one meta page, or one packet page, or both.
     */
    blei        r3, RAWR_MPAGE_SIZE, mpage_sem_pindex_fetch_update
    nop
    
ppage_sem_pindex_fetch_update:

    /*
     * Fetch/update memory page pindex for storing packet
     */
    RAWR_ALLOC_SEM_PINDEX_FETCH_UPDATE(1, r4,
                                       RAWRPR_ALLOC_IDX,
                                       rawr_s1_ppage_sem_pindex_post_update)
                                       
    blei        r3, RAWR_PPAGE_SIZE, desc_sem_pindex_fetch_update
    nop
    
mpage_sem_pindex_fetch_update:

    /*
     * Fetch/update memory page pindex for storing metaheader
     */
    RAWR_ALLOC_SEM_PINDEX_FETCH_UPDATE(2, r4,
                                       RAWRPR_SMALL_ALLOC_IDX,
                                       rawr_s1_mpage_sem_pindex_post_update)
desc_sem_pindex_fetch_update:

    /*
     * Fetch/update buffer descriptor pindex for queuing to chain ring
     */
    RAWR_ALLOC_SEM_PINDEX_FETCH_UPDATE(0, r4,
                                       RAWRDR_ALLOC_IDX,
                                       rawr_s1_desc_sem_pindex_post_update)
    nop.e
    nop

packet_len_err_discard:

    /*
     * Discard packet due to invalid packet length
     */
    phvwr.e     p.p4_intr_global_drop, 1
    nop    

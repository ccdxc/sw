#include "raw_redir_common.h"

struct phv_                             p;
struct rawr_chain_xfer_k                k;

%%
    .align

/*
 * Set up DMA transfer of meta header and packet to memory page(s).
 * Also point descriptor to the page(s) and enqueue it to the
 * next service queue.
 *
 * NOTE: this is a common function arrived at via a direct jump
 * as opposed to a table read.
 *
 * Register args:
 *     On entry, r1 contains the allocated chain ring pindex
 */
rawr_s6_chain_xfer:

    CAPRI_CLEAR_TABLE3_VALID
    phvwri      p.p4_rxdma_intr_dma_cmd_ptr, CAPRI_PHV_START_OFFSET(dma_pkt0_dma_cmd_type) / 16
        
    /*
     * Set up DMA of p4_to_p4plus_cpu_pkt_t header and packet to memory page.
     * They either fit entirely in one mpage, or one pppage, or both, as
     * guaranteed by rawr_s0_rx_start.
     */
    sne         c1, k.common_phv_mpage_valid, r0
    add.c1      r3, r0, k.to_s6_mpage
    addi.c1     r4, r0, RAWR_MPAGE_SIZE
    add.!c1     r3, r0, k.to_s6_ppage
    addi.!c1    r4, r0, RAWR_PPAGE_SIZE

    /*
     * if p4_to_p4plus_cpu_pkt_t header plus packet > current page size
     *   1st DMA transfer will only include p4_to_p4plus_cpu_pkt_t header
     */    
    add         r6, r0, k.common_phv_packet_len
    add         r7, r0, r6
    sle         c2, r6, r4
    addi.!c2    r7, r0, RAWR_P4PLUS_APP_HEADER_SZ
    
    phvwr       p.dma_pkt0_dma_cmd_addr, r3
    phvwr       p.dma_pkt0_dma_cmd_size, r7
    phvwri      p.dma_pkt0_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM

    /*
     * Also prep the queue descriptor AOL's starting with AOL0
     */
    phvwr       p.aol_A0, r3.dx
    phvwr       p.aol_L0, r7.wx
    
    /*
     * Handle DMA to 2nd page if applicable:
     * if copy_len < packet_len
     *    transfer rest of packet to 2nd page
     */
    slt         c3, r7, r6
    subi        r6, r6, RAWR_P4PLUS_APP_HEADER_SZ
    
    add.c3      r3, r0, k.to_s6_ppage
    phvwr.c3    p.dma_pkt1_dma_cmd_addr, r3
    phvwr.c3    p.dma_pkt1_dma_cmd_size, r6
    phvwri.c3   p.dma_pkt1_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM
    phvwri.!c3  p.dma_pkt1_dma_cmd_type, CAPRI_DMA_COMMAND_NOP

    /*
     * Depending on the number of pages required, prep AOL1
     */
    phvwr.c3    p.aol_A1, r3.dx
    phvwr.c3    p.aol_L1, r6.wx
    
    /*
     * Set up transfer of the AOL's into queue descriptor
     */    
    add         r5, r0, k.to_s6_desc
    addi        r5, r5, NIC_DESC_ENTRY_0_OFFSET     // skip scratch area
    phvwr       p.dma_desc_dma_cmd_addr, r5
    phvwri      p.dma_desc_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(aol_A0)
    phvwri      p.dma_desc_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(aol_next_pkt)
    phvwri      p.dma_desc_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    
    /*
     * Set up DMA to enqueue descriptor to next service chain
     */
    add         r5, r0, r1  // r1 = chain pindex from caller
    add         r6, r0, k.common_phv_chain_entry_size_shift
    sllv        r5, r5, r6
    add         r5, r5, k.{common_phv_chain_ring_base}.wx
    phvwr       p.dma_chain_dma_cmd_addr, r5

    /*
     * Some service chain queue may require descriptor valid bit to be set
     */
    add         r5, r0, k.common_phv_desc_valid_bit_req
    sll         r5, r5, DESC_VALID_BIT_SHIFT
    or          r5, r5, k.to_s6_desc
    
    phvwr       p.ring_entry_descr_addr, r5         // content for this PHV2MEM
    phvwri      p.dma_chain_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(ring_entry_descr_addr)
    phvwri      p.dma_chain_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(ring_entry_descr_addr)
    phvwri      p.dma_chain_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

    /*
     * If the next service ring belongs to a TxQ, set up DMA to increment pindex
     * and ring doorbell.
     */
    add         r5, r0, k.common_phv_chain_to_rxq
    sne         c1, r5, r0
    phvwri.c1   p.dma_chain_dma_cmd_eop, TRUE       // delay slot
    phvwri.c1.e p.dma_chain_dma_cmd_wr_fence, TRUE
    nop
    
    /*
     * Only get here if chaining to a TxQ:
     * Set up DMA to increment PI and ring doorbell
     * with scheduler bit set/unset as an option
     */
    add         r3, r0, k.common_phv_chain_doorbell_no_sched
    sne         c1, r3, r0
    cmov        r3, c1, DB_NO_SCHED_WR, DB_SCHED_WR_EVAL_RING
    
    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE,
                        DB_INC_PINDEX,
                        r3,
                        k.{common_phv_chain_lif_sbit0_ebit7...\
                           common_phv_chain_lif_sbit8_ebit10},
                        k.common_phv_chain_qtype,
                        r5)
    CAPRI_SETUP_DB_DATA(k.{common_phv_chain_qid_sbit0_ebit1...\
                           common_phv_chain_qid_sbit18_ebit23},
                        k.common_phv_chain_ring_index_select,
                        r0,     // current PI is actually dontcare for DB_INC_PINDEX
                        r6)
                        
    phvwr       p.chain_txq_db_data_data, r6
    phvwr       p.dma_doorbell_dma_cmd_addr, r5
    phvwri      p.dma_doorbell_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(chain_txq_db_data_data)
    phvwri      p.dma_doorbell_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(chain_txq_db_data_data)
    phvwri      p.dma_doorbell_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM;
    phvwri      p.dma_doorbell_dma_cmd_eop, TRUE
    phvwri.e    p.dma_doorbell_dma_cmd_wr_fence, TRUE
    nop
     

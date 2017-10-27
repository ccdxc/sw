#include "raw_redir_common.h"

struct phv_                             p;
struct rawr_chain_xfer_k                k;

#define CHAIN_PINDEX_R                  r1  // param from caller
#define PAGE_ADDR_R                     r3  // page address
#define PAGE_SIZE_R                     r4  // page size
#define RAWR_HDR_FLAGS_R                r5
#define PACKET_LEN_R                    r6  // current packet length
#define XFER_LEN_R                      r7  // current transfer length

/*
 * Register reuse (chain TxQ handling)
 */
#define TXQ_DB_SCHED_R                  r2
#define TXQ_DB_ADDR_R                   r3
#define TXQ_DB_DATA_R                   r4
#define DESC_R                          r5  // descriptor address
#define CHAIN_ENTRY_R                   r6  // chain ring entry
#define SCRATCH_R                       r7

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
    phvwri      p.p4_rxdma_intr_dma_cmd_ptr, CAPRI_PHV_START_OFFSET(dma_cpu_pkt_dma_cmd_type) / 16
        
    /*
     * Assume that HW would have dropped the packet if there had been
     * any L3/L4 checksum errors
     */
    addi        RAWR_HDR_FLAGS_R, r0, PEN_APP_REDIR_L3_CSUM_CHECKED + \
                                      PEN_APP_REDIR_L4_CSUM_CHECKED
    /*
     * Continue to fill out meta headers
     */
    sne         c1, k.common_phv_redir_span_instance, r0
    ori.c1      RAWR_HDR_FLAGS_R, RAWR_HDR_FLAGS_R, PEN_APP_REDIR_SPAN_INSTANCE
    sne         c1, k.common_phv_redir_pipeline_lpbk_enable, r0
    ori.c1      RAWR_HDR_FLAGS_R, RAWR_HDR_FLAGS_R, PEN_APP_REDIR_PIPELINE_LOOPBK_EN
    
    phvwri      p.pen_app_redir_hdr_h_proto, PEN_APP_REDIR_ETHERTYPE
    phvwri      p.pen_app_redir_version_hdr_format, PEN_RAW_REDIR_V1_FORMAT
    phvwri      p.pen_app_redir_version_hdr_hdr_len, PEN_APP_REDIR_VERSION_HEADER_SIZE +\
                                                     PEN_RAW_REDIR_HEADER_V1_SIZE
    /*
     * Set up DMA of meta headers (p4_to_p4plus_cpu_pkt_t + app_redir headers) 
     * and the packet content. They either fit entirely in one mpage, 
     * or one pppage, or both, as guaranteed by rawr_s0_rx_start.
     */
    sne         c1, k.common_phv_mpage_valid, r0
    add.c1      PAGE_ADDR_R, r0, k.to_s6_mpage
    addi.c1     PAGE_SIZE_R, r0, RAWR_MPAGE_SIZE
    ori.c1      RAWR_HDR_FLAGS_R, RAWR_HDR_FLAGS_R, PEN_APP_REDIR_A0_RNMPR_SMALL
    add.!c1     PAGE_ADDR_R, r0, k.to_s6_ppage
    addi.!c1    PAGE_SIZE_R, r0, RAWR_PPAGE_SIZE

    phvwr       p.pen_raw_redir_hdr_v1_flags, RAWR_HDR_FLAGS_R
                                                     
    /*
     * First, transfer the p4_to_p4plus_cpu_pkt_t header.
     * Also prep the queue descriptor AOL's starting with AOL0
     */
    phvwr       p.aol_A0, PAGE_ADDR_R.dx
    phvwr       p.dma_cpu_pkt_dma_cmd_addr, PAGE_ADDR_R
    phvwri      p.dma_cpu_pkt_dma_cmd_size, P4PLUS_CPU_PKT_SZ
    phvwri      p.dma_cpu_pkt_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM
    
    /*
     * Next, insert the meta headers that we constructed locally
     */
    addi        PAGE_ADDR_R, PAGE_ADDR_R, P4PLUS_CPU_PKT_SZ
    phvwr       p.dma_meta_dma_cmd_addr, PAGE_ADDR_R
    phvwri      p.dma_meta_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(pen_app_redir_hdr_h_dest)
    phvwri      p.dma_meta_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(pen_raw_redir_hdr_v1_end_pad)
    phvwri      p.dma_meta_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    
    addi        XFER_LEN_R, r0, P4PLUS_CPU_PKT_SZ + P4PLUS_RAW_REDIR_HDR_SZ
      
    /*
     * if total len (p4_to_p4plus_cpu_pkt_t + meta headers + packet) > current page size
     *   transfer the rest of the packet to the next page
     */    
    add         PACKET_LEN_R, r0, k.common_phv_packet_len
    phvwr       p.aol_L0, PACKET_LEN_R.wx
    sle         c2, PACKET_LEN_R, PAGE_SIZE_R
    addi.c2     PAGE_ADDR_R, PAGE_ADDR_R, P4PLUS_RAW_REDIR_HDR_SZ
    phvwr.!c2   p.aol_L0, XFER_LEN_R.wx
    add.!c2     PAGE_ADDR_R, r0, k.to_s6_ppage
    
    sub         PACKET_LEN_R, PACKET_LEN_R, XFER_LEN_R
    phvwr       p.dma_pkt_content_dma_cmd_addr, PAGE_ADDR_R
    phvwr       p.dma_pkt_content_dma_cmd_size, PACKET_LEN_R
    phvwri      p.dma_pkt_content_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM
    
    /*
     * Depending on the number of pages required, prep AOL1
     */
    phvwr.!c2   p.aol_A1, PAGE_ADDR_R.dx
    phvwr.!c2   p.aol_L1, PACKET_LEN_R.wx

    /*
     * Set up transfer of the AOL's into queue descriptor
     */    
    add         DESC_R, k.to_s6_desc, NIC_DESC_ENTRY_0_OFFSET
    phvwr       p.dma_desc_dma_cmd_addr, DESC_R
    phvwri      p.dma_desc_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(aol_A0)
    phvwri      p.dma_desc_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(aol_next_pkt)
    phvwri      p.dma_desc_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    
    /*
     * Set up DMA to enqueue descriptor to next service chain
     */
    add         CHAIN_ENTRY_R, r0, CHAIN_PINDEX_R      // chain pindex from caller
    add         SCRATCH_R, r0, k.common_phv_chain_entry_size_shift
    sllv        CHAIN_ENTRY_R, CHAIN_ENTRY_R, SCRATCH_R
    add         CHAIN_ENTRY_R, CHAIN_ENTRY_R, k.{common_phv_chain_ring_base}.wx
    phvwr       p.dma_chain_dma_cmd_addr, CHAIN_ENTRY_R

    /*
     * Some service chain queue may require descriptor valid bit to be set
     */
    add         DESC_R, k.to_s6_desc, k.common_phv_desc_valid_bit_req, DESC_VALID_BIT_SHIFT
    
    phvwr       p.ring_entry_descr_addr, DESC_R         // content for this PHV2MEM
    phvwri      p.dma_chain_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(ring_entry_descr_addr)
    phvwri      p.dma_chain_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(ring_entry_descr_addr)
    phvwri      p.dma_chain_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

    /*
     * If the next service ring belongs to a TxQ, set up DMA to increment pindex
     * and ring doorbell.
     */
    sne         c1, k.common_phv_chain_to_rxq, r0
    phvwri.c1   p.dma_chain_dma_cmd_eop, TRUE
    phvwri.c1.e p.dma_chain_dma_cmd_wr_fence, TRUE
    nop
    
    /*
     * Only get here if chaining to a TxQ:
     * Set up DMA to increment PI and ring doorbell
     * with scheduler bit set/unset as an option
     */
    sne         c1, k.common_phv_chain_doorbell_no_sched, r0
    cmov        TXQ_DB_SCHED_R, c1, DB_NO_SCHED_WR, DB_SCHED_WR_EVAL_RING
    
    RAWR_SETUP_DB_ADDR(DB_ADDR_BASE,
                       DB_INC_PINDEX,
                       TXQ_DB_SCHED_R,
                       k.{common_phv_chain_lif_sbit0_ebit7...\
                          common_phv_chain_lif_sbit8_ebit10},
                       k.common_phv_chain_qtype,
                       TXQ_DB_ADDR_R)
    RAWR_SETUP_DB_DATA(k.{common_phv_chain_qid_sbit0_ebit1...\
                          common_phv_chain_qid_sbit18_ebit23},
                       k.common_phv_chain_ring_index_select,
                       r0,      // current PI is actually dontcare for DB_INC_PINDEX
                       TXQ_DB_DATA_R)
                        
    phvwr       p.chain_txq_db_data_data, TXQ_DB_DATA_R
    phvwr       p.dma_doorbell_dma_cmd_addr, TXQ_DB_ADDR_R
    phvwri      p.dma_doorbell_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(chain_txq_db_data_data)
    phvwri      p.dma_doorbell_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(chain_txq_db_data_data)
    phvwri      p.dma_doorbell_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM;
    phvwri      p.dma_doorbell_dma_cmd_eop, TRUE
    phvwri.e    p.dma_doorbell_dma_cmd_wr_fence, TRUE
    nop


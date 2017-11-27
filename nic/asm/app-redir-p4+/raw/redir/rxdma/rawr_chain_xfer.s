#include "app_redir_common.h"

struct phv_                             p;
struct rawr_chain_xfer_k                k;

#define r_chain_pindex                  r1  // must match rawr_chain_qidx_post_read
#define r_rawrcb_flags                  r2
#define r_page_addr                     r3  // page address
#define r_page_size                     r4  // page size
#define r_rawr_hdr_flags                r5
#define r_pkt_len                       r6  // current packet length
#define r_xfer_len                      r7  // current transfer length

/*
 * Register reuse (chain TxQ handling)
 */
#define r_txq_db_sched                  r1
#define r_txq_db_addr                   r3
#define r_txq_db_data                   r4
#define r_desc                          r5  // descriptor address
#define r_chain_entry                   r6  // chain ring entry
#define r_scratch                       r7

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
    phvwri      p.p4_rxdma_intr_dma_cmd_ptr,\
                CAPRI_PHV_START_OFFSET(dma_cpu_pkt_dma_cmd_type) / 16
    /*
     * Assume that HW would have dropped the packet if there had been
     * any L3/L4 checksum errors
     */
    addi        r_rawr_hdr_flags, r0, PEN_APP_REDIR_L3_CSUM_CHECKED + \
                                      PEN_APP_REDIR_L4_CSUM_CHECKED
    /*
     * Continue to fill out meta headers
     */
    sne         c1, k.common_phv_redir_span_instance, r0
    ori.c1      r_rawr_hdr_flags, r_rawr_hdr_flags,\
                PEN_APP_REDIR_SPAN_INSTANCE
    add         r_rawrcb_flags, r0, k.common_phv_rawrcb_flags
    smeqh       c1, r_rawrcb_flags, APP_REDIR_DOL_PIPELINE_LOOPBK_EN,\
                                    APP_REDIR_DOL_PIPELINE_LOOPBK_EN
    ori.c1      r_rawr_hdr_flags, r_rawr_hdr_flags,\
                PEN_APP_REDIR_PIPELINE_LOOPBK_EN
    phvwri      p.pen_app_redir_hdr_h_proto, PEN_APP_REDIR_ETHERTYPE
    phvwri      p.pen_app_redir_version_hdr_format, PEN_RAW_REDIR_V1_FORMAT
    phvwri      p.pen_app_redir_version_hdr_hdr_len,\
                PEN_APP_REDIR_VERSION_HEADER_SIZE + \
                PEN_RAW_REDIR_HEADER_V1_SIZE
    /*
     * Set up DMA of meta headers (p4_to_p4plus_cpu_pkt_t + app_redir headers) 
     * and the packet content. They either fit entirely in one mpage, 
     * or one pppage, or both, as guaranteed by rawr_s0_rx_start.
     */
    add         r_page_addr, r0, k.{to_s6_mpage_sbit0_ebit3...\
                                    to_s6_mpage_sbit28_ebit33}
    sne         c1, r_page_addr, r0
    addi.c1     r_page_size, r0, APP_REDIR_MPAGE_SIZE
    ori.c1      r_rawr_hdr_flags, r_rawr_hdr_flags,\
                PEN_APP_REDIR_A0_RNMPR_SMALL
    add.!c1     r_page_addr, r0, k.{to_s6_ppage_sbit0_ebit5...\
                                    to_s6_ppage_sbit30_ebit33}
    addi.!c1    r_page_size, r0, APP_REDIR_PPAGE_SIZE

    phvwr       p.pen_raw_redir_hdr_v1_flags, r_rawr_hdr_flags
                                                     
    /*
     * First, transfer the p4_to_p4plus_cpu_pkt_t header.
     * Also prep the queue descriptor AOL's starting with AOL0
     */
    phvwr       p.aol_A0, r_page_addr.dx
    phvwr       p.dma_cpu_pkt_dma_cmd_addr, r_page_addr
    phvwri      p.dma_cpu_pkt_dma_cmd_size, P4PLUS_CPU_PKT_SZ
    phvwri      p.dma_cpu_pkt_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM
    
    /*
     * Next, insert the meta headers that we constructed locally
     */
    addi        r_page_addr, r_page_addr, P4PLUS_CPU_PKT_SZ
    phvwr       p.dma_meta_dma_cmd_addr, r_page_addr
    phvwri      p.dma_meta_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(pen_app_redir_hdr_h_dest)
    phvwri      p.dma_meta_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(pen_raw_redir_hdr_v1_end_pad)
    phvwri      p.dma_meta_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    
    addi        r_xfer_len, r0, P4PLUS_CPU_PKT_SZ + P4PLUS_RAW_REDIR_HDR_SZ
      
    /*
     * if total len (p4_to_p4plus_cpu_pkt_t + meta headers + packet) > current page size
     *   transfer the rest of the packet to the next page
     */    
    add         r_pkt_len, r0, k.common_phv_packet_len
    phvwr       p.aol_L0, r_pkt_len.wx
    sle         c2, r_pkt_len, r_page_size
    addi.c2     r_page_addr, r_page_addr, P4PLUS_RAW_REDIR_HDR_SZ
    phvwr.!c2   p.aol_L0, r_xfer_len.wx
    add.!c2     r_page_addr, r0, k.{to_s6_ppage_sbit0_ebit5...\
                                    to_s6_ppage_sbit30_ebit33}
    
    sub         r_pkt_len, r_pkt_len, r_xfer_len
    phvwr       p.dma_pkt_content_dma_cmd_addr, r_page_addr
    phvwr       p.dma_pkt_content_dma_cmd_size, r_pkt_len
    phvwri      p.dma_pkt_content_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM
    
    /*
     * Depending on the number of pages required, prep AOL1
     */
    phvwr.!c2   p.aol_A1, r_page_addr.dx
    phvwr.!c2   p.aol_L1, r_pkt_len.wx

    /*
     * Set up transfer of the AOL's into queue descriptor
     */    
    add         r_desc, k.{to_s6_desc_sbit0_ebit31...\
                           to_s6_desc_sbit32_ebit33},\
                NIC_DESC_ENTRY_0_OFFSET
    phvwr       p.dma_desc_dma_cmd_addr, r_desc
    phvwri      p.dma_desc_dma_cmd_phv_start_addr,\
                CAPRI_PHV_START_OFFSET(aol_A0)
    phvwri      p.dma_desc_dma_cmd_phv_end_addr,\
                CAPRI_PHV_END_OFFSET(aol_next_pkt)
    phvwri      p.dma_desc_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    
    /*
     * Set up DMA to enqueue descriptor to next service chain
     */
    add         r_chain_entry, r0, r_chain_pindex      // chain pindex from caller
    add         r_scratch, r0, k.common_phv_chain_entry_size_shift
    sllv        r_chain_entry, r_chain_entry, r_scratch
    add         r_chain_entry, r_chain_entry, k.{common_phv_chain_ring_base}.wx
    phvwr       p.dma_chain_dma_cmd_addr, r_chain_entry

    /*
     * Some service chain queue may require descriptor valid bit to be set
     */
    smeqh       c1, r_rawrcb_flags, APP_REDIR_DESC_VALID_BIT_REQ,  \
                                    APP_REDIR_DESC_VALID_BIT_REQ
    add.c1      r_desc, k.{to_s6_desc_sbit0_ebit31...\
                           to_s6_desc_sbit32_ebit33},\
                1, DESC_VALID_BIT_SHIFT
    add.!c1     r_desc, k.{to_s6_desc_sbit0_ebit31...\
                           to_s6_desc_sbit32_ebit33}, r0
    
    phvwr       p.ring_entry_descr_addr, r_desc // content for this PHV2MEM
    phvwri      p.dma_chain_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(ring_entry_descr_addr)
    phvwri      p.dma_chain_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(ring_entry_descr_addr)
    phvwri      p.dma_chain_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

    /*
     * If the next service ring belongs to a TxQ, set up DMA to increment
     * pindex and ring doorbell.
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
    smeqh       c1, r_rawrcb_flags, APP_REDIR_CHAIN_DOORBELL_NO_SCHED,\
                                    APP_REDIR_CHAIN_DOORBELL_NO_SCHED
    cmov        r_txq_db_sched, c1, DB_NO_SCHED_WR,\
                DB_SCHED_WR_EVAL_RING
    
    APP_REDIR_SETUP_DB_ADDR(DB_ADDR_BASE,
                            DB_INC_PINDEX,
                            r_txq_db_sched,
                            k.{common_phv_chain_lif_sbit0_ebit7...\
                               common_phv_chain_lif_sbit8_ebit10},
                            k.common_phv_chain_qtype,
                            r_txq_db_addr)
    APP_REDIR_SETUP_DB_DATA(k.{common_phv_chain_qid_sbit0_ebit1...\
                               common_phv_chain_qid_sbit18_ebit23},
                            k.common_phv_chain_ring_index_select,
                            r0, // curr PI is dontcare for DB_INC_PINDEX
                            r_txq_db_data)
                        
    phvwr       p.chain_txq_db_data_data, r_txq_db_data
    phvwr       p.dma_doorbell_dma_cmd_addr, r_txq_db_addr
    phvwri      p.dma_doorbell_dma_cmd_phv_start_addr,\
                CAPRI_PHV_START_OFFSET(chain_txq_db_data_data)
    phvwri      p.dma_doorbell_dma_cmd_phv_end_addr,\
                CAPRI_PHV_END_OFFSET(chain_txq_db_data_data)
    phvwri      p.dma_doorbell_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM;
    phvwri      p.dma_doorbell_dma_cmd_eop, TRUE
    phvwri.e    p.dma_doorbell_dma_cmd_wr_fence, TRUE
    nop


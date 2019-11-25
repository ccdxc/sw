#include "app_redir_common.h"

struct phv_             p;
struct s5_tbl1_k        k;

#define r_chain_pindex                  r1  // must match rawr_chain_sem_pindex_post_update
#define r_rawrcb_flags                  r2
#define r_page_addr                     r3  // page address
#define r_desc                          r4  // descriptor address
#define r_rawr_hdr_flags                r5
#define r_pkt_len                       r6  // current packet length

/*
 * Register reuse (chain TxQ handling)
 */
#define r_txq_db_sched                  r1
#define r_txq_db_addr                   r3
#define r_txq_db_data                   r4
#define r_chain_entry                   r6  // chain ring entry
#define r_scratch                       r7

%%
    .param      rawr_normal_stats_inc
    
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
rawr_chain_xfer:

    CAPRI_CLEAR_TABLE1_VALID
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
    sne         c1, RAWR_KIVEC0_REDIR_SPAN_INSTANCE, r0
    ori.c1      r_rawr_hdr_flags, r_rawr_hdr_flags,\
                PEN_APP_REDIR_SPAN_INSTANCE
    add         r_rawrcb_flags, r0, RAWR_KIVEC0_RAWRCB_FLAGS
    smeqh       c2, r_rawrcb_flags, APP_REDIR_DOL_PIPELINE_LOOPBK_EN,\
                                    APP_REDIR_DOL_PIPELINE_LOOPBK_EN
    ori.c2      r_rawr_hdr_flags, r_rawr_hdr_flags,\
                PEN_APP_REDIR_PIPELINE_LOOPBK_EN
    phvwri      p.pen_app_redir_hdr_h_proto, PEN_APP_REDIR_ETHERTYPE
    phvwrpair   p.pen_app_redir_version_hdr_hdr_len,\
                PEN_APP_REDIR_VERSION_HEADER_SIZE + PEN_RAW_REDIR_HEADER_V1_SIZE, \
                p.pen_app_redir_version_hdr_format, PEN_RAW_REDIR_V1_FORMAT
    phvwr       p.pen_raw_redir_hdr_v1_flags, r_rawr_hdr_flags
                
    /*
     * Set up DMA of meta headers (p4_to_p4plus_cpu_pkt_t + app_redir headers) 
     * and the packet content. They are guaranteed by rawr_rx_start to fit
     * entirely in one pppage.
     */
    add         r_desc, r0, RAWR_KIVEC0_PPAGE
    add         r_page_addr, r_desc, RAWR_RNMDPR_PAGE_OFFSET
                                                     
    /*
     * First, transfer the p4_to_p4plus_cpu_pkt_t header.
     * Also prep the queue descriptor AOL's starting with AOL0
     */
    phvwr       p.aol_A0, r_page_addr.dx
    phvwr       p.dma_cpu_pkt_dma_cmd_addr, r_page_addr
    phvwrpair   p.dma_cpu_pkt_dma_cmd_size, P4PLUS_CPU_PKT_SZ,\
                p.dma_cpu_pkt_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM
    
    /*
     * Next, insert the meta headers that we constructed locally
     */
    addi        r_page_addr, r_page_addr, P4PLUS_CPU_PKT_SZ
    phvwrpair   p.dma_meta_dma_cmd_addr, r_page_addr,\
                p.dma_meta_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwrpair   p.dma_meta_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(pen_raw_redir_hdr_v1_end_pad), \
                p.dma_meta_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(pen_app_redir_hdr_h_dest)
    
    add         r_pkt_len, r0, RAWR_KIVEC0_PACKET_LEN
    phvwr       p.aol_L0, r_pkt_len.wx
    addi        r_page_addr, r_page_addr, P4PLUS_RAW_REDIR_HDR_SZ
    
    sub         r_pkt_len, r_pkt_len, P4PLUS_CPU_PKT_SZ + P4PLUS_RAW_REDIR_HDR_SZ
    phvwr       p.dma_pkt_content_dma_cmd_addr, r_page_addr
    phvwrpair   p.dma_pkt_content_dma_cmd_size, r_pkt_len,\
                p.dma_pkt_content_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM
    
    /*
     * Set up transfer of the AOL's into queue descriptor
     */    
    add         r_scratch, r_desc, NIC_DESC_ENTRY_0_OFFSET
    phvwrpair   p.dma_desc_dma_cmd_addr, r_scratch,\
                p.dma_desc_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr       p.dma_desc_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(aol_next_pkt)
    phvwr       p.dma_desc_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(aol_A0)
    
    /*
     * Set up DMA to enqueue descriptor to next service chain
     */
    add         r_chain_entry, r_chain_pindex, r0
    mincr       r_chain_entry, RAWR_KIVEC0_CHAIN_RING_SIZE_SHIFT, r0
    sllv        r_chain_entry, r_chain_entry, \
                RAWR_KIVEC2_CHAIN_ENTRY_SIZE_SHIFT
    add         r_chain_entry, r_chain_entry, RAWR_KIVEC2_CHAIN_RING_BASE
    
    /*
     * Service chain's queue may be expecting to get a desc that has already
     * been adjusted to point to the beginning of the AOL area.
     */
    smeqh       c4, r_rawrcb_flags, APP_REDIR_CHAIN_DESC_ADD_AOL_OFFSET,\
                                    APP_REDIR_CHAIN_DESC_ADD_AOL_OFFSET
    add.c4      r_desc, r_desc, NIC_DESC_ENTRY_0_OFFSET

    /*
     * The same queue may also require descriptor valid bit to be set
     */
    smeqh       c5, r_rawrcb_flags, APP_REDIR_DESC_VALID_BIT_REQ,  \
                                    APP_REDIR_DESC_VALID_BIT_REQ
    srl         r_scratch, r_chain_pindex, RAWR_KIVEC0_CHAIN_RING_SIZE_SHIFT
    add.c5      r_desc, r_desc, r_scratch[0], DESC_VALID_BIT_SHIFT
    phvwr       p.ring_entry_descr_addr, r_desc // content for this PHV2MEM
    phvwrpair   p.dma_chain_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(ring_entry_descr_addr), \
                p.dma_chain_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(ring_entry_descr_addr)
                
    phvwrpair   p.dma_chain_dma_cmd_addr, r_chain_entry,\
                p.dma_chain_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

    /*
     * Gather packet redirect statistics
     */
    RAWR_METRICS_SET(redir_pkts)

    /*
     * If the next service ring belongs to a TxQ, set up DMA to increment
     * pindex and ring doorbell.
     */
    sne         c6, RAWR_KIVEC0_CHAIN_TO_RXQ, r0
    phvwrpair.c6.e p.dma_chain_dma_cmd_wr_fence, TRUE, \
                   p.dma_chain_dma_cmd_eop, TRUE
    
    /*
     * Only get here if chaining to a TxQ:
     * Set up DMA to increment PI and ring doorbell
     * with scheduler bit set/unset as an option
     */
    smeqh       c7, r_rawrcb_flags, APP_REDIR_CHAIN_DOORBELL_NO_SCHED,\
                                    APP_REDIR_CHAIN_DOORBELL_NO_SCHED // delay slot
    cmov        r_txq_db_sched, c7, DB_NO_SCHED_WR,\
                DB_SCHED_WR_EVAL_RING
    
    APP_REDIR_SETUP_DB_ADDR(DB_ADDR_BASE,
                            DB_INC_PINDEX,
                            r_txq_db_sched,
                            RAWR_KIVEC2_CHAIN_LIF,
                            RAWR_KIVEC2_CHAIN_QTYPE,
                            r_txq_db_addr)
    APP_REDIR_SETUP_DB_DATA(RAWR_KIVEC2_CHAIN_QID,
                            RAWR_KIVEC0_CHAIN_RING_INDEX_SELECT,
                            r0, // curr PI is dontcare for DB_INC_PINDEX
                            r_txq_db_data)
                        
    phvwr       p.chain_txq_db_data_data, r_txq_db_data.dx
    phvwrpair   p.dma_doorbell_dma_cmd_addr, r_txq_db_addr, \
                p.dma_doorbell_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwrpair.e p.dma_doorbell_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(chain_txq_db_data_data), \
                p.dma_doorbell_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(chain_txq_db_data_data)
    phvwrpair   p.dma_doorbell_dma_cmd_wr_fence, TRUE, \
                p.dma_doorbell_dma_cmd_eop, TRUE    // delay slot


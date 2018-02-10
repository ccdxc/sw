#include "app_redir_common.h"

struct phv_                             p;
struct proxyr_chain_xfer_k              k;
struct proxyr_chain_xfer_chain_xfer_d   d;

/*
 * Registers usage
 */
#define r_chain_pindex                  r1  // param from caller
#define r_proxyrcb_flags                r2
#define r_page_addr                     r3  // page address
#define r_proxyr_hdr_flags              r4
#define r_desc                          r5  // descriptor address
#define r_chain_entry                   r6  // chain ring entry
#define r_scratch                       r7

%%
    .param      proxyr_normal_stats_inc
    
    .align

/*
 * Next service chain queue index table post read handling.
 * Upon entry in this stage, the table has been locked
 * allowing for atomic read-update.
 *
 * After ARQ semaphore read, set up DMA transfer of meta header to memory page.
 * Also point descriptor to the page(s) and enqueue it to the
 * next service queue.
 */
proxyr_s5_chain_xfer:

    CAPRI_CLEAR_TABLE0_VALID
        
    /*
     * TODO: check for queue full
     */    
    add         r_chain_pindex, r0, d.{arq_pindex}.wx
    mincr       r_chain_pindex, k.{common_phv_chain_ring_size_shift}, r0

    /*
     * Assume that HW would have dropped the packet if there had been
     * any L3/L4 checksum errors
     */
    phvwri      p.p4_txdma_intr_dma_cmd_ptr, \
                CAPRI_PHV_START_OFFSET(dma_meta_dma_cmd_type) / 16
    addi        r_proxyr_hdr_flags, r0, PEN_APP_REDIR_L3_CSUM_CHECKED + \
                                        PEN_APP_REDIR_L4_CSUM_CHECKED
    /*
     * App redirect proxy packet will take a different path in FTE where
     * none of the L2/L3/L4 headers will be read. These offsets are set
     * to invalid to catch any FTE read attempts.
     */
    phvwri      p.{p4plus_cpu_pkt_l2_offset...p4plus_cpu_pkt_payload_offset}, -1
     
    /*
     * Continue to fill out meta headers
     */
    sne         c1, k.common_phv_redir_span_instance, r0
    ori.c1      r_proxyr_hdr_flags, r_proxyr_hdr_flags, PEN_APP_REDIR_SPAN_INSTANCE
    add         r_proxyrcb_flags, r0, k.{common_phv_proxyrcb_flags_sbit0_ebit5...\
                                         common_phv_proxyrcb_flags_sbit14_ebit15}
    smeqh       c1,  r_proxyrcb_flags, APP_REDIR_DOL_PIPELINE_LOOPBK_EN, \
                                       APP_REDIR_DOL_PIPELINE_LOOPBK_EN
    ori.c1      r_proxyr_hdr_flags, r_proxyr_hdr_flags, PEN_APP_REDIR_PIPELINE_LOOPBK_EN
    
    phvwri      p.pen_app_redir_hdr_h_proto, PEN_APP_REDIR_ETHERTYPE
    phvwrpair   p.pen_app_redir_version_hdr_hdr_len, \
                PEN_APP_REDIR_VERSION_HEADER_SIZE + PEN_PROXY_REDIR_HEADER_V1_SIZE, \
                p.pen_app_redir_version_hdr_format, PEN_PROXY_REDIR_V1_FORMAT
    /*
     * Set up DMA of meta headers:
     * Because p4plus_cpu_pkt + pen_app_redir_hdr + pen_app_redir_version_hdr +
     * pen_proxyr_hdr_v1 would cause pen_proxyr_hdr_v1 to be split across 2 flits
     * with some intervening bits. A pad had been inserted to push L7 headers
     * to a completely new flit. Hence, 2 DMA commands are needed to transfer
     * all the headers.
     */
    add         r_page_addr, r0, k.{to_s5_mpage_sbit0_ebit3...\
                                    to_s5_mpage_sbit28_ebit33}
    addi        r_scratch, r0, P4PLUS_CPU_PKT_SZ + \
                               PEN_APP_REDIR_HEADER_SIZE + \
                               PEN_APP_REDIR_VERSION_HEADER_SIZE + \
                               PEN_PROXY_REDIR_HEADER_V1_SIZE
    phvwrpair   p.aol_A0, r_page_addr.dx, p.aol_L0, r_scratch.wx
    ori         r_proxyr_hdr_flags, r_proxyr_hdr_flags, PEN_APP_REDIR_A0_RNMPR_SMALL
    phvwr       p.pen_proxyr_hdr_v1_flags, r_proxyr_hdr_flags
    
    phvwrpair   p.dma_meta_dma_cmd_addr, r_page_addr, \
                p.dma_meta_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_meta_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(pen_app_redir_version_hdr_end_pad)
    phvwri      p.dma_meta_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(p4plus_cpu_pkt_src_lif)

    /*
     * Second flit
     */    
    addi        r_page_addr, r_page_addr, P4PLUS_CPU_PKT_SZ + \
                                          PEN_APP_REDIR_HEADER_SIZE + \
                                          PEN_APP_REDIR_VERSION_HEADER_SIZE
    phvwrpair   p.dma_proxyr_hdr_dma_cmd_addr, r_page_addr, \
                p.dma_proxyr_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_proxyr_hdr_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(pen_proxyr_hdr_v1_end_pad)
    phvwri      p.dma_proxyr_hdr_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(pen_proxyr_hdr_v1_flow_id)
    
    /*
     * Set up transfer of the AOL's into queue descriptor
     */    
    add         r_desc, k.{to_s5_desc_sbit0_ebit31...\
                           to_s5_desc_sbit32_ebit33},\
                NIC_DESC_ENTRY_0_OFFSET
    phvwrpair   p.dma_desc_dma_cmd_addr, r_desc, \
                p.dma_desc_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_desc_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(aol_A0)
    phvwri      p.dma_desc_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(aol_next_pkt)
    
    /*
     * Set up DMA to enqueue descriptor to next service chain
     */
    sllv        r_chain_entry, r_chain_pindex, \
                k.{common_phv_chain_entry_size_shift_sbit0_ebit0...\
                   common_phv_chain_entry_size_shift_sbit1_ebit4}
    add         r_chain_entry, r_chain_entry, \
                k.{common_phv_chain_ring_base_sbit0_ebit31...\
                   common_phv_chain_ring_base_sbit32_ebit33}
    /*
     * Some service chain queue may require descriptor valid bit to be set
     */
    smeqh       c1,  r_proxyrcb_flags, APP_REDIR_DESC_VALID_BIT_REQ, \
                                       APP_REDIR_DESC_VALID_BIT_REQ
    add.c1      r_desc, k.{to_s5_desc_sbit0_ebit31...\
                           to_s5_desc_sbit32_ebit33},\
                1, DESC_VALID_BIT_SHIFT
    add.!c1     r_desc, k.{to_s5_desc_sbit0_ebit31...\
                           to_s5_desc_sbit32_ebit33}, r0
    /*
     * Gather packet redirect statistics
     */
    PROXYRCB_NORMAL_STAT_INC_LAUNCH(3, r_scratch, 
                                    k.{common_phv_qstate_addr_sbit0_ebit5...\
                                       common_phv_qstate_addr_sbit30_ebit33},
                                    p.t3_s2s_inc_stat_pkts_redir)

    phvwrpair   p.dma_chain_dma_cmd_addr, r_chain_entry, \
                p.dma_chain_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr       p.ring_entry_descr_addr, r_desc // content for this PHV2MEM
    phvwrpair.e p.dma_chain_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(ring_entry_descr_addr), \
                p.dma_chain_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(ring_entry_descr_addr)
    phvwrpair   p.dma_chain_dma_cmd_wr_fence, TRUE, \
                p.dma_chain_dma_cmd_eop, TRUE   // delay slot
    

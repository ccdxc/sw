#include "app_redir_common.h"

struct phv_                             p;
struct proxyr_chain_xfer_k              k;
struct proxyr_chain_xfer_chain_xfer_d   d;

#define CHAIN_PINDEX_R                  r1  // param from caller
#define PAGE_ADDR_R                     r2  // page address
#define PROXYR_HDR_FLAGS_R              r3
#define DESC_R                          r4  // descriptor address
#define CHAIN_ENTRY_R                   r5  // chain ring entry
#define SCRATCH_R                       r6

%%
    .align

/*
 * Next service chain queue index table post read handling.
 * Upon entry in this stage, the table has been locked
 * allowing for atomic read-update.
 *
 * After QIDXR read, set up DMA transfer of meta header to memory page.
 * Also point descriptor to the page(s) and enqueue it to the
 * next service queue.
 */
proxyr_s6_chain_xfer:

    CAPRI_CLEAR_TABLE1_VALID
        
    /*
     * TODO: check for queue full
     */    
    /*
     * Evaluate which per-core queue applies
     */
    add         SCRATCH_R, r0, k.common_phv_chain_ring_index_select
    seq         c1, SCRATCH_R, CHAIN_QIDXR_PI_0
    seq         c2, SCRATCH_R, CHAIN_QIDXR_PI_1
    seq         c3, SCRATCH_R, CHAIN_QIDXR_PI_2

    /*
     * Pass the obtained pindex to a common DMA transfer function via CHAIN_PINDEX_R
     */
    add         CHAIN_PINDEX_R, r0, d.pi_0
    tbladd.c1   d.pi_0, 1
    add.c2      CHAIN_PINDEX_R, r0, d.pi_1
    tbladd.c2   d.pi_1, 1
    add.c3      CHAIN_PINDEX_R, r0, d.pi_2
    tbladd.c3   d.pi_2, 1
    mincr       CHAIN_PINDEX_R, k.{common_phv_chain_ring_size_shift}, r0
    
    /*
     * Assume that HW would have dropped the packet if there had been
     * any L3/L4 checksum errors
     */
    phvwri      p.p4_txdma_intr_dma_cmd_ptr, CAPRI_PHV_START_OFFSET(dma_meta_dma_cmd_type) / 16
    addi        PROXYR_HDR_FLAGS_R, r0, PEN_APP_REDIR_L3_CSUM_CHECKED + \
                                        PEN_APP_REDIR_L4_CSUM_CHECKED
    /*
     * App redirect proxy packet will take a different path in FTE where
     * none of the L2/L3/L4 headers will be read. These offsets are set
     * to invalid to catch any FTE read attempts.
     */
    phvwri      p.p4plus_cpu_pkt_l2_offset, 0xffff
    phvwri      p.p4plus_cpu_pkt_l3_offset, 0xffff
    phvwri      p.p4plus_cpu_pkt_l4_offset, 0xffff
    phvwri      p.p4plus_cpu_pkt_payload_offset, 0xffff
     
    /*
     * Continue to fill out meta headers
     */
    sne         c1, k.common_phv_redir_span_instance, r0
    ori.c1      PROXYR_HDR_FLAGS_R, PROXYR_HDR_FLAGS_R, PEN_APP_REDIR_SPAN_INSTANCE
    smeqb       c1,  k.common_phv_dol_flags, APP_REDIR_DOL_PIPELINE_LOOPBK_EN, \
                                             APP_REDIR_DOL_PIPELINE_LOOPBK_EN
    ori.c1      PROXYR_HDR_FLAGS_R, PROXYR_HDR_FLAGS_R, PEN_APP_REDIR_PIPELINE_LOOPBK_EN
    
    phvwri      p.pen_app_redir_hdr_h_proto, PEN_APP_REDIR_ETHERTYPE
    phvwri      p.pen_app_redir_version_hdr_format, PEN_PROXY_REDIR_V1_FORMAT
    phvwri      p.pen_app_redir_version_hdr_hdr_len, PEN_APP_REDIR_VERSION_HEADER_SIZE +\
                                                     PEN_PROXY_REDIR_HEADER_V1_SIZE
    phvwr       p.pen_proxyr_hdr_v1_flags, PROXYR_HDR_FLAGS_R
    
    /*
     * Set up DMA of meta headers:
     * Because p4plus_cpu_pkt + pen_app_redir_hdr + pen_app_redir_version_hdr +
     * pen_proxyr_hdr_v1 would cause pen_proxyr_hdr_v1 to be split across 2 flits
     * with some intervening bits. A pad had been inserted to push pen_proxyr_hdr_v1
     * to a completely new flit. Hence, 2 DMA commands are needed to trp4plus_cpu_pkt_src_lifansfer
     * all the headers.
     */
    add         PAGE_ADDR_R, r0, k.to_s6_mpage
    phvwr       p.aol_A0, PAGE_ADDR_R.dx
    phvwr       p.dma_meta_dma_cmd_addr, PAGE_ADDR_R
    phvwri      p.dma_meta_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(p4plus_cpu_pkt_src_lif)
    phvwri      p.dma_meta_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(pen_app_redir_version_hdr_end_pad)
    phvwri      p.dma_meta_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

    /*
     * Second flit
     */    
    addi        PAGE_ADDR_R, PAGE_ADDR_R, P4PLUS_CPU_PKT_SZ + \
                                          PEN_APP_REDIR_HEADER_SIZE + \
                                          PEN_APP_REDIR_VERSION_HEADER_SIZE
    phvwr       p.dma_proxyr_hdr_dma_cmd_addr, PAGE_ADDR_R
    phvwri      p.dma_proxyr_hdr_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(pen_proxyr_hdr_v1_flow_id)
    phvwri      p.dma_proxyr_hdr_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(pen_proxyr_hdr_v1_end_pad)
    phvwri      p.dma_proxyr_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    
    addi        SCRATCH_R, r0, P4PLUS_CPU_PKT_SZ + \
                               PEN_APP_REDIR_HEADER_SIZE + \
                               PEN_APP_REDIR_VERSION_HEADER_SIZE + \
                               PEN_PROXY_REDIR_HEADER_V1_SIZE
    phvwr       p.aol_L0, SCRATCH_R.wx
    
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
    add         CHAIN_ENTRY_R, r0, CHAIN_PINDEX_R
    add         SCRATCH_R, r0, k.{common_phv_chain_entry_size_shift_sbit0_ebit2...\
                                  common_phv_chain_entry_size_shift_sbit3_ebit4}
    sllv        CHAIN_ENTRY_R, CHAIN_ENTRY_R, SCRATCH_R
    add         CHAIN_ENTRY_R, CHAIN_ENTRY_R, k.{common_phv_chain_ring_base}.wx
    phvwr       p.dma_chain_dma_cmd_addr, CHAIN_ENTRY_R

    /*
     * Some service chain queue may require descriptor valid bit to be set
     */
    smeqb       c1,  k.common_phv_dol_flags, APP_REDIR_DOL_DESC_VALID_BIT_REQ, \
                                             APP_REDIR_DOL_DESC_VALID_BIT_REQ
    add.c1      DESC_R, k.to_s6_desc, 1, DESC_VALID_BIT_SHIFT
    
    phvwr       p.ring_entry_descr_addr, DESC_R         // content for this PHV2MEM
    phvwri      p.dma_chain_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(ring_entry_descr_addr)
    phvwri      p.dma_chain_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(ring_entry_descr_addr)
    phvwri      p.dma_chain_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

    phvwri      p.dma_chain_dma_cmd_eop, TRUE
    phvwri.e    p.dma_chain_dma_cmd_wr_fence, TRUE
    nop
    

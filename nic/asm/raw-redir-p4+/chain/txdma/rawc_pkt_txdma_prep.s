#include "raw_redir_common.h"

struct phv_                             p;
struct rawc_desc_aol_k                  k;
struct rawc_desc_aol_pkt_txdma_prep_d   d;

/*
 * Registers for phvwrp accesses to DMA_MEM2PKT descriptors
 */
#define NUM_DMA_R                   r1  // # of DMA descriptors written
#define DMA_BASE_R                  r2  // DMA descriptor base offset
#define ADDR_R                      r3  // DMA page address
#define SIZE_R                      r4  // DMA page size
#define SKIP_R                      r5  // adjustment to ADDR_R/SIZE_R
#define RETURN_R                    r6  // subroutine return address
#define CPU_HEADER_ADDR_R           r7  // page address of cpu_to_p4plus_header_t

#define BIT_OFFS_PHV(phv_field)         \
    offsetof(p, phv_field)

#define BIT_OFFS_DMA_MEM2PKT(field)     \
    offsetof(struct capri_dma_cmd_mem2pkt_t, field)
    
#define BIT_SIZE_DMA_MEM2PKT(field)     \
    sizeof(struct capri_dma_cmd_mem2pkt_t.field)

%%

    .param      rawc_s3_pkt_txdma_post
    .align
    
/*
 * Evaluate AOLs contained in the incoming packet descriptor and prep
 * DMA commands to inject the packet. During DMA commands setup,
 * strip certain meta headers as necessary.
 *
 * Note1: If a next service chain TxQ had been configured, none of the
 * DMA commands here will actually execute (see rawc_s3_pkt_txdma_post).
 * However, AOLs evaluation is always done in case freeing of the pages
 * becomes necessary.
 *
 * Note2: the final step of TxDMA, if applicable, requires restoring 
 * the src_lif in the cap_phv_intr_global_t for the DMA. This value will
 * be read from a meta header and passed to the next stage.
 */
rawc_s2_pkt_txdma_prep:

    CAPRI_CLEAR_TABLE0_VALID
    
    /*
     * Make some assumptions to simplify implementation:
     * Meta headers and packet data are described within the first 3 AOLs,
     * i.e., next_addr and next_pkt are assumed NULL.
     *
     * An AOL can be of zero length, but if non-zero, it is expected
     * to contain at least a full cpu_to_p4plus_header_t
     */
    add         NUM_DMA_R, r0, r0
    add         DMA_BASE_R, r0, BIT_OFFS_PHV(dma_pkt0_dma_cmd_type)
    addi        SKIP_R, r0, CPU_TO_P4PLUS_HEADER_SIZE
    add         ADDR_R, r0, d.{A0}.dx
    add         ADDR_R, ADDR_R, d.{O0}.wx
    add         SIZE_R, r0, d.{L0}.wx
    sne         c1, SIZE_R, r0
    phvwr.c1    p.s2_t0_s2s_aol_A0, d.A0
    bal.c1      RETURN_R, dma_mem2pkt_prep
    phvwri.c1   p.s2_t0_s2s_aol_A0_valid, TRUE  // delay slot

    bal         RETURN_R, next_dma_base_get
    add         ADDR_R, r0, d.{A1}.dx           // delay slot
    add         ADDR_R, ADDR_R, d.{O1}.wx
    add         SIZE_R, r0, d.{L1}.wx
    sne         c1, SIZE_R, r0
    phvwr.c1    p.s2_t0_s2s_aol_A1, d.A1
    bal.c1      RETURN_R, dma_mem2pkt_prep
    phvwri.c1   p.s2_t0_s2s_aol_A1_valid, TRUE  // delay slot

    bal         RETURN_R, next_dma_base_get
    add         ADDR_R, r0, d.{A2}.dx           // delay slot
    add         ADDR_R, ADDR_R, d.{O2}.wx
    add         SIZE_R, r0, d.{L2}.wx
    sne         c1, SIZE_R, r0
    phvwr.c1    p.s2_t0_s2s_aol_A2, d.A2
    bal.c1      RETURN_R, dma_mem2pkt_prep
    phvwri.c1   p.s2_t0_s2s_aol_A2_valid, TRUE  // delay slot

    /*
     * Ensure at least one DMA descriptor was written
     */
    beq         NUM_DMA_R, r0, aol_error
    subi        NUM_DMA_R, NUM_DMA_R, 1         // delay slot
    bal         RETURN_R, next_dma_base_get
    nop
            
    phvwrpi     DMA_BASE_R, BIT_OFFS_DMA_MEM2PKT(pkteop),       \
                BIT_SIZE_DMA_MEM2PKT(pkteop), TRUE;
                
    phvwrpi     DMA_BASE_R, BIT_OFFS_DMA_MEM2PKT(cmdeop),       \
                BIT_SIZE_DMA_MEM2PKT(cmdeop), TRUE;
    /*
     * Launch read of cpu_to_p4plus_header_t to obtain src_lif info plus
     * page freeing instruction. The header address was calculated
     * by prep_dma_mem2pkt.
     */
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                          rawc_s3_pkt_txdma_post,
                          CPU_HEADER_ADDR_R,
                          TABLE_SIZE_32_BITS)
    nop.e
    nop

next_dma_base_get:

    /*
     * Layout of DMA descriptors in PHV is not straight forward, in that
     * dma0's bit offset is usually less than dma1's offset, except when
     * they span 2 different flits. So we can't rely on add/sub but
     * must manually calculate the new DMA_BASE_R here.
     */
    add         DMA_BASE_R, r0, BIT_OFFS_PHV(dma_pkt0_dma_cmd_type)
    seq         c1, NUM_DMA_R, 1
    add.c1      DMA_BASE_R, r0, BIT_OFFS_PHV(dma_pkt1_dma_cmd_type)
    seq         c1, NUM_DMA_R, 2
    jr          RETURN_R
    add.c1      DMA_BASE_R, r0, BIT_OFFS_PHV(dma_pkt2_dma_cmd_type) // delay slot
    
    
dma_mem2pkt_prep:

    /*
     * On entry, SIZE_R contains a non-zero AOL length (guaranteed
     * by the caller).
     *
     * This function returns:
     *    SKIP_R: reset to zero if cpu_to_p4plus_header_t was skipped
     *    DMA_INDX_R: incremented if current DMA_BASE_R was programmed.
     *
     * SKIP_R may equal 0 or sizeof(cpu_to_p4plus_header_t);
     * Note: when SIZE_R is > 0, it must also be at least >= SKIP_R
     */
    beq         SIZE_R, r0, aol_error   // enforce caller's guarantee
    slt         c2, SIZE_R, SKIP_R      // delay slot, C2 used again later below
    bcf         [c2], aol_error
    sne         c4, SKIP_R, r0          // delay slot

    /*
     * Calculate and return the page address of cpu_to_p4plus_header_t
     * It should be the page address of the first AOL that has
     * non-zero length (when SKIP_R > 0).
     */
    add.c4      CPU_HEADER_ADDR_R, r0, ADDR_R

    /*
     * Will adjust ADDR_R/SIZE_R by the amount indicated in SKIP_R
     * and set up the DMA descriptor. Ensure that SIZE_R > SKIP_R,
     * Otherwise, no need to set up the current descriptor (i.e.,
     * when SIZE_R == SKIP_R, the entire descriptor will be skipped)
     */    
    sle         c3, SIZE_R, SKIP_R
    add.!c3     ADDR_R, ADDR_R, SKIP_R
    sub.!c3     SIZE_R, SIZE_R, SKIP_R
    phvwrpi.!c3 DMA_BASE_R, BIT_OFFS_DMA_MEM2PKT(cmdtype),  \
                BIT_SIZE_DMA_MEM2PKT(cmdtype), CAPRI_DMA_COMMAND_MEM_TO_PKT;
                
    phvwrp.!c3  DMA_BASE_R, BIT_OFFS_DMA_MEM2PKT(addr),     \
                BIT_SIZE_DMA_MEM2PKT(addr), ADDR_R;
                
    phvwrp.!c3  DMA_BASE_R, BIT_OFFS_DMA_MEM2PKT(size),     \
                BIT_SIZE_DMA_MEM2PKT(size), SIZE_R;
    /*
     * Update NUM_DMA_R and SKIP_R on return:
     *    Only need to skip sizeof(cpu_to_p4plus_header_t) once so the
     *    next skip would be zero if we had actually done it.
     */
    addi.!c3    NUM_DMA_R, NUM_DMA_R, 1
    jr          RETURN_R
    add.!c2     SKIP_R, r0, r0          // delay slot

aol_error:

    /*
     * Either none of the AOLs contained any data, or cpu_to_p4plus_header_t
     * spanned across multiple AOLs!
     */
    RAWR_TXDMA_INVALID_AOL_TRAP()
    nop.e
    nop

    

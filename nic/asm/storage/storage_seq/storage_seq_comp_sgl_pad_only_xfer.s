/*****************************************************************************
 *  seq_comp_sgl_pad_only_xfer: on entry, d points to the last Barco SGL descriptor
 *                         where the comp engine has written the last chunk of
 *                         output compressed data (to d.addr0). Here, we'll 
 *                         calculate how much it has written and transfer the
 *                         remaining buffer with pad data.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s3_tbl2_k k;
struct s3_tbl2_seq_comp_sgl_pad_only_xfer_d d;
struct phv_ p;

/*
 * Registers usage
 * Note: avoid r1 as it is implicitly used by DMA_MEM2MEM_SETUP() 
 */
#define r_pad_len                   r3  // padding length
#define r_pad_dest                  r4  // pointer to destination to apply padding

%%

storage_seq_comp_sgl_pad_only_xfer:

    // dst_addr to apply padding = d.addr0 + last_blk_len
    
    add         r_pad_len, SEQ_KIVEC3_PAD_LEN, r0
    beq         r_pad_len, r0, exit
    add         r_pad_dest, d.addr0, SEQ_KIVEC3_LAST_BLK_LEN    // delay slot
    
    DMA_MEM2MEM_NO_LIF_SETUP(CAPRI_DMA_M2M_TYPE_SRC, SEQ_KIVEC5_PAD_BUF_ADDR,
                             r_pad_len, dma_m2m_6)
    DMA_MEM2MEM_NO_LIF_SETUP_REG_ADDR(CAPRI_DMA_M2M_TYPE_DST, r_pad_dest,
                                      r_pad_len, dma_m2m_7)

exit:
    CLEAR_TABLE2_e


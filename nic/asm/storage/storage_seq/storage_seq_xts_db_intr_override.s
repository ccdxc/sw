/*****************************************************************************
 *  seq_xts_db_intr_override: override a next_db ring with an interrupt generation.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s4_tbl1_k k;
struct phv_ p;

/*
 * Registers usage:
 */
#define r_stage                     r3
#define r_src_qaddr                 r4  // for SEQ_METRICS_TABLE_COMMIT

%%
    SEQ_METRICS_PARAMS()

storage_seq_xts_db_intr_override:

    // Bubble up to stage 4
    mfspr       r_stage, spr_mpuid
    slt         c1, r_stage[4:2], SEQ_INTR_OVERRIDE_STAGE
    nop.c1.e
    nop
    
    bbne        SEQ_KIVEC5XTS_INTR_EN, 0, intr_override
    nop
    
    // cancel any barco push prep
    SEQ_XTS_NEXT_DB_CANCEL(dma_p2m_19)
    b           exit
    nop
    
intr_override:
    PCI_SET_INTERRUPT_ADDR_DMA(SEQ_KIVEC10_INTR_ADDR,
                               dma_p2m_19)
    SEQ_METRICS_SET(interrupts_raised)
    
exit:

   // Relaunch stats commit for table 1
   SEQ_METRICS0_TABLE1_COMMIT_e(SEQ_KIVEC5XTS_SRC_QADDR)
    

#include "app_redir_common.h"

struct phv_             p;
struct s5_tbl_k         k;

/*
 * Registers usage
 */
#define r_stage                     r3

%%
    .param      rawr_ppage_free
    
    .align

/*
 * Common code to lanuch cleanup code to free ppage
 * due to semaphore pindex full on one or more such resources.
 */
rawr_cleanup_discard:

    /*
     * Free ppage via ASCQ (ARM Send Completion Queue)
     */
    seq         c2, RAWR_KIVEC0_PPAGE, r0       // delay slot
    nop.c2.e
    CAPRI_CLEAR_TABLE0_VALID                    // delay slot
    
    bbeq        RAWR_KIVEC0_PKT_FREEQ_NOT_CFG, 0, _ppage_free_launch
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP_e(0, rawr_ppage_free)
    
_ppage_free_launch:
    CAPRI_NEXT_TABLE_READ_e(3, TABLE_LOCK_DIS,
                            rawr_ppage_free,
                            RAWR_KIVEC1_ASCQ_SEM_INF_ADDR,
                            TABLE_SIZE_64_BITS)
    nop                            

#include "app_redir_common.h"

struct phv_                         p;
struct rawr_desc_k                  k;
struct rawr_desc_desc_post_alloc_d  d;

%%

    .param      rawr_s3_chain_pindex_pre_alloc
    .align
    
rawr_s2_desc_post_alloc:

    CAPRI_CLEAR_TABLE0_VALID

    /*
     * Move to common stage to launch ARQ semaphore read.
     * In the case of SPAN, post hashing result will handle the stage advance.
     */ 
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, rawr_s3_chain_pindex_pre_alloc)

    seq.e       c1, k.common_phv_desc_sem_pindex_full, r0
    phvwr.c1    p.to_s4_desc, d.desc  // delay slot

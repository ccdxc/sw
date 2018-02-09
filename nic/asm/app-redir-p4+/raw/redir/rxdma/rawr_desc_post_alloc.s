#include "app_redir_common.h"

struct phv_                         p;
struct rawr_desc_k                  k;
struct rawr_desc_desc_post_alloc_d  d;

%%

    .param      rawr_s3_toeplitz_hash_none
    .align
    
rawr_s2_desc_post_alloc:

    CAPRI_CLEAR_TABLE0_VALID

    /*
     * Move to common stage to launch ARQ semaphore read.
     * In the case of SPAN, post hashing result will handle the stage advance
     * so we cancel the stage launch here
     */
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, rawr_s3_toeplitz_hash_none)
    
#if !APP_REDIR_VISIBILITY_USE_MIRROR_SESSION
    sne         c1, k.common_phv_redir_span_instance, r0
    phvwri.c1   p.app_header_table1_valid, 0
#endif

    seq.e       c1, k.common_phv_desc_sem_pindex_full, r0
    phvwr.c1    p.to_s5_desc, d.desc  // delay slot

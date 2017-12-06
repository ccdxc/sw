#include "app_redir_common.h"

struct phv_                         p;
struct rawr_desc_k                  k;
struct rawr_desc_desc_post_alloc_d  d;

%%

    .param      rawr_s3_chain_qidxr_stage_advance
    .align
    
rawr_s2_desc_post_alloc:

    CAPRI_CLEAR_TABLE0_VALID

    phvwr       p.to_s6_desc, d.desc

    /*
     * Service chain RxQs that are ARM CPU bound (e.g. ARQs) cannot use
     * semaphores because the ARM does not have HW support for semaphore
     * manipulation (for freeing). In such cases, special HBM queue index
     * regions are provided for direct access under table lock to prevent
     * race condition.
     *
     * Note: table lock is only effective for a given stage so all P4+
     * programs must coordinate so that they lock a given table in
     * the same stage. For the ARM ARQ, that is stage 6.
     */ 
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, rawr_s3_chain_qidxr_stage_advance)
    nop.e
    nop

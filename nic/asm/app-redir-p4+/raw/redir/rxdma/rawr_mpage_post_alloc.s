#include "app_redir_common.h"

struct phv_                             p;
struct rawr_mpage_k                     k;
struct rawr_mpage_mpage_post_alloc_d    d;

%%
    .align

rawr_s2_mpage_post_alloc:

    CAPRI_CLEAR_TABLE2_VALID

    seq.e       c1, k.common_phv_mpage_sem_pindex_full, r0
    phvwr.c1    p.to_s4_mpage, d.page     // delay slot
    

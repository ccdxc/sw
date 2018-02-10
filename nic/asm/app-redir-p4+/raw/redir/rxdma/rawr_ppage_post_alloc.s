#include "app_redir_common.h"

struct phv_                             p;
struct rawr_ppage_k                     k;
struct rawr_ppage_ppage_post_alloc_d    d;

%%
    .align

rawr_s2_ppage_post_alloc:

    CAPRI_CLEAR_TABLE1_VALID

    seq.e       c1, k.common_phv_ppage_sem_pindex_full, r0
    phvwr.c1    p.to_s4_ppage, d.page       // delay slot
    

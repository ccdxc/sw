#include "rawr-defines.h"

struct phv_                             p;
struct rawr_ppage_k                     k;
struct rawr_ppage_ppage_post_alloc_d    d;

%%
    .align

rawr_s2_ppage_post_alloc:

    CAPRI_CLEAR_TABLE1_VALID

    phvwri      p.common_phv_ppage_valid, 1
    phvwr.e     p.to_s6_ppage, d.page
    nop
    

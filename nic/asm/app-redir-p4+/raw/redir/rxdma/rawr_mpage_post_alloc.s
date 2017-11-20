#include "app_redir_common.h"

struct phv_                             p;
struct rawr_mpage_k                     k;
struct rawr_mpage_mpage_post_alloc_d    d;

%%
    .align

rawr_s2_mpage_post_alloc:

    CAPRI_CLEAR_TABLE2_VALID

    phvwri      p.common_phv_mpage_valid, TRUE
    phvwr.e     p.to_s6_mpage, d.page
    nop
    

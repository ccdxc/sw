#include "app_redir_common.h"

struct phv_                                         p;
struct proxyc_cpu_flags_post_k                      k;
struct proxyc_cpu_flags_post_cpu_flags_post_read_d  d;

/*
 * Registers usage
 */
#define r_cpu_flags                 r1

%%

    .param      proxyc_s3_cleanup_discard_prep
    .align

/*
 * Save cpu_to_p4plus_header_t flags which tell us how to free the AOL
 * pages in the current descriptor, that is, if we need to do so
 * as indicated in k.to_s3_do_cleanup_discard.
 */
proxyc_s3_cpu_flags_post_read:

    CAPRI_CLEAR_TABLE1_VALID

    add         r_cpu_flags, r0, d.{flags}.hx
    smeqh       c1, r_cpu_flags, PEN_APP_REDIR_A0_RNMPR_SMALL,  \
                                 PEN_APP_REDIR_A0_RNMPR_SMALL
    phvwri.c1   p.t1_s2s_aol_A0_small, TRUE
    smeqh       c1, r_cpu_flags, PEN_APP_REDIR_A1_RNMPR_SMALL,  \
                                 PEN_APP_REDIR_A1_RNMPR_SMALL
    phvwri.c1   p.t1_s2s_aol_A1_small, TRUE
    smeqh       c1, r_cpu_flags, PEN_APP_REDIR_A2_RNMPR_SMALL,  \
                                 PEN_APP_REDIR_A2_RNMPR_SMALL
    phvwri.c1   p.t1_s2s_aol_A2_small, TRUE

_cleanup_discard_check:
    
    sne         c1, k.common_phv_do_cleanup_discard, r0
    j.c1        proxyc_s3_cleanup_discard_prep
    nop.!c1.e
    nop

    .align

/*
 * Entered as a stage transition when cpu_to_p4plus_header_t flags
 * were not available to be read.
 */         
proxyc_s3_cpu_flags_skip_read:

    CAPRI_CLEAR_TABLE2_VALID
    b           _cleanup_discard_check
    nop


    

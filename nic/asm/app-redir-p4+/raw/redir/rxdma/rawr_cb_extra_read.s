#include "app_redir_common.h"

struct phv_                     p;
struct s1_tbl1_k                k;
struct s1_tbl1_cb_extra_read_d  d;

/*
 * Registers usage
 */
#define r_ascq_sem_inf_addr         r3
#define r_ascq_base                 r4

%%

    .align
    
rawr_cb_extra_read:

    CAPRI_CLEAR_TABLE1_VALID
    add         r_ascq_sem_inf_addr, d.{ascq_sem_inf_addr}.dx, r0
    add         r_ascq_base, d.{ascq_base}.dx, r0
    seq         c1, r_ascq_sem_inf_addr, r0
    phvwri.c1   p.rawr_kivec0_pkt_freeq_not_cfg, TRUE
    seq         c1, r_ascq_base, r0
    phvwri.c1   p.rawr_kivec0_pkt_freeq_not_cfg, TRUE
    
    phvwr.e     p.rawr_kivec1_ascq_sem_inf_addr, r_ascq_sem_inf_addr
    phvwr       p.rawr_kivec3_ascq_base, r_ascq_base


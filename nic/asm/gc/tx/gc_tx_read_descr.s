#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"

struct phv_ p;
struct gc_tx_read_descr_read_descr_d d;

%%
    .param          gc_tx_inc_rnmdr_free_pair_pi
    .param          gc_tx_inc_tnmdr_free_pair_pi

.align
gc_tx_read_rnmdr_descr:
    phvwr           p.t0_s2s_a0, d.A0
    phvwr           p.t0_s2s_a1, d.A1
    phvwr           p.t0_s2s_a2, d.A2
    /*
     * Read descriptor FP.PI, to get index to write the freed descr address to
     */
    CAPRI_NEXT_TABLE_READ_i(0, TABLE_LOCK_DIS, gc_tx_inc_rnmdr_free_pair_pi,
                    RNMDR_FREE_IDX, TABLE_SIZE_64_BITS)
    nop.e
    nop

.align
gc_tx_read_tnmdr_descr:
    phvwr           p.t0_s2s_a0, d.A0
    phvwr           p.t0_s2s_a1, d.A1
    phvwr           p.t0_s2s_a2, d.A2
    /*
     * Read descriptor FP.PI, to get index to write the freed descr address to
     */
    CAPRI_NEXT_TABLE_READ_i(0, TABLE_LOCK_DIS, gc_tx_inc_tnmdr_free_pair_pi,
                    TNMDR_FREE_IDX, TABLE_SIZE_64_BITS)
    nop.e
    nop

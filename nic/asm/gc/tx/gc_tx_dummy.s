#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"

struct phv_ p;

%%
    .param          gc_tx_inc_rnmdr_free_pair_pi
    .param          gc_tx_inc_tnmdr_free_pair_pi
    .param          GC_GLOBAL_TABLE_BASE

.align
gc_tx_rnmdr_dummy:
    /*
     * Read descriptor FP.PI, to get index to write the freed descr address to
     */
    CAPRI_NEXT_TABLE_READ_i(0, TABLE_LOCK_EN, gc_tx_inc_rnmdr_free_pair_pi,
                    GC_GLOBAL_TABLE_BASE, TABLE_SIZE_64_BITS)
    nop.e
    nop

.align
gc_tx_tnmdr_dummy:
    /*
     * Read descriptor FP.PI, to get index to write the freed descr address to
     */
    CAPRI_NEXT_TABLE_READ_i(0, TABLE_LOCK_EN, gc_tx_inc_tnmdr_free_pair_pi,
                    GC_GLOBAL_TABLE_BASE, TABLE_SIZE_64_BITS)
    nop.e
    nop

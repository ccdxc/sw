#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_hdr_template_t d;
struct req_tx_s5_t0_k k;

%%

.align
req_tx_load_hdr_template_process:

    // Pin load_hdr_template to stage 5 to hide HBM lookup latency with other lookups in this stage.
    // Currently this is only invoked for inline/zero-length packets.
    // TODO this should go in parallel to wqe/sge stage
    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_5
    bcf           [!c1], bubble_to_next_stage
    nop

    phvwr         p.pad1, d.data
    phvwr         p.pad2[511:496], r0[15:0]

    nop.e
    CAPRI_SET_TABLE_0_VALID(0)

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_4
    bcf           [!c1], exit
    nop           // Branch Delay Slot

    //invoke the same routine, but with valid header-template-addr entry as d[] vector
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, k.common_te0_phv_table_addr)

exit:
    nop.e
    nop


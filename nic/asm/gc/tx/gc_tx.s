#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"

struct phv_ p;
struct gc_tx_initial_action_k k;
struct gc_tx_initial_action_initial_action_d d;

%%
    .param          gc_tx_read_rnmdr_addr
    .param          gc_tx_read_tnmdr_addr

/*
 * Each ring (RNMDR, TNMDR etc.) that needs to be garbage collected, gets its
 * own Qtype, and hence its own start offset.
 *
 * Each producer (TCP, TLS, IPSEC etc.) that need to free to a ring, gets a QID
 * The ring id is always 0
 */

/*
 * free a descriptor into RNMDR
 */
.align
gc_tx_rnmdr_s0_start:
    CAPRI_OPERAND_DEBUG(d.ring_base)

    /*
     * Check pi == ci and exit if no work to be done
     */
    seq             c1, d.{ci_0}, d.{pi_0}
    b.c1            gc_tx_abort

    /*
     * r5 = Address to read from descriptor GC ring
     *
     * r4 = doorbell address
     * r3 = doorbell data
     */

    // sw_ci is used for queue full check - set this to
    // the current value
    tblwr           d.sw_ci, d.{ci_0}.hx

    // r5 is the start address in ring to launch next read
    add             r5, d.{ring_base}.dx, d.{ci_0}.hx, RNMDR_TABLE_ENTRY_SIZE_SHFT

    /*
     * r1[7:0] = number of entries to clean (max 8 or until end of ring)
     *
     */
    sub             r1, d.{pi_0}.hx, d.{ci_0}.hx
    and             r1, r1, RNMDR_GC_PER_PRODUCER_MASK
    // clean up max of 8
    slt             c1, 8, r1
    add.c1          r1, r0, 8
    // don't clean up more than end of ring
    sub             r2, RNMDR_GC_PER_PRODUCER_SIZE, d.{ci_0}.hx
    slt             c1, r2, r1
    add.c1          r1, r0, r2

    tblmincr.f      d.{ci_0}.hx, RNMDR_GC_PER_PRODUCER_SHIFT, r1
    phvwr           p.common_phv_num_entries_freed, r1

    /*
     * Ring doorbell to EVAL if pi == ci
     */
    seq             c1, d.{ci_0}.hx, d.{pi_0}.hx
    b.!c1           gc_tx_launch_rnmdr_and_end
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, \
                        DB_SCHED_UPD_EVAL, ASIC_HBM_GC_RNMDR_QTYPE, LIF_GC)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA_QID(k.p4_txdma_intr_qid)
    memwr.dx        r4, r3

gc_tx_launch_rnmdr_and_end:
    CAPRI_NEXT_TABLE_READ_e(0, TABLE_LOCK_DIS, gc_tx_read_rnmdr_addr, r5, TABLE_SIZE_512_BITS)
    nop

/*
 * free a descriptor into TNMDR
 */
.align
gc_tx_tnmdr_s0_start:
    CAPRI_OPERAND_DEBUG(d.ring_base)

    /*
     * Check pi == ci and exit if no work to be done
     */
    seq             c1, d.{ci_0}, d.{pi_0}
    b.c1            gc_tx_abort

    /*
     * r5 = Address to read from descriptor GC ring
     *
     * r4 = doorbell address
     * r3 = doorbell data
     */

    // sw_ci is used for queue full check - set this to
    // the current value
    tblwr           d.sw_ci, d.{ci_0}.hx

    // r5 is the start address in ring to launch next read
    add             r5, d.{ring_base}.dx, d.{ci_0}.hx, TNMDR_TABLE_ENTRY_SIZE_SHFT

    /*
     * r1[7:0] = number of entries to clean (max 8 or until end of ring)
     *
     */
    sub             r1, d.{pi_0}.hx, d.{ci_0}.hx
    and             r1, r1, TNMDR_GC_PER_PRODUCER_MASK
    // clean up max of 8
    slt             c1, 8, r1
    add.c1          r1, r0, 8
    // don't clean up more than end of ring
    sub             r2, TNMDR_GC_PER_PRODUCER_SIZE, d.{ci_0}.hx
    slt             c1, r2, r1
    add.c1          r1, r0, r2

    tblmincr.f      d.{ci_0}.hx, TNMDR_GC_PER_PRODUCER_SHIFT, r1
    phvwr           p.common_phv_num_entries_freed, r1

    /*
     * Ring doorbell to EVAL if pi == ci
     */
    seq             c1, d.{ci_0}.hx, d.{pi_0}.hx
    b.!c1           gc_tx_launch_tnmdr_and_end
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, \
                        DB_SCHED_UPD_EVAL, ASIC_HBM_GC_TNMDR_QTYPE, LIF_GC)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA_QID(k.p4_txdma_intr_qid)
    memwr.dx        r4, r3

gc_tx_launch_tnmdr_and_end:
    CAPRI_NEXT_TABLE_READ_e(0, TABLE_LOCK_DIS, gc_tx_read_tnmdr_addr, r5, TABLE_SIZE_512_BITS)
    nop

gc_tx_abort:
    nop.e
    phvwr           p.p4_intr_global_drop, 1


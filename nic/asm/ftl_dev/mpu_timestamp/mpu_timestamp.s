#include "ftl_dev_shared_asm.h"

struct phv_                             p;
struct s0_tbl_k                         k;
struct s0_tbl_timestamp_start_d         d;

/*
 * Table engine always fetches the qstate (in d-vector)
 * for processing by the stage MPU.
 * 
 * In addition, 
 * R1 = table lookup hash value
 * R2 = packet size
 * R3 = random number
 * R4 = current time
 * R5 = programmable table constant
 * R6 = phv timestamp
 * R7 = qstate ring[7:0] not_empty
 */
 
/*
 * Registers usage
 */
#define r_timestamp                     r4      // must use r4 (see above comments)
#define r_not_timestamp                 r5      // complement of timestamp
#define r_db_addr                       r6
#define r_db_data                       r7

%%
    .align

mpu_timestamp_start:

    SCANNER_DB_ADDR_SCHED_EVAL(HW_MPU_INTRINSIC_LIF,
                               HW_MPU_TXDMA_INTRINSIC_QTYPE)
.brbegin
    brpri       r7[MPU_TIMESTAMP_RING_MAX-1:0], [1, 0]
    seq         c2, d.cb_activate, MPU_TIMESTAMP_CB_ACTIVATE // delay slot
    
.brcase MPU_TIMESTAMP_RING_NORMAL
    
    setcf       c3, [c0]        // will consume ring 0 slot
    SCANNER_DB_DATA_WITH_RING(HW_MPU_TXDMA_INTRINSIC_QID,
                              MPU_TIMESTAMP_RING_NORMAL) 
_timestamp_start:

    // On packet reception, P4 is expected to stamp flows with only a subset
    // of the timestamp bits. However, we return the full timestamp here
    // to give SW more flexibility.
    //
    // Also, when the MPU program completes, the entire table will be flushed
    // as one atomic write (the ARM will never see any partial writes).
    
    tblwr.c2    d.timestamp, r_timestamp[47:0]
    tbladd.c2   d.num_updates, 1
    
    /*
     * Consume ring slot and issue scheduler update eval
     */                          
    tblwr.c3.f  d.ci_0, d.pi_0
    tblwr.!c3.f d.ci_1, d.pi_1
    memwr.dx    r_db_addr, r_db_data
    
    phvwr.!c2.e p.p4_intr_global_drop, 1
    CLEAR_TABLE0                                        // delay slot

    // CB in activated state so reschedule with a timer
    
    SCANNER_DB_ADDR_TIMER(MPU_TIMESTAMP_RESCHED_TIMER,
                          HW_MPU_INTRINSIC_LIF)
    SCANNER_DB_DATA_TIMER_WITH_RING(HW_MPU_TXDMA_INTRINSIC_QTYPE,
                                    HW_MPU_TXDMA_INTRINSIC_QID,
                                    MPU_TIMESTAMP_RING_TIMER,
                                    MPU_TIMESTAMP_RESCHED_TICKS)
    phvwr.e     p.p4_intr_global_drop, 1
    memwr.dx    r_db_addr, r_db_data                    // delay slot
    
.brcase MPU_TIMESTAMP_RING_TIMER

    SCANNER_DB_DATA_WITH_RING(HW_MPU_TXDMA_INTRINSIC_QID,
                              MPU_TIMESTAMP_RING_TIMER) 
    b           _timestamp_start
    setcf       c3, [!c0]       // will consume ring 1 slot
    
.brcase MPU_TIMESTAMP_RING_MAX
    
    phvwr.e     p.p4_intr_global_drop, 1
    CLEAR_TABLE0                                        // delay slot
    
.brend


#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_write_back_process_k_t k;
struct sqcb0_t d;

%%

.align
req_tx_write_back_process:
     tblwr         d.busy, k.args.busy
     seq           c1, k.args.release_cb1_busy, 1
     tblwr.c1      d.cb1_busy, 0
     tblwr         d.num_sges, k.args.num_sges
     tblwr         d.in_progress, k.args.in_progress
     tblwr         d.current_sge_id, k.args.current_sge_id
     tblwr         d.current_sge_offset, k.args.current_sge_offset
     tblwr         d.curr_wqe_ptr, k.to_stage.wqe_addr
     tblwr         d.curr_op_type, k.args.op_type

     // if (write_back_info_p->last)
     // RING_C_INDEX_INCREMENT(sqcb0_p, SQ_RING_ID)
     seq           c1, k.args.last, 1
     //tblmincri.c1  SQ_C_INDEX, d.log_num_wqes, 1
     bcf           [!c1], skip_db_update
     nop           //BD Slot

     //add.c1        r1, SQ_C_INDEX, 1 // Delay slot
     //add           r1, SQ_C_INDEX, r0
     //mincr         r1, d.log_num_wqes, 1
     tblmincri     SQ_C_INDEX, d.log_num_wqes, 1
     // Ordering rules:
     // We decided NOT to ring doorbell to update the c_index and there by re-evaluate scheduler.
     // The decision is taken because of the ordering complexity between cb0/cb1 busy bit release
     // and cindex increments.
     // Rather, we simply use above tblmincri to increment c_index using table writes.
     // Since doorbell is not re-evaluated, scheduler may invoke us one more time.
     // Then in stage0 we check r7 register (which has a bitmap of rings not empty). If this register
     // is 0, then we invoke doorbell to turn off scheduler.
     // This way, we don't need to invoke doorbell invocation (which is 4-5 instructions) for every
     // wqe consumption. As long as qp has enough work posted, we should be fine. Disadvantage is
     // that if wqes are posted sporadically, we may encounter one more scheduler invocation for
     // every wqe that is posted.
     //DOORBELL_WRITE_CINDEX(k.global.lif, k.global.qtype, k.global.qid, SQ_RING_ID, SQ_C_INDEX, r2, r3)

skip_db_update:

     add           r1, k.args.tbl_id, r0
     CAPRI_SET_TABLE_I_VALID(r1, 0)

     nop.e
     nop


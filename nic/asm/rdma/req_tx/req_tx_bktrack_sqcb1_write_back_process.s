#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_bktrack_sqcb1_write_back_process_k_t k;
struct sqcb1_t d;

%%

.align
req_tx_bktrack_sqcb1_write_back_process:
     tblwr         d.tx_psn, k.args.tx_psn
     tblwr         d.ssn, k.args.ssn
     seq           c1, k.args.skip_wqe_start_psn, 1
     tblwr.!c1     d.wqe_start_psn, k.args.wqe_start_psn
    
     // Release cb1 busy bit
     SQCB0_ADDR_GET(r1)
     add            r2, r1, FIELD_OFFSET(sqcb0_t, cb1_byte)
     memwr.b        r2, r0

     add           r1, k.args.tbl_id, r0
     CAPRI_SET_TABLE_I_VALID(r1, 0)

     nop.e
     nop

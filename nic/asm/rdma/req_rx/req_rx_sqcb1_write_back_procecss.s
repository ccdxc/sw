#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct req_rx_sqcb1_write_back_process_k_t k;
struct sqcb1_t d;

%%

.align
req_rx_sqcb1_write_back_process:
     tblwr         d.in_progress, k.args.in_progress
     tblwr         d.rrqwqe_cur_sge_id, k.args.cur_sge_id
     tblwr         d.rrqwqe_cur_sge_offset, k.args.cur_sge_offset
     tblwr         d.e_rsp_psn, k.args.e_rsp_psn
     tblwr         d.rexmit_psn, k.args.rexmit_psn
     seq           c1, k.args.incr_nxt_to_go_token_id, 1
     tblmincri.c1  d.nxt_to_go_token_id, SIZEOF_TOKEN_ID_BITS, 1

     add           r1, k.args.tbl_id, r0
     CAPRI_SET_TABLE_I_VALID(r1, 0)

     nop.e
     nop


#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct rrqwqe_t d;
struct req_rx_rrqwqe_process_k_t k;

#define RRQWQE_TO_SGE_T struct req_rx_rrqwqe_to_sge_info_t

%%

    .param    req_rx_rrqsge_process

.align
req_rx_rrqwqe_process:

    // TODO ack processing

   // if (in_progress) 
   //     pkt->psn == sqcb1_p->e_rsp_psn
   //  else
   //     pkt->psn == rrqwqe_p->psn 
   seq             c1, k.args.in_progress, 1 
   sne.c1          c2, k.to_stage.bth_psn, k.args.e_rsp_psn
   sne.!c1         c2, k.to_stage.bth_psn, d.psn
   bcf             [c2], out_of_order_rsp

   sle             c2, d.msn, k.to_stage.msn
   sub.c2          r1, d.msn, 1           

   add             r5, r0, k.global.flags
   ARE_ALL_FLAGS_SET(c3, r5, REQ_RX_FLAG_FIRST)

   // if (first)
   //     min((rrqwqe_p->msn -1), sqcb1_to_rrqwqe_info_p->psn)
   // else
   //     min(rrqwqe_p->msn, sqcb1_to_rrqwqe_info_p->psn)
   cmov.c3         r1, c2, r1, k.to_stage.msn
   cmov.!c3        r1, c2, d.msn, k.to_stage.msn   
   phvwr           p.cqwqe.id.msn, r1

   seq             c2, d.read_rsp_or_atomic, RSQ_OP_TYPE_READ
   bcf             [!c2], atomic

read:
    CAPRI_GET_TABLE_0_ARG(req_rx_phv_t, r7)
    CAPRI_SET_FIELD(r7, RRQWQE_TO_SGE_T, is_atomic, 0)
    CAPRI_SET_FIELD(r7, RRQWQE_TO_SGE_T, remaining_payload_bytes, k.args.remaining_payload_bytes)
    CAPRI_SET_FIELD(r7, RRQWQE_TO_SGE_T, cur_sge_id, k.args.cur_sge_id)
    CAPRI_SET_FIELD(r7, RRQWQE_TO_SGE_T, cur_sge_offset, k.args.cur_sge_offset)
    CAPRI_SET_FIELD(r7, RRQWQE_TO_SGE_T, in_progress, k.args.in_progress)
    sub            r1, d.num_sges, k.args.cur_sge_id
    CAPRI_SET_FIELD(r7, RRQWQE_TO_SGE_T, num_valid_sges, r1)
    cmov           r1, c1, k.args.e_rsp_psn, d.psn
    add            r1, r1, 1
    CAPRI_SET_FIELD(r7, RRQWQE_TO_SGE_T, e_rsp_psn, r1)

    CAPRI_GET_TABLE_0_K(req_rx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_rx_rrqsge_process)
    add            r1, d.read.wqe_sge_list_addr, k.args.cur_sge_id, LOG_SIZEOF_SGE_T
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r1)
    b              set_cqcb_arg

atomic:
   
   

set_cqcb_arg:

out_of_order_rsp:

    nop.e
    nop

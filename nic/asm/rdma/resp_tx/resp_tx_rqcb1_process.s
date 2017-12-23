#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_tx_phv_t p;
struct rqcb1_t d;
struct resp_tx_rqcb1_process_k_t k;

#define RSQWQE_INFO_T struct resp_tx_rqcb1_to_rsqwqe_info_t
#define RAW_TABLE_PC    r3
#define TO_STAGE_T struct resp_tx_to_stage_t

%%
    .param      resp_tx_rsqwqe_process

resp_tx_rqcb1_process:

    phvwr       p.bth.dst_qp, d.dst_qp

    // prepare aeth
    phvwr       p.aeth.msn, d.aeth.msn
    phvwr       p.aeth.syndrome, d.aeth.syndrome

    // check for read_rsp_lock. If already taken, give up scheduling 
    // opportunity
    seq         c1, d.read_rsp_lock, 1
    bcf         [c1], drop
    nop         // BD Slot

    // take lock
    tblwr       d.read_rsp_lock, 1
    
    CAPRI_GET_TABLE_0_ARG(resp_tx_phv_t, r4)
    CAPRI_SET_FIELD(r4, RSQWQE_INFO_T, curr_read_rsp_psn, d.curr_read_rsp_psn)
    CAPRI_SET_FIELD(r4, RSQWQE_INFO_T, serv_type, k.args.serv_type)
    CAPRI_SET_FIELD(r4, RSQWQE_INFO_T, log_pmtu, k.args.log_pmtu)
    CAPRI_SET_FIELD(r4, RSQWQE_INFO_T, header_template_addr, d.header_template_addr)
    CAPRI_SET_FIELD(r4, RSQWQE_INFO_T, header_template_size, d.header_template_size)
    CAPRI_SET_FIELD(r4, RSQWQE_INFO_T, read_rsp_in_progress, d.read_rsp_in_progress)

    // Pass dcqcn_cb_addr to stage 3.
    add         r2, HDR_TEMPLATE_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT
    CAPRI_GET_STAGE_3_ARG(resp_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, s3.rsq_rkey.dcqcn_cb_addr, r2)

    CAPRI_GET_TABLE_0_K(resp_tx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_tx_rsqwqe_process)

    // load 256 bits (i.e,32 bytes) which is the size of rsqwqe
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, RAW_TABLE_PC, k.args.rsqwqe_addr)

exit:
    nop.e
    nop

drop:
    phvwr.e   p.common.p4_intr_global_drop, 1
    nop

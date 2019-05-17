#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t0_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s1_t0_k_ k;
struct s1_t0_nvme_req_tx_sqe_process_d d;

%%
    .param  nvme_req_tx_nscb_process
    .param  nvme_nscb_base

.align
nvme_req_tx_sqe_process:

    //copy important sqe params to cmd ctxt
    phvwr       p.{cmd_ctxt_opc...cmd_ctxt_nsid}, d.{opc...nsid}
    phvwr       p.{cmd_ctxt_slba...cmd_ctxt_nlb}, d.{slba...nlb}
    phvwrpair   p.{cmd_ctxt_lif...cmd_ctxt_sq_id}, K_GLOBAL_LIF_QID, \
                p.{cmd_ctxt_session_id...cmd_ctxt_pad}, r0

    phvwr       p.to_s2_info_opc, d.opc
    phvwr       p.to_s2_info_slba, d.{slba}.dx
    phvwr       p.to_s2_info_nlb, d.{nlb}.hx
   
    phvwr       p.t0_s2s_sqe_to_nscb_info_prp1, d.{prp1}.dx
    phvwr       p.t0_s2s_sqe_to_nscb_info_prp2, d.{prp2}.dx
   
    //calculate nscb address = 
    //  nscb_base + ((lif_ns_start + nsid -1) << sizeof(nscb))
    addui   r2, r0, hiword(nvme_nscb_base)
    addi    r2, r2, loword(nvme_nscb_base)
    // make nsid 0-based
    sub     r1, d.{nsid}.wx, 1
    add     r1, r1, k.to_s1_info_lif_ns_start
    add     r1, r2, r1, LOG_NSCB_SIZE

    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_EN,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_req_tx_nscb_process,
                                r1) //Exit Slot

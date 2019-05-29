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

#define CID     r3
#define NSID    r4
#define SLBA    r5
#define NLB     r6

.align
nvme_req_tx_sqe_process:

    //copy important sqe params to cmd ctxt and pdu ctxt

    //phvwr       p.{cmd_ctxt_opc...cmd_ctxt_nsid}, d.{opc...nsid}
    //phvwr       p.{cmd_ctxt_slba...cmd_ctxt_nlb}, d.{slba...nlb}

    add         CID, r0, d.{cid}.hx
    add         NSID, r0, d.{nsid}.wx
    add         SLBA, r0, d.{slba}.dx
    add         NLB, r0, d.{nlb}.hx

    //store params in big-endian format in cmd-ctxt
    //for some reason, phvwrpair is not working in combination with 
    //endian conversion
    phvwr       p.{cmd_ctxt_opc...cmd_ctxt_psdt}, d.{opc...psdt}
    phvwrpair   p.cmd_ctxt_cid, CID, \
                p.cmd_ctxt_nsid, NSID
    phvwrpair   p.cmd_ctxt_slba, SLBA, \
                p.cmd_ctxt_nlb, NLB
    phvwrpair   p.cmd_ctxt_prp1, d.prp1, \
                p.cmd_ctxt_prp2, d.prp2

    phvwrpair   p.pdu_ctxt0_cmd_opc, d.opc, \
                p.pdu_ctxt0_pdu_opc, NVME_O_TCP_PDU_TYPE_CAPSULECMD
    phvwrpair   p.pdu_ctxt0_slba, SLBA, \
                p.pdu_ctxt0_nlb, NLB
    
    phvwrpair   p.{cmd_ctxt_lif...cmd_ctxt_sq_id}, K_GLOBAL_LIF_QID, \
                p.{cmd_ctxt_num_prps...cmd_ctxt_state}, r0

    phvwr       p.to_s2_info_opc, d.opc
    phvwrpair   p.to_s2_info_slba, SLBA, \
                p.to_s2_info_nlb, NLB
   
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

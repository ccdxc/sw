#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t0_nvme_sessprexts_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s1_t0_nvme_sessprexts_tx_k_ k;
struct s1_t0_nvme_sessprexts_tx_sess_wqe_process_d d;

#define PDU_CTXT_P  r6
#define HWXTSCB_P   r7

%%
    .param  nvme_tx_pdu_context_base
    .param  nvme_sessprexts_tx_pdu_ctxt_process
    .param  nvme_tx_hwxtscb_addr
    .param  nvme_sessprexts_tx_xtscb_process

.align
nvme_sessprexts_tx_sess_wqe_process:
    addui   PDU_CTXT_P, r0, hiword(nvme_tx_pdu_context_base)
    addi    PDU_CTXT_P, PDU_CTXT_P, loword(nvme_tx_pdu_context_base)
    add     PDU_CTXT_P, PDU_CTXT_P, d.pduid, LOG_PDU_CTXT_SIZE

    phvwr   p.to_s3_info_pdu_ctxt_ptr, PDU_CTXT_P

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN,
                              CAPRI_TABLE_SIZE_512_BITS,
                              nvme_sessprexts_tx_pdu_ctxt_process,
                              PDU_CTXT_P) //Exit Slot

    addui   HWXTSCB_P, r0, hiword(nvme_tx_hwxtscb_addr)
    addi    HWXTSCB_P, HWXTSCB_P, loword(nvme_tx_hwxtscb_addr)

    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_EN,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_sessprexts_tx_xtscb_process,
                                HWXTSCB_P) //Exit Slot

exit:
    phvwr.e p.p4_intr_global_drop, 1
    nop             //Exit Slot

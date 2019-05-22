#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t0_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s1_t0_k_ k;
struct s1_t0_nvme_sesspostxts_tx_sess_wqe_process_d d;

%%
    .param  nvme_txsessprodcb_base
    .param  nvme_sesspostxts_tx_sessprodcb_process

.align
nvme_sesspostxts_tx_sess_wqe_process:
    phvwr       p.sess_wqe_cid, d.cid 

    addui       r6, r0, hiword(nvme_txsessprodcb_base)
    addi        r6, r6, loword(nvme_txsessprodcb_base)
    add         r6, r6, k.to_s1_info_session_id, LOG_TXSESSPRODCB_SIZE

    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_0_BITS,
                                nvme_sesspostxts_tx_sessprodcb_process,
                                r6) //Exit Slot

#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t1_nvme_req_rx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s2_t1_nvme_req_rx_k_ k;
struct s2_t1_nvme_req_rx_pduid_fetch_process_d d;

#define TO_S_WB_P       to_s5_info

#define PDU_CTXT_P      r6

%%
    .param  nvme_rx_pdu_context_base

.align
nvme_req_rx_pduid_fetch_process:
    
    CAPRI_SET_FIELD2(TO_S_WB_P, pduid, PDUID)

exit:
    phvwr.e     p.sess_wqe_pduid, PDUID
    CAPRI_SET_TABLE_1_VALID(0) // Exit Slot

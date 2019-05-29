#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s6_t0_nvme_req_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s6_t0_nvme_req_tx_k_ k;
struct s6_t0_nvme_req_tx_sqcb_writeback_process_d d;

%%

.align
nvme_req_tx_sqcb_writeback_process:

    tblmincri       SQ_C_INDEX, d.log_num_wqes, 1
    tblwr.f         d.busy, 0
    nop.e
    CAPRI_SET_TABLE_0_VALID(0)  //Exit Slot

        

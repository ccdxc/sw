#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s6_t0_nvme_req_rx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s6_t0_nvme_req_rx_k_ k;
struct s6_t0_nvme_req_rx_incr_refcnt_process_d d;

#define IN_P        t0_s2s_rqcb_writeback_to_incr_refcnt_info

%%

.align
nvme_req_rx_incr_refcnt_process:
    
    tblmincri        d.refcnt, 1, 1
    tblwr.e          d.more_pdus, CAPRI_KEY_FIELD(IN_P, more_pdus)
    CAPRI_SET_TABLE_0_VALID(0) // Exit Slot

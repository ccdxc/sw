#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s5_t1_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s5_t1_k_ k;
struct s5_t1_nvme_req_tx_cmdid_fetch_process_d d;

%%

.align
nvme_req_tx_cmdid_fetch_process:
    phvwr       p.sess_wqe_cid, CMDID
    CAPRI_SET_TABLE_1_VALID(0)
    nop.e
    nop

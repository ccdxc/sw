#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s5_t2_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s5_t2_k_ k;
struct s5_t2_nvme_req_tx_aolid_fetch_process_d d;

#define DMA_CMD_BASE    r5
#define CMD_CTXT_P      r6
#define TMP_R           r7

%%
    .param  nvme_cmd_context_base

.align
nvme_req_tx_aolid_fetch_process:
    
    phvwr.e     p.cmd_ctxt_aolid, AOLID
    CAPRI_SET_TABLE_2_VALID(0)      //Exit Slot

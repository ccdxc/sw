#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s6_t0_nvme_sesspostxts_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s6_t0_nvme_sesspostxts_tx_k_ k;
struct s6_t0_nvme_sesspostxts_tx_cb_writeback_process_d d;

%%

.align
nvme_sesspostxts_tx_cb_writeback_process:

    tblmincri       SESSPOSTXTSTX_C_INDEX, d.log_num_entries, 1
    tblmincri.f.e   d.wb_r1_busy, 1, 1  //Flush

    CAPRI_SET_TABLE_0_VALID(0)  //Exit Slot

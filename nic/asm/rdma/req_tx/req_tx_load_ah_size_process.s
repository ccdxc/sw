#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

#define TO_S5_SQCB_WB_P to_s5_sqcb_wb_add_hdr_info
struct req_tx_phv_t p;
struct req_tx_ah_size_t d;

%%

.align
req_tx_load_ah_size_process:

    phvwr.e           CAPRI_PHV_FIELD(TO_S5_SQCB_WB_P, ah_size), d.size

    CAPRI_SET_TABLE_3_VALID(0)

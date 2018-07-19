#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

#define SQCB_WRITE_BACK_P t2_s2s_sqcb_write_back_info
struct req_tx_phv_t p;
struct req_tx_ah_size_t d;

%%

.align
req_tx_load_ah_size_process:

    phvwr.e           CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, ah_size), d.size

    CAPRI_SET_TABLE_1_VALID(0)

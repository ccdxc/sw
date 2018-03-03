#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct sqcb1_t d;

#define TO_STAGE_T struct req_tx_to_stage_t 

%%

.align
req_tx_sqcb1_hdr_template_addr_fetch_process:

    // Pass header template address to stage 4 to load hdr_template_addr
    CAPRI_GET_STAGE_4_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.header_template_addr, d.header_template_addr)

    CAPRI_SET_TABLE_1_VALID(0)

    nop.e
    nop

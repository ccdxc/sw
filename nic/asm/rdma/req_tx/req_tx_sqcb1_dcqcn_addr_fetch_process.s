#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct sqcb1_t d;
struct req_tx_sqcb1_process_k_t k;

#define TO_STAGE_T struct req_tx_to_stage_t 

%%

.align
req_tx_sqcb1_dcqcn_addr_fetch_process:

    // Pass header template address to stage 2 and 3 to load dcqcn cb in stage 4
    CAPRI_GET_STAGE_2_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.header_template_addr, d.header_template_addr)

    CAPRI_GET_STAGE_3_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.header_template_addr, d.header_template_addr)

    // Pass congestion_mgmt_enable flag to stage 3 and 4
    CAPRI_GET_STAGE_2_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.congestion_mgmt_enable, 1)
    CAPRI_GET_STAGE_3_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.congestion_mgmt_enable, 1)
    CAPRI_GET_STAGE_4_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, sq.congestion_mgmt_enable, 1)

    CAPRI_SET_TABLE_2_VALID(0)

    nop.e
    nop

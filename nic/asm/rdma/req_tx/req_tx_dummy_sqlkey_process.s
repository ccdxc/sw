#include "capri.h"
#include "req_tx.h"
#include "rqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct req_tx_s3_t0_k k;

#define IN_TO_S_P to_s3_sq_to_stage

#define SGE_TO_LKEY_T struct req_tx_sge_to_lkey_info_t
#define K_HEADER_TEMPLATE_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, header_template_addr_sbit0_ebit7, header_template_addr_sbit24_ebit31)
#define SQCB_WRITE_BACK_P t2_s2s_sqcb_write_back_info

%%
    .param  req_tx_sqlkey_invalidate_process
    .param  req_tx_dcqcn_enforce_process

.align
req_tx_dummy_sqlkey_process:

    /*
     * Load sqlkey in tables 0 and 1 for invalidation. 
     * Its done in both tables coz next phv should be able to see invalidation in bypass-cache.
     */
    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, SGE_TO_LKEY_T, sge_index, 0)
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqlkey_invalidate_process, k.common_te0_phv_table_addr)

    CAPRI_GET_TABLE_1_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, SGE_TO_LKEY_T, sge_index, 1)
    CAPRI_GET_TABLE_1_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqlkey_invalidate_process, k.common_te0_phv_table_addr)

    // Skip DCQCN stage if congestion-mgmt is not enabled.
    bbeq    CAPRI_KEY_FIELD(IN_TO_S_P, congestion_mgmt_enable), 0, dcqcn_mpu_only 
    phvwr   CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, non_packet_wqe), 1 // BD-slot

    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_tx_dcqcn_enforce_process, r0)
    nop.e
    nop

dcqcn_mpu_only:
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dcqcn_enforce_process, r0)
    nop.e
    nop

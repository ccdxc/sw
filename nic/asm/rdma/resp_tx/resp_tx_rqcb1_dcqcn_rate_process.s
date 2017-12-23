#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_tx_phv_t p;
struct rqcb1_t d;
struct resp_tx_rqcb1_process_k_t k;

#define RAW_TABLE_PC        r3
#define DCQCN_CB_ADDR       r4
%%

    .param      resp_tx_dcqcn_rate_process
    .param      resp_tx_dcqcn_timer_process

resp_tx_rqcb1_dcqcn_rate_process:

    add     DCQCN_CB_ADDR, HDR_TEMPLATE_T_SIZE_BYTES, d.header_template_addr, HDR_TEMP_ADDR_SHIFT 
    bbeq    k.args.timer_event_process, 1 , timer_event

    CAPRI_GET_TABLE_0_K(resp_tx_phv_t, r2) // BD-slot
    // Load dcqcn_cb MPU only. HBM addr will be loaded from stage 3 to run algo and update rate.
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_tx_dcqcn_rate_process)
    CAPRI_NEXT_TABLE_I_READ(r2, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_0_BITS, RAW_TABLE_PC, DCQCN_CB_ADDR)
    nop.e
    nop

timer_event:
    // Load dcqcn_cb MPU only. HBM addr will be loaded from stage 3 to update timer params.
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_tx_dcqcn_timer_process)
    CAPRI_NEXT_TABLE_I_READ(r2, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_0_BITS, RAW_TABLE_PC, DCQCN_CB_ADDR)
    
    nop.e
    nop

#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s2_t0_k k;

#define GLOBAL_FLAGS        r7

%%

    .param  resp_rx_rqwqe_process
    .param  resp_rx_rqwqe_wrid_process

.align
resp_rx_rqwqe_mpu_only_process:

    add              GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS 

    //CAPRI_NEXT_TABLE0_READ_PC_CE(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, \
    //                             resp_rx_rqwqe_wrid_process, resp_rx_rqwqe_process, K_WQE_PTR, c1)
    ARE_ALL_FLAGS_SET(c1, GLOBAL_FLAGS, RESP_RX_FLAG_WRITE|RESP_RX_FLAG_IMMDT) 
    CAPRI_GET_TABLE_0_K(resp_rx_phv_t, r7)
    bcf              [c1], rqwqe_wrid
    nop // BD Slot

    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_PC_E(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqwqe_process)

rqwqe_wrid:
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_PC_E(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqwqe_wrid_process)

#include "capri.h"                                                                                   
#include "resp_rx.h"                                                             
#include "rqcb.h"                                                                                
#include "common_phv.h"                                                                              
                                                                                                     
struct resp_rx_phv_t p;                                                                                                                                                                                    
struct rqcb1_t d;                                                                                    
struct resp_rx_ecn_process_k_t k;                                                                                  
                                                                                                                   
#define RAW_TABLE_PC r5                                                                              
                                                                                                     
%%                                                              
    .param    resp_rx_dcqcn_cnp_process
                                                                                                     
.align                                                         
resp_rx_rqcb1_cnp_process:                                                                       
    // Load dcqcn_cb mpu-only (HBM addr will be loaded in stage 3) to process cnp based on dcqcn algo.
    CAPRI_GET_TABLE_2_K(resp_rx_phv_t, r4)                                  
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_dcqcn_cnp_process)
    add     r2, d.header_template_addr, HDR_TEMPLATE_T_SIZE_BYTES //dcqcn_cb addr
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_0_BITS, RAW_TABLE_PC, r2)

    nop.e
    nop

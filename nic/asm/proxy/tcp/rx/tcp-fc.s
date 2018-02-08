/*
 *  Implements the CC stage of the RxDMA P4+ pipeline
 */


#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-constants.h"  
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s5_t0_tcp_rx_k.h"
    
struct phv_ p;
struct s5_t0_tcp_rx_k_ k;
struct s5_t0_tcp_rx_tcp_fc_d d;
    
%%
    .param          tcp_rx_write_serq_stage_start
    .param          tcp_rx_write_arq_stage_start
    .param          tcp_rx_write_l7q_stage_start
    .param          ARQRX_QIDXR_BASE
    .align  
tcp_rx_fc_stage_start:
    CAPRI_SET_DEBUG_STAGE4_7(p.s6_s2s_debug_stage4_7_thread, CAPRI_MPU_STAGE_5, CAPRI_MPU_TABLE_0)
    sne         c1, k.common_phv_write_arq, r0
    bcf         [c1], tcp_cpu_rx

    // TODO : FC stage has to be implemented

    
    /*
     * c1 = ooo received, store allocated page and descr in d
     *
     * c2 = pkt received with ooo buffer allocated, use stored
     * page and descr
     */
    sne         c1, k.common_phv_ooo_rcv, r0
    sne         c2, k.common_phv_ooo_in_rx_q, r0
    bcf         [!c1 & !c2], flow_fc_process_done

    tblwr.c1    d.page, k.to_s5_page
    tblwr.c1    d.descr, k.to_s5_descr
    tblwr.c1    d.l7_descr, k.to_s5_l7_descr

    phvwr.c2    p.to_s6_descr, d.descr
    phvwr.c2    p.to_s6_page, d.page
    phvwr.c2    p.s6_t2_s2s_l7_descr, d.l7_descr

flow_fc_process_done:   
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                tcp_rx_write_serq_stage_start, k.common_phv_qstate_addr,
                TCP_TCB_WRITE_SERQ_OFFSET, TABLE_SIZE_512_BITS)
   
    sne     c1, k.common_phv_l7_proxy_en, r0
    bcf     [c1], tcp_l7_rx
    nop
    nop.e
    nop

tcp_cpu_rx:

#ifdef DO_NOT_USE_CPU_SEM
    addui       r5, r0, hiword(ARQRX_QIDXR_BASE)
    addi        r5, r5, loword(ARQRX_QIDXR_BASE)
    CPU_ARQRX_QIDX_ADDR(0, r3, r5)

    CAPRI_NEXT_TABLE_READ_OFFSET(1,
                                 TABLE_LOCK_EN,
                                 tcp_rx_write_arq_stage_start,
                                 r3,
                                 0,
                                 TABLE_SIZE_512_BITS)
#else
    CPU_ARQ_SEM_IDX_INC_ADDR(RX, 0, r3)

    CAPRI_NEXT_TABLE_READ(1, 
                          TABLE_LOCK_DIS,
                          tcp_rx_write_arq_stage_start,
                          r3,
                          TABLE_SIZE_64_BITS)

#endif
    b           flow_fc_process_done
    nop

tcp_l7_rx:
    CAPRI_NEXT_TABLE_READ_OFFSET(2,
                                 TABLE_LOCK_EN,
                                 tcp_rx_write_l7q_stage_start,
                                 k.common_phv_qstate_addr,
                                 TCP_TCB_WRITE_L7Q_OFFSET, 
                                 TABLE_SIZE_512_BITS)
    nop.e
    nop
   

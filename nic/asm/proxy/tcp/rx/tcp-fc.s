/*
 *  Implements the CC stage of the RxDMA P4+ pipeline
 */


#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-constants.h"  
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t0_tcp_rx_k.h"
    
struct phv_ p;
struct s4_t0_tcp_rx_k_ k;
struct s4_t0_tcp_rx_tcp_fc_d d;
    
%%
    .param          tcp_rx_write_serq_stage_start
    .param          tcp_rx_write_arq_stage_start
    .param          tcp_rx_write_l7q_stage_start
    .param          ARQRX_QIDXR_BASE
    .param          ARQRX_BASE
    .align  
tcp_rx_fc_stage_start:
    CAPRI_SET_DEBUG_STAGE4_7(p.s5_s2s_debug_stage4_7_thread, CAPRI_MPU_STAGE_4, CAPRI_MPU_TABLE_0)
    sne         c1, k.common_phv_write_arq, r0
    bcf         [c1], tcp_cpu_rx
    phvwr       p.to_s5_xrq_base, d.serq_base
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

    tblwr.c1    d.page, k.to_s4_page
    tblwr.c1    d.descr, k.to_s4_descr
    tblwr.c1    d.l7_descr, k.to_s4_l7_descr

    phvwr.c2    p.to_s5_descr, d.descr
    phvwr.c2    p.to_s5_page, d.page
    phvwr.c2    p.s5_t2_s2s_l7_descr, d.l7_descr

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

    phvwri      p.to_s5_xrq_base, ARQRX_BASE

    CPU_ARQRX_QIDX_ADDR(0, r3, ARQRX_QIDXR_BASE)

    CAPRI_NEXT_TABLE_READ_OFFSET(1,
                                 TABLE_LOCK_EN,
                                 tcp_rx_write_arq_stage_start,
                                 r3,
                                 0,
                                 TABLE_SIZE_512_BITS)

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
   

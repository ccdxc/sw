#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s0_t0_ooq_tcp_tx_k.h"

struct phv_ p;
struct s0_t0_ooq_tcp_tx_k_ k;
struct s0_t0_ooq_tcp_tx_ooq_tcp_txdma_load_stage0_d d;

%%
    .align
    .param tcp_ooq_txdma_load_rx2tx_slot
    .param tcp_ooq_txdma_process_next_descr_addr
tcp_ooq_load_qstate:
     seq c1, d.pi_0, d.ci_0
     bcf [c1], tcp_ooq_load_qstate_do_nothing
     phvwrpair p.common_phv_fid, k.p4_txdma_intr_qid, \
               p.common_phv_qstate_addr, k.p4_txdma_intr_qstate_addr

     seq c2, d.num_entries, d.curr_index
     sne c3, d.num_entries, r0
     sne c4, d.current_descr_qbase_addr, r0
     seq c5, d.ooq_proc_in_progress, 1
     bcf [c2 & c3 & c4 & c5], tcp_ooq_load_qstate_completed_work_for_this_request 
     nop
     bcf [!c2 & c3 & c4 & c5], tcp_ooq_load_qstate_process_next_pkt_descr
     nop
     bcf [!c5], tcp_ooq_load_qstate_process_new_request
     nop.e
     nop

tcp_ooq_load_qstate_do_nothing:
    phvwri          p.p4_intr_global_drop, 1
    nop.e
    nop 

tcp_ooq_load_qstate_completed_work_for_this_request:
    tblwr d.ooq_proc_in_progress, r0
    tblwr d.current_descr_qbase_addr, r0
    tblwr d.curr_index, r0
    tblwr d.num_entries, r0
    tblwr.f d.num_pkts, r0
    phvwri          p.p4_intr_global_drop, 1
    nop.e
    nop 

tcp_ooq_load_qstate_process_next_pkt_descr:
    add r1, d.current_descr_qbase_addr, r0
    sll r2, d.curr_index, 3
    add r3, d.curr_index, 1
    phvwr p.to_s5_curr_index, r3
    add r2, r1, r2
    //phvwr p.to_s3_curr_rnmdr_addr, r2
    //Launch dummy table for stage1
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tcp_ooq_txdma_process_next_descr_addr, r2, TABLE_SIZE_128_BITS) 
    nop.e
    nop

tcp_ooq_load_qstate_process_new_request: 
    // New request
    add r1, d.ooq_per_flow_ring_base, r0
    and r2, d.ci_0, TCP_OOO_RX2TX_ENTRY_RING_SIZE-1
    sll r2, r2, TCP_OOO_RX2TX_ENTRY_SHIFT
    add r2, r1, r2 
    //launch table with this address
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tcp_ooq_txdma_load_rx2tx_slot, r2, TABLE_SIZE_128_BITS) 
    tblwr d.ooq_proc_in_progress, 1  
    nop.e
    nop


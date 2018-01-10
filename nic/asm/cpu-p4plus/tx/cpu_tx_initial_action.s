#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_tx_initial_action_k k;
struct cpu_tx_initial_action_d d;

%%
    .param cpu_tx_read_asq_ci_start
    .align
cpu_tx_stage0_start:
    /* Check if PI == CI */
    seq     c1, d.{u.cpu_tx_initial_action_d.ci_0}.hx, d.{u.cpu_tx_initial_action_d.pi_0}.hx
    b.c1    cpu_tx_initial_action_done

#ifdef CAPRI_IGNORE_TIMESTAMP
    add     r6, r0, r0
#endif
    phvwr   p.quiesce_pkt_trlr_timestamp, r6.wx
    
    phvwr   p.common_phv_qstate_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
    phvwr   p.common_phv_qid, k.p4_txdma_intr_qid
    phvwr   p.common_phv_flags, d.u.cpu_tx_initial_action_d.flags

table_read_asq_cindex:
    add     r3, d.{u.cpu_tx_initial_action_d.asq_base}.dx, d.{u.cpu_tx_initial_action_d.ci_0}.hx, NIC_ASQ_ENTRY_SIZE_SHIFT 
    phvwr   p.to_s1_asq_ci_addr, r3
    tbladd  d.{u.cpu_tx_initial_action_d.ci_0}.hx, 1

    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, cpu_tx_read_asq_ci_start, r3, TABLE_SIZE_64_BITS)

    seq     c1, d.{u.cpu_tx_initial_action_d.ci_0}.hx, d.{u.cpu_tx_initial_action_d.pi_0}.hx
    b.!c1   cpu_tx_initial_action_done

    /*
     * Ring doorbell to set CI if pi == ci
     */

     /* address will be in r4 */
    CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, LIF_CPU)
    
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.p4_txdma_intr_qid, CPU_SCHED_RING_ASQ, d.{u.cpu_tx_initial_action_d.ci_0}.hx)
    memwr.dx        r4, r3
   
cpu_tx_initial_action_done:
    nop.e
    nop


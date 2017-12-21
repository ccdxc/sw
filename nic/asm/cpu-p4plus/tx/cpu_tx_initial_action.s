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
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_OPERAND_DEBUG(r6)
    phvwr   p.quiesce_pkt_trlr_timestamp, r6.wx
    
    phvwr   p.common_phv_qstate_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
    phvwr   p.common_phv_qid, k.p4_txdma_intr_qid
    phvwr   p.common_phv_flags, d.u.cpu_tx_initial_action_d.flags

table_read_asq_cindex:
    phvwr   p.to_s2_asq_cidx, d.{u.cpu_tx_initial_action_d.ci_0}.hx
    add     r3, d.{u.cpu_tx_initial_action_d.asq_base}.dx, d.{u.cpu_tx_initial_action_d.ci_0}.hx, NIC_ASQ_ENTRY_SIZE_SHIFT 
    phvwr   p.to_s1_asq_ci_addr, r3

cpu_tx_initial_action_done:
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, cpu_tx_read_asq_ci_start, r3, TABLE_SIZE_64_BITS)
    nop.e
    nop


#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_tx_initial_action_k k;
struct cpu_tx_initial_action_cpu_tx_initial_action_d d;

%%
    .param cpu_tx_read_asq_ci_start
    .align
cpu_tx_stage0_start:
    CAPRI_CLEAR_TABLE_VALID(0)
    addi    r4, r0, 0x1
    add     r6, r4, r0
	
    // set the qid in the phv so that the rest of the stages can use it
    //phvwr		p.common_phv_fid, k.p4_rxdma_intr_qid
    add     r4, r0, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
    phvwr   p.common_phv_qstate_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}

table_read_asq_cindex:
    CAPRI_OPERAND_DEBUG(d.{ci_0}.hx)
    CAPRI_OPERAND_DEBUG(d.{asq_base}.wx)

    phvwr   p.to_s2_asq_cidx, d.{ci_0}.hx
    add     r3, d.{asq_base}.wx, d.{ci_0}.hx, NIC_ASQ_ENTRY_SIZE_SHIFT 
    phvwr   p.to_s1_asq_ci_addr, r3

cpu_tx_initial_action_done:
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, cpu_tx_read_asq_ci_start, r3, TABLE_SIZE_64_BITS)
    nop.e
    nop


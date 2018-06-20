#include "INGRESS_p.h"
#include "ingress.h"
#include "cpu-table.h"

#define r_cpu_hdr_addr   r2
#define r_dot1q_hdr_addr r3
        
struct phv_ p;
struct cpu_tx_read_asq_descr_k k;
struct cpu_tx_read_asq_descr_d d;

%%
    .param CPU_TX_DOT1Q_HDR_OFFSET
    .param cpu_tx_read_cpu_hdr_start
    .param cpu_tx_read_l2_vlan_hdr_start
    .align
cpu_tx_read_asq_descr_start:
    CAPRI_CLEAR_TABLE_VALID(0)
    CAPRI_OPERAND_DEBUG(d.{u.read_asq_descr_d.L0}.wx)
    // CPU hdr should be on the top of the page
    add     r_cpu_hdr_addr, r0, d.{u.read_asq_descr_d.A0}.dx
    add     r_cpu_hdr_addr, r_cpu_hdr_addr, d.{u.read_asq_descr_d.O0}.wx
    phvwr   p.to_s4_page_addr, r_cpu_hdr_addr 
    phvwr   p.to_s4_len, d.{u.read_asq_descr_d.L0}.wx

    addi    r_dot1q_hdr_addr, r_cpu_hdr_addr, CPU_TX_DOT1Q_HDR_OFFSET

table_read_cpu_hdr:
    CAPRI_NEXT_TABLE_READ(0, 
                          TABLE_LOCK_DIS,
                          cpu_tx_read_cpu_hdr_start,
                          r_cpu_hdr_addr,
                          TABLE_SIZE_128_BITS)
table_read_vlan_hdr:
    CAPRI_NEXT_TABLE_READ(1,
                          TABLE_LOCK_DIS,
                          cpu_tx_read_l2_vlan_hdr_start,
                          r_dot1q_hdr_addr,
                          TABLE_SIZE_512_BITS)
    nop.e
    nop


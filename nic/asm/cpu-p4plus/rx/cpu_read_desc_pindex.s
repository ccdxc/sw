#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_rx_read_cpu_desc_k k;
struct cpu_rx_read_cpu_desc_d d;

%%
    .param cpu_rx_desc_alloc_start
    .param CPU_RX_DPR_TABLE_BASE
    .param cpu_rx_semaphore_full_drop_error
    .align

cpu_rx_read_desc_pindex_start:
    CAPRI_CLEAR_TABLE1_VALID
    add     r4, r0, d.{u.read_cpu_desc_d.desc_pindex}.wx
    andi    r4, r4, ((1 << ASIC_CPU_RX_DPR_RING_SHIFT) - 1)
    phvwr   p.s2_t1_s2s_desc_pindex, r4 
    seq     c1, d.u.read_cpu_desc_d.desc_pindex_full, 1
    //bcf     [c1], cpu_rx_semaphore_full_error
    bcf     [c1], cpu_rx_semaphore_full_error_new

table_read_desc_alloc:
    addui   r3, r0, hiword(CPU_RX_DPR_TABLE_BASE)
    addi    r3, r3, loword(CPU_RX_DPR_TABLE_BASE)
    sll     r1, r4, CPU_RX_DPR_TABLE_ENTRY_SIZE_SHFT
    add     r1, r1, r3

    phvwri  p.common_te1_phv_table_lock_en, 0

    // Read 512 bits if the pindex is byte aligned
    smeqb   c1, r4, 0x7, 0x0
    phvwri.c1  p.common_te1_phv_table_raw_table_size, TABLE_SIZE_512_BITS 
    phvwri.!c1 p.common_te1_phv_table_raw_table_size, TABLE_SIZE_64_BITS 
    
    phvwri  p.common_te1_phv_table_pc, cpu_rx_desc_alloc_start[33:6]
    phvwr   p.common_te1_phv_table_addr, r1 
    phvwri  p.app_header_table1_valid, 1
	nop.e
	nop 

cpu_rx_semaphore_full_error:
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    add    r3, CPU_CB_WRITE_ARQRX_OFFSET, k.common_phv_qstate_addr
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN,
                         cpu_rx_semaphore_full_drop_error,
                         r3,
                         TABLE_SIZE_512_BITS)
    nop.e
    nop

cpu_rx_semaphore_full_error_new:
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    add r7, CPU_CB_WRITE_ARQRX_OFFSET, k.common_phv_qstate_addr
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, CPU_RX_CB_SEM_FULL_OFFSET, 1)
    phvwri p.common_phv_dpr_sem_full_drop, 1
    phvwri.e  p.p4_intr_global_drop, 1
    nop

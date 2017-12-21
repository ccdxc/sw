#include "INGRESS_p.h"
#include "ingress.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_tx_read_asq_descr_k k;
struct cpu_tx_read_asq_descr_d d;

%%
    .param cpu_tx_read_cpu_hdr_start 
    .align
cpu_tx_read_asq_descr_start:
    CAPRI_CLEAR_TABLE_VALID(0)
    CAPRI_OPERAND_DEBUG(d.{u.read_asq_descr_d.L0}.wx)
    add     r3, r0, d.{u.read_asq_descr_d.A0}.dx
    add     r3, r3, d.{u.read_asq_descr_d.O0}.wx
    phvwr   p.to_s5_page_addr, r3 
    phvwr   p.to_s5_len, d.{u.read_asq_descr_d.L0}.wx

table_read_cpu_hdr:
    // CPU hdr should be on the top of the page
    CAPRI_NEXT_TABLE_READ(0, 
                          TABLE_LOCK_DIS,
                          cpu_tx_read_cpu_hdr_start,
                          r3,
                          TABLE_SIZE_128_BITS)
    nop.e
    nop


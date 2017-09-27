#include "INGRESS_p.h"
#include "ingress.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_tx_read_asq_ci_k k;
struct cpu_tx_read_asq_ci_d d;

%%
    .param cpu_tx_asq_consume_start
    .param cpu_tx_read_asq_descr_start
    .align
cpu_tx_read_asq_ci_start:

    CAPRI_OPERAND_DEBUG(d.{u.read_asq_ci_d.desc_addr}.dx);

    phvwr   p.to_s5_asq_desc_addr, d.{u.read_asq_ci_d.desc_addr}.dx

    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, cpu_tx_asq_consume_start)

    add     r3, d.{u.read_asq_ci_d.desc_addr}.dx, NIC_DESC_ENTRY_0_OFFSET
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, cpu_tx_read_asq_descr_start, r3, TABLE_SIZE_512_BITS)
    
    nop.e
    nop

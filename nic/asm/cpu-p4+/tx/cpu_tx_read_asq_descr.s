#include "INGRESS_p.h"
#include "ingress.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_tx_read_asq_descr_k k;
struct cpu_tx_read_asq_descr_d d;

%%
    .param cpu_tx_write_pkt_start
    .align
cpu_tx_read_asq_descr_start:
    CAPRI_CLEAR_TABLE_VALID(0)
    CAPRI_OPERAND_DEBUG(d.{u.read_asq_descr_d.A0}.dx)
    CAPRI_OPERAND_DEBUG(d.{u.read_asq_descr_d.O0}.wx)
    CAPRI_OPERAND_DEBUG(d.{u.read_asq_descr_d.L0}.wx)
    phvwr   p.to_s3_page_addr, d.{u.read_asq_descr_d.A0}.dx
    phvwr   p.to_s3_offset, d.{u.read_asq_descr_d.O0}.wx
    phvwr   p.to_s3_len, d.{u.read_asq_descr_d.L0}.wx

    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, cpu_tx_write_pkt_start)
    nop.e
    nop


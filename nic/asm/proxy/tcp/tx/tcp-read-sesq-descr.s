/*
 *	Implements the TX stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-sched.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct tcp_tx_read_sesq_descr_k k;
struct tcp_tx_read_sesq_descr_read_sesq_descr_d d;

%%
        .align
        .param          tcp_tx_process_stage3_start

tcp_tx_sesq_read_descr_stage2_start:

        CAPRI_OPERAND_DEBUG(d.A0)
        CAPRI_OPERAND_DEBUG(d.O0)
        CAPRI_OPERAND_DEBUG(d.L0)
        phvwr           p.to_s3_addr, d.{A0}.dx
        phvwr           p.to_s3_offset, d.{O0}.wx
        phvwr           p.to_s3_len, d.{L0}.wx

        CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                            tcp_tx_process_stage3_start,
                            k.common_phv_qstate_addr,
                            TCP_TCB_TX_OFFSET, TABLE_SIZE_512_BITS)
        nop.e
        nop

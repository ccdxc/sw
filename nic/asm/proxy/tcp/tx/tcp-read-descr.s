/*
 *	Implements the TX stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct s3_t0_read_descr_k k;
struct s3_t0_read_descr_read_descr_d d;

%%
    .align
    .param          tcp_retx_process_start

tcp_tx_read_descr_start:

    CAPRI_OPERAND_DEBUG(d.A0)
    CAPRI_OPERAND_DEBUG(d.O0)
    CAPRI_OPERAND_DEBUG(d.L0)
    phvwr           p.to_s4_addr, d.{A0}.dx
    phvwr           p.to_s4_offset, d.{O0}.wx
    phvwr           p.to_s4_len, d.{L0}.wx
    phvwr           p.t0_s2s_next_addr, d.{next_addr}.dx

    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_retx_process_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_RETX_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop

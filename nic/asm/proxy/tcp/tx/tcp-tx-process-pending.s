/*
 *	Implements the rx2tx shared extra state read stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-sched.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct phv_ p;
struct tcp_tx_s2_t0_k k;
	
%%
    .align
    .param          tcp_tx_process_stage3_start

tcp_tx_process_pending_stage2_start:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_tx_process_stage3_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_TX_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop


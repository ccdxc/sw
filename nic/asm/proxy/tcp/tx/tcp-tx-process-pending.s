/*
 *	Implements the rx2tx shared extra state read stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct phv_ p;
struct tcp_tx_pending_k k;
struct tcp_tx_pending_pending_d d;
	
%%
    .align
    .param          tcp_tx_read_descr_start
    .param          tcp_tx_s3_bubble_start

tcp_tx_process_pending_start:
    seq             c1, k.common_phv_pending_sesq, 1
    seq             c2, k.common_phv_pending_asesq, 1
    bcf             [!c1 & !c2], launch_pending
    CAPRI_NEXT_TABLE_READ_e(0, TABLE_LOCK_DIS,
                        tcp_tx_read_descr_start, k.to_s2_sesq_desc_addr, TABLE_SIZE_512_BITS)
    nop
launch_pending:
    seq             c3, d.retx_next_desc, r0
    b.c3            launch_pending_empty_retx
    CAPRI_NEXT_TABLE_READ_e(0, TABLE_LOCK_DIS,
                        tcp_tx_read_descr_start, d.retx_next_desc, TABLE_SIZE_512_BITS)
    nop
launch_pending_empty_retx:
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, tcp_tx_s3_bubble_start)
    nop.e
    nop


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
struct s1_t1_tcp_tx_k k;
struct s1_t1_tcp_tx_pending_d d;
	
%%
    .align
    .param          tcp_tx_read_descr_start
    .param          tcp_tx_s2_bubble_start

tcp_tx_process_pending_start:
    CAPRI_CLEAR_TABLE_VALID(1)
    seq             c3, d.retx_next_desc, r0
    bcf             [c3], launch_pending_empty_retx
    add             r3, d.retx_next_desc, NIC_DESC_ENTRY_0_OFFSET
    CAPRI_NEXT_TABLE_READ_e(0, TABLE_LOCK_DIS,
                        tcp_tx_read_descr_start, r3, TABLE_SIZE_512_BITS)
    nop
launch_pending_empty_retx:
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, tcp_tx_s2_bubble_start)
    nop.e
    nop


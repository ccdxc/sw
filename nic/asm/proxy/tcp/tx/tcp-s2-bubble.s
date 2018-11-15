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
#include "INGRESS_s2_t0_tcp_tx_k.h"

struct phv_ p;
struct s2_t0_tcp_tx_k_ k;

%%
    .align
    .param          tcp_retx_process_start
    .param          tcp_clean_retx_process_start

tcp_tx_s2_bubble_start:
    bbeq            k.common_phv_pending_retx_cleanup[0], 1, tcp_tx_s2_bubble_launch_clean_retx

tcp_tx_s2_bubble_launch_retx:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_retx_process_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_RETX_OFFSET, TABLE_SIZE_512_BITS)

    nop.e
    nop

tcp_tx_s2_bubble_launch_clean_retx:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_clean_retx_process_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_RETX_OFFSET, TABLE_SIZE_512_BITS)

    nop.e
    nop

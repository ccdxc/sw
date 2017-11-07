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
struct tcp_tx_read_descr_k k;

%%
    .align
    .param          tcp_tx_process_start

tcp_tx_s3_bubble_start:

    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_tx_process_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_TX_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop

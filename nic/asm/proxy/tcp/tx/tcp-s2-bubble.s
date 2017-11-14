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
struct s2_t0_pending_k k;

%%
    .align
    .param          tcp_tx_s3_bubble_start

tcp_tx_s2_bubble_start:
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, tcp_tx_s3_bubble_start)

    nop.e
    nop

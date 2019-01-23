/*
 *	Implements the rx2tx shared extra state read stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t0_tcp_tx_k.h"

struct phv_ p;
struct s1_t0_tcp_tx_k_ k;
struct s1_t0_tcp_tx_read_rx2tx_extra_d d;

%%
    .align
    .param      tcp_tx_process_pending_start
    .param      tcp_tx_s2_bubble_start

tcp_tx_read_rx2tx_shared_extra_idle_start:
    // HACK: Force a timer of 100 ticks
#ifdef HW
    phvwrpair       p.to_s4_snd_cwnd, d.snd_cwnd, \
                        p.to_s4_rto, TCP_RTO_MIN_TICK
#else
    // Set it to 1 tick in simulation so timer immediately expires
    // on stepping it
    phvwrpair       p.to_s4_snd_cwnd, d.snd_cwnd, \
                        p.to_s4_rto, 1
#endif
    /*
     * For pending_ack_send, we need to launch the bubble stage
     */
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, tcp_tx_s2_bubble_start)

tcp_tx_rx2tx_extra_end:
    nop.e
    nop


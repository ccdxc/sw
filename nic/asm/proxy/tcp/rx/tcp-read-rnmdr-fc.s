/* Reads rnmdr pindex and cindex, calculates the number of free entries in rnmdr
** and updates rnmdr_size in PHV so that at stage 5 window calculation logic can
** include this in the receive window calculation.
*/

#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t1_tcp_rx_k.h"

struct phv_ p;
struct s2_t3_tcp_rx_read_rnmdr_fc_d d;

%%
    .align
tcp_rx_read_rnmdr_fc:
    phvwr           p.to_s5_rnmdr_size_valid, 1
    add             r1, d.{rnmdr_cidx}.wx, (1 << ASIC_RNMDPR_BIG_RING_SHIFT)
    sub             r1, r1, d.{rnmdr_pidx}.wx
    and             r1, r1, ((1 << ASIC_RNMDPR_BIG_RING_SHIFT) - 1)

    phvwr           p.to_s5_rnmdr_size, r1
    CAPRI_CLEAR_TABLE_VALID(3)
    nop.e
    nop

/*
 *	Read tcp flags for the packet from the descriptor
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
struct s2_t2_tcp_tx_k k;
struct s2_t2_tcp_tx_read_tcp_flags_d d;

%%
    .align

tcp_tx_read_tcp_flags_start:
    CAPRI_CLEAR_TABLE_VALID(2)

    CAPRI_OPERAND_DEBUG(d.tcp_flags)
    smeqb           c1, d.tcp_flags, TCPHDR_FIN, TCPHDR_FIN
    phvwri.c1       p.common_phv_fin, 1
    phvwri.c1       p.tcp_header_flags, TCPHDR_ACK | TCPHDR_FIN
    nop.e
    nop

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
#include "INGRESS_s2_t1_tcp_tx_k.h"

struct phv_ p;
struct s2_t1_tcp_tx_k_ k;
struct s2_t1_tcp_tx_read_tcp_flags_d d;

%%
    .align

tcp_tx_read_tcp_flags_start:
    CAPRI_CLEAR_TABLE_VALID(1)

    CAPRI_OPERAND_DEBUG(d.tcp_flags)
    /*
     * FIN only works in bypass barco mode for now.
     */
    seq             c1, k.common_phv_debug_dol_bypass_barco, 0
    b.c1            read_tcp_flags_end

    smeqb           c1, d.tcp_flags, TCPHDR_FIN, TCPHDR_FIN
    phvwri.c1       p.common_phv_fin, 1
    phvwri.c1       p.tcp_header_flags, TCPHDR_ACK | TCPHDR_FIN

    smeqb           c1, d.tcp_flags, TCPHDR_RST, TCPHDR_RST
    phvwri.c1       p.tcp_header_flags, TCPHDR_ACK | TCPHDR_RST
    phvwr.c1        p.common_phv_rst, 1
    phvwr.c1        p.tx2rx_rst_sent, 1
read_tcp_flags_end:
    nop.e
    nop

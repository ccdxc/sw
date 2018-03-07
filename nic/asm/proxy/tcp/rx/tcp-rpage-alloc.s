/*
 *    Implements the page allocation stage of the RxDMA P4+ pipeline
 */

#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t2_tcp_rx_k.h"

struct phv_ p;
struct s4_t2_tcp_rx_k_ k;
struct s4_t2_tcp_rx_rpage_alloc_d d;

%%
    
tcp_rx_rpage_alloc_start:
    CAPRI_CLEAR_TABLE2_VALID

    CAPRI_OPERAND_DEBUG(d.page)
    sne             c1, k.common_phv_ooo_rcv, r0
    phvwr.c1        p.to_s5_page, d.page
    phvwr.e         p.to_s6_page, d.page
    nop

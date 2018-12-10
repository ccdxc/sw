/*
 *  Implements the RX stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t3_tcp_rx_k.h"

struct phv_ p;
struct s4_t3_tcp_rx_k_ k;
struct s4_t3_tcp_rx_d d;

%%
    .align
    .param tcp_ooo_processing_launch_dummy2
tcp_ooo_processing_launch_dummy1:
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, tcp_ooo_processing_launch_dummy2)
    nop.e
    nop




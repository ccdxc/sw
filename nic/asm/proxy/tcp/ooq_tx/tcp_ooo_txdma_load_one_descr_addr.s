#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s3_t0_ooq_tcp_tx_k.h"

struct phv_ p;
struct s3_t0_ooq_tcp_tx_k_ k;
struct s3_t0_ooq_tcp_tx_ooq_tcp_txdma_load_descr_addr_d d;

%%
    .align
    .param tcp_ooo_txdma_load_one_descr 
tcp_ooo_txdma_load_one_descr_addr:
    phvwr p.to_s4_curr_rnmdr_addr, d.one_descr_addr
    phvwr p.{tcp_app_header_ts...tcp_app_header_prev_echo_ts}, d.one_descr_addr 
    nop.e
    nop

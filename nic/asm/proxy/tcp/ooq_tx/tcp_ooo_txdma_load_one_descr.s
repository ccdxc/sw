#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t0_ooq_tcp_tx_k.h"

struct phv_ p;
struct s4_t0_ooq_tcp_tx_k_ k;
struct s4_t0_ooq_tcp_tx_ooq_tcp_txdma_load_one_descr_d d;

%%
    .align
    .param tcp_ooo_txdma_load_qstate_and_update
tcp_ooo_txdma_load_one_descr:
    //write tcp flags from descr scratch to phv
    add r1, k.common_phv_qstate_addr, r0 
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tcp_ooo_txdma_load_qstate_and_update, r1, TABLE_SIZE_512_BITS) 

    phvwr p.tcp_app_header_p4plus_app_id, d.tcp_app_header_p4plus_app_id
    phvwr p.tcp_app_header_from_ooq_txdma, d.tcp_app_header_from_ooq_txdma
    phvwri p.tcp_app_header_from_ooq_txdma, 1
    phvwr p.{tcp_app_header_num_sack_blocks...tcp_app_header_urgentPtr}, d.{tcp_app_header_num_sack_blocks...tcp_app_header_urgentPtr}
    phvwr p.{tcp_app_header_ts...tcp_app_header_prev_echo_ts}, k.to_s4_curr_rnmdr_addr 
    nop.e
    nop

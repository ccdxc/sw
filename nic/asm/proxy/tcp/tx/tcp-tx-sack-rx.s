/*
 *    Implements the TX stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t1_tcp_tx_k.h"

struct phv_ p;
struct s4_t1_tcp_tx_k_ k;
struct s4_t1_tcp_tx_sack_rx_d d;

%%
    .align

/*
 * r1 = phv ptr
 * r2 = num sack options
 * r3 = next sack option #
 * r4 = found (return val)
 *
 * c5 = ts_opt_present
 */
#define r_phv_ptr r1
#define r_num_sack_opts r2
#define r_next_sack_opt r3
#define r_ret_val r3

#define c_ts_opt c5

tcp_tx_sack_rx:
    // TODO : handle tsopt also present
    smeqb           c_ts_opt, d.tcp_opt_flags, TCP_OPT_FLAG_TIMESTAMPS, \
                        TCP_OPT_FLAG_TIMESTAMPS
    add.c_ts_opt    r_phv_ptr, offsetof(struct phv_, tcp_sack_opt_start_seq1), 0
    add.!c_ts_opt   r_phv_ptr, offsetof(struct phv_, tcp_ts_opt_ts_val), 0
    add             r_num_sack_opts, 0, 0
    add             r_next_sack_opt, 0, 1

add_sack_option:
    bal             r7, add_another_sack_option
    sne             c1, r4, 0
    b.c1            add_sack_option
    nop

    seq             c1, r_num_sack_opts, 0
    b.c1            add_sack_option_end
    nop

    // TODO : handle tsopt present vs not
    phvwr.c_ts_opt  p.tcp_sack_opt_kind, TCPOPT_SACK
    phvwr.!c_ts_opt p.tcp_ts_opt_kind, TCPOPT_SACK
    add             r5, 2, r_num_sack_opts, 3
    phvwr.c_ts_opt  p.tcp_sack_opt_len, r5
    phvwr.!c_ts_opt p.tcp_ts_opt_len, r5
    phvwr           p.to_s5_sack_opt_len, r5

add_sack_option_end:
    CAPRI_CLEAR_TABLE_VALID(1)
    nop.e
    nop

add_another_sack_option:
    add             r4, 0, 0
sack_add_q0:
    sne             c1, d.tail_index0, 0
    seq.c1          c1, d.q0_pos, r_next_sack_opt
    b.!c1           sack_add_q1
    nop
    add             r_num_sack_opts, r_num_sack_opts, 1
    add             r_next_sack_opt, r_next_sack_opt, 1
    phvwrp          r_phv_ptr, 0, 32, d.start_seq0
    sub             r_phv_ptr, r_phv_ptr, 32
    phvwrp          r_phv_ptr, 0, 32, d.end_seq0
    sub             r_phv_ptr, r_phv_ptr, 64
    jr              r7
    add             r4, 0, 1
sack_add_q1:
    sne             c1, d.tail_index1, 0
    seq.c1          c1, d.q1_pos, r_next_sack_opt
    b.!c1           sack_add_q2
    nop
    add             r_num_sack_opts, r_num_sack_opts, 1
    add             r_next_sack_opt, r_next_sack_opt, 1
    phvwrp          r_phv_ptr, 0, 32, d.start_seq1
    sub             r_phv_ptr, r_phv_ptr, 32
    phvwrp          r_phv_ptr, 0, 32, d.end_seq1
    sub             r_phv_ptr, r_phv_ptr, 64
    jr              r7
    add             r4, 0, 1
sack_add_q2:
    sne             c1, d.tail_index2, 0
    seq.c1          c1, d.q2_pos, r_next_sack_opt
    b.!c1           sack_add_q3
    nop
    add             r_num_sack_opts, r_num_sack_opts, 1
    add             r_next_sack_opt, r_next_sack_opt, 1
    phvwrp          r_phv_ptr, 0, 32, d.start_seq2
    sub             r_phv_ptr, r_phv_ptr, 32
    phvwrp          r_phv_ptr, 0, 32, d.end_seq2
    sub             r_phv_ptr, r_phv_ptr, 64
    jr              r7
    add             r4, 0, 1

sack_add_q3:
    sne             c1, d.tail_index3, 0
    seq.c1          c1, d.q3_pos, r_next_sack_opt
    b.!c1           sack_add_end
    nop
    add             r_num_sack_opts, r_num_sack_opts, 1
    add             r_next_sack_opt, r_next_sack_opt, 1
    phvwrp          r_phv_ptr, 0, 32, d.start_seq3
    sub             r_phv_ptr, r_phv_ptr, 32
    phvwrp          r_phv_ptr, 0, 32, d.end_seq3
    sub             r_phv_ptr, r_phv_ptr, 64
    jr              r7
    add             r4, 0, 1

sack_add_end:
    jr              r7
    nop

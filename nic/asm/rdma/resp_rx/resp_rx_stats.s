#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_stats_process_k_t k;
struct rqcb4_t d;

#define STATS_INFO_T struct resp_rx_stats_info_t

#define GLOBAL_FLAGS r7

%%

.align
resp_rx_stats_process:

    sub              r1, k.args.bubble_count, 1
    bne              r1, r0, bubble_down_to_next_stage
    
    add              GLOBAL_FLAGS, r0, k.global.flags //BD slot

    tblsadd          d.num_bytes, k.to_stage.s5.stats.bytes
    tblsadd          d.num_pkts, 1

    crestore         [c6, c5, c4, c3, c2, c1], GLOBAL_FLAGS, (RESP_RX_FLAG_RING_DBELL | RESP_RX_FLAG_ACK_REQ | RESP_RX_FLAG_INV_RKEY | RESP_RX_FLAG_ATOMIC_FNA | RESP_RX_FLAG_ATOMIC_CSWAP | RESP_RX_FLAG_READ_REQ)

    tblsadd.c6       d.num_ring_dbell, 1
    tblsadd.c5       d.num_ack_requested, 1
    tblsadd.c4       d.num_send_msgs_inv_rkey, 1
    tblsadd.c3       d.num_atomic_fna_msgs, 1
    tblsadd.c2       d.num_atomic_cswap_msgs, 1
    tblsadd.c1       d.num_read_req_msgs, 1

    bcf              [c6 | c4 | c3 | c2 | c1], done

    ARE_ALL_FLAGS_SET(c6, GLOBAL_FLAGS, RESP_RX_FLAG_IMMDT|RESP_RX_FLAG_SEND) //BDF
    tblsadd.c6       d.num_send_msgs_imm_data, 1
    ARE_ALL_FLAGS_SET(c6, GLOBAL_FLAGS, RESP_RX_FLAG_IMMDT|RESP_RX_FLAG_WRITE)
    tblsadd.c6       d.num_write_msgs_imm_data, 1

    //send messages without inv_rkey and imm_data
    crestore         [c6, c5, c4, c3, c2, c1], GLOBAL_FLAGS, (RESP_RX_FLAG_INV_RKEY | RESP_RX_FLAG_IMMDT | RESP_RX_FLAG_WRITE | RESP_RX_FLAG_SEND | RESP_RX_FLAG_MIDDLE | RESP_RX_FLAG_FIRST)
    //send & & !inv_rkey !imm_data & !middle & !first 
    setcf            c7, [!c6 & !c5 & c3 & !c2 & !c1]
    tblsadd.c7       d.num_send_msgs, 1

    //write messages without imm_data
    //write & !imm_data & !middle & !first 
    setcf            c7, [!c5 & c4 & !c2 & !c1]
    tblsadd.c7       d.num_write_msgs, 1

    IS_ANY_FLAG_SET(c6, GLOBAL_FLAGS, RESP_RX_FLAG_ONLY | RESP_RX_FLAG_FIRST | RESP_RX_FLAG_READ_REQ | RESP_RX_FLAG_ATOMIC_CSWAP | RESP_RX_FLAG_ATOMIC_FNA)
    tblwr.c6         d.num_pkts_in_cur_msg, 1
    tblsadd.!c6      d.num_pkts_in_cur_msg, 1

    //peak
    add              r4, r0, d.max_pkts_in_any_msg
    sslt             c6, r4, d.num_pkts_in_cur_msg, r0
    tblwr.c6         d.max_pkts_in_any_msg, d.num_pkts_in_cur_msg

done:

    CAPRI_SET_TABLE_1_VALID(0)
    nop.e
    nop

bubble_down_to_next_stage:
    
    CAPRI_GET_TABLE_1_ARG_NO_RESET(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD(r4, STATS_INFO_T, bubble_count, r1)
    seq     c1, r1, 1

    CAPRI_GET_TABLE_1_K(resp_rx_phv_t, r4)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_C(r4, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, c1)

    nop.e
    nop


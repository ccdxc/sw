#include "capri.h"
#include "req_rx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_rx_phv_t p;
struct req_rx_s7_t3_k k;
struct sqcb5_t d;

#define IN_P    to_s7_stats_info

#define GLOBAL_FLAGS r7
#define SQCB5_ADDR   r3
#define STATS_PC     r6

#define MASK_16 16
#define MASK_32 32

%%

.align
req_rx_stats_process:

    // Pin stats process to stage 7
    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_7
    bcf              [!c1], bubble_to_next_stage

    add              GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS //BD Slot

    crestore         [c6, c5, c4, c3, c2, c1], GLOBAL_FLAGS, (REQ_RX_FLAG_RDMA_FEEDBACK | REQ_RX_FLAG_ATOMIC_AETH | REQ_RX_FLAG_ACK | REQ_RX_FLAG_READ_RESP | REQ_RX_FLAG_FIRST | REQ_RX_FLAG_ONLY)

    bcf              [c6], done
    tblmincri.c6     d.num_feedback, MASK_16, 1 //BD Slot

    tbladd           d.num_bytes, CAPRI_KEY_FIELD(to_s7_stats_info, pyld_bytes)
    tblmincri        d.num_pkts, MASK_32, 1

    tblmincri.c5     d.num_atomic_ack, MASK_16, 1
    tblmincri.c4     d.num_ack, MASK_16, 1

    bcf              [c5 | c4], done
    setcf            c6, [c3 & c2 & c1] //BD Slot

    tblmincri.c2     d.num_read_resp_pkts, MASK_32, 1
    tblmincri.c6     d.num_read_resp_msgs, MASK_16, 1

    //ignoring this update for ACK and ATOMIC_ACK and tracking only for read responses
    IS_ANY_FLAG_SET(c6, GLOBAL_FLAGS, REQ_RX_FLAG_ONLY | REQ_RX_FLAG_FIRST)
    tblwr.c6         d.num_pkts_in_cur_msg, 1
    tblmincri.!c6    d.num_pkts_in_cur_msg, MASK_16, 1

    //peak
    add              r4, r0, d.max_pkts_in_any_msg
    sslt             c6, r4, d.num_pkts_in_cur_msg, r0
    tblwr.c6         d.max_pkts_in_any_msg, d.num_pkts_in_cur_msg

done:

    nop.e
    nop

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_6
    bcf           [!c1], exit

    //invoke the same routine, but with valid d[]
    //using static config, stage-7/table-3 is set as memory_only - to improve on latency
    CAPRI_GET_TABLE_3_K(req_rx_phv_t, r7) //BD Slot
    SQCB5_ADDR_GET(SQCB5_ADDR)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, SQCB5_ADDR)

exit:
    nop.e
    nop



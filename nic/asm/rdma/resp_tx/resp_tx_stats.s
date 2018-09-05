#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct resp_tx_s7_t3_k k;
struct rqcb4_t d;

#define IN_P    to_s7_stats_info

#define GLOBAL_FLAGS r7
#define RQCB4_ADDR   r3
#define STATS_PC     r6

#define MASK_16 16
#define MASK_32 32

%%

.align
resp_tx_stats_process:

    // Pin stats process to stage 7
    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_7
    bcf              [!c1], bubble_to_next_stage

    add              GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS //BD Slot

    tbladd           d.num_bytes, CAPRI_KEY_FIELD(to_s7_stats_info, pyld_bytes)
    tblmincri        d.num_pkts, MASK_32, 1

    crestore         [c7, c6, c5, c4, c3, c2, c1], GLOBAL_FLAGS, (RESP_TX_FLAG_ONLY | RESP_TX_FLAG_FIRST | RESP_TX_FLAG_MIDDLE | RESP_TX_FLAG_LAST | RESP_TX_FLAG_READ_RESP | RESP_TX_FLAG_ATOMIC_RESP | RESP_TX_FLAG_ACK)

    tblmincri.c1     d.num_acks, MASK_32, 1
    tblmincri.c2     d.num_atomic_resp_msgs, MASK_16, 1

    tblmincri.c3     d.num_read_resp_pkts, MASK_32, 1
    setcf            c1, [c7 | c6]
    setcf            c3, [c3 & c1]
    tblmincri.c3     d.num_read_resp_msgs, MASK_16, 1

    IS_ANY_FLAG_SET(c6, GLOBAL_FLAGS, RESP_TX_FLAG_ONLY | RESP_TX_FLAG_FIRST | RESP_TX_FLAG_ATOMIC_RESP)
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
    CAPRI_GET_TABLE_3_K(resp_tx_phv_t, r7) //BD Slot
    RQCB4_ADDR_GET(RQCB4_ADDR)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, RQCB4_ADDR)

exit:
    nop.e
    nop



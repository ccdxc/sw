#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t0_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s2_t0_k_ k;
struct s2_t0_nvme_req_tx_nscb_process_d d;

#define RR_SESSION_ID               r2
#define RR_SESSION_ID_TOP_BITS      r2[7:6]
#define RR_SESSION_ID_BOTTOM_BITS   r2[5:0]
#define LOG_MAX_SESSIONS            8
#define MAX_SESSION_BITS            (1 << LOG_MAX_SESSIONS)
#define MAX_SESSION_LONG_INTS       (MAX_SESSION_BITS/CAPRI_SIZEOF_U64_BITS)
#define SESSION_BITMAP_BOUNDARY     256

%%
    .param  nvme_txsessprodcb_base
    .param  nvme_tx_resourcecb_addr
    .param  nvme_req_tx_sessprodcb_process
    .param  nvme_req_tx_resourcecb_process

.align
nvme_req_tx_nscb_process:
    // assumption is that next session to be served is in the 
    // table field rr_session_id_served and it is within the 
    // limits of 0 to 255. If the bit corresponding to that session id is
    // not set (i.e., the session is not active), bitmap is searched for
    // the next bit that is set (with wrap-around).
    add     RR_SESSION_ID, r0, d.rr_session_id_served

    // track the count
    add     r5, r0, r0

loop:
    // go to the long int boundary of session id to be served
    add     r1, offsetof(d, valid_session_bitmap), RR_SESSION_ID_TOP_BITS, CAPRI_LOG_SIZEOF_U64_BITS

    // read the long int under examination
    tblrdp  r3, r1, 0, CAPRI_SIZEOF_U64_BITS

    // look for a bit that is set starting from the bit corresponding to
    // rr_session_id_served. for the first iteration, it could start from a 
    // non-zero offset within a long int, then onwards RR_SESSION_ID becomes 
    // a multiple of long-int
    ffsv    r4, r3, RR_SESSION_ID

    // check if we found a bit in this long int
    seq     c1, r4, -1
    bcf     [!c1], sess_found
    add.!c1 RR_SESSION_ID, r4, RR_SESSION_ID_TOP_BITS, CAPRI_LOG_SIZEOF_U64_BITS  
        // sessid = base of this long int + bit position found

    // increment the long ints examined
    add     r5, r5, 1

    // move the ptr to next long int and set offset to 0
    add     RR_SESSION_ID, CAPRI_SIZEOF_U64_BITS, RR_SESSION_ID_TOP_BITS, CAPRI_LOG_SIZEOF_U64_BITS

    // did we loop thru all sessions ?
    // note that the count should reach one more than MAX_SESSION_LONG_INTS as
    // we might have started with a partial long int and hence need to revisit
    // that from the beginning after visiting all other long ints.
    sle     c1, r5, MAX_SESSION_LONG_INTS
    bcf     [!c1], sess_not_found 
    // did we reach wrap-around ?
    seq     c2, RR_SESSION_ID, MAX_SESSION_BITS//BD Slot
    b       loop
    cmov    RR_SESSION_ID, c2, 0, RR_SESSION_ID//BD Slot

sess_found:
    //calculate sessprodcb address = 
    // txsessprodcb_base + ((ns->sess_prodcb_start + rr_session_id) << sizeof(txsessprodcb))
    add     r5, RR_SESSION_ID, d.sess_prodcb_start

    // store the next session to be served
    mincr   RR_SESSION_ID, LOG_MAX_SESSIONS, 1
    tblwr.f d.rr_session_id_served, RR_SESSION_ID

    addui   r6, r0, hiword(nvme_txsessprodcb_base)
    addi    r6, r6, loword(nvme_txsessprodcb_base)

    add     r6, r6, r5, LOG_TXSESSPRODCB_SIZE

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS,
                              CAPRI_TABLE_SIZE_0_BITS,
                              nvme_req_tx_sessprodcb_process,
                              r6)

    addui   r6, r0, hiword(nvme_tx_resourcecb_addr)
    addi    r6, r6, loword(nvme_tx_resourcecb_addr)

    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_0_BITS,
                                nvme_req_tx_resourcecb_process,
                                r6) //Exit Slot
                            

exit:
    CAPRI_SET_TABLE_0_VALID_CE(c0, 0)
    nop //Exit Slot

sess_not_found:
    //TODO: raise an alram. We do not have even a single valid session 
    // for this namespace !!
    b       exit
    phvwr   p.p4_intr_global_drop, 1        //BD Slot

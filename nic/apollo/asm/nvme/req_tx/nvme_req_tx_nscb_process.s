#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t0_nvme_req_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s2_t0_nvme_req_tx_k_ k;
struct s2_t0_nvme_req_tx_nscb_process_d d;

#define DMA_CMD_BASE                r7
#define RR_SESSION_ID               r2
#define RR_SESSION_ID_TOP_BITS      r2[7:6]
#define RR_SESSION_ID_BOTTOM_BITS   r2[5:0]
#define LOG_MAX_SESSIONS            8
#define MAX_SESSION_BITS            (1 << LOG_MAX_SESSIONS)
#define MAX_SESSION_LONG_INTS       (MAX_SESSION_BITS/CAPRI_SIZEOF_U64_BITS)
#define SESSION_BITMAP_BOUNDARY     256

%%
    .param  nvme_txsessprodcb_base
    .param  nvme_resourcecb_addr
    .param  nvme_req_tx_sessprodcb_process
    .param  nvme_req_tx_resourcecb_process
    .param  nvme_req_tx_sqe_prp_process

.align
nvme_req_tx_nscb_process:
    // assumption is that next session to be served is in the 
    // table field rr_session_id_to_be_served and it is within the 
    // limits of 0 to 255. If the bit corresponding to that session id is
    // not set (i.e., the session is not active), bitmap is searched for
    // the next bit that is set (with wrap-around).
    add     RR_SESSION_ID, r0, d.rr_session_id_to_be_served

    // track the count
    add     r5, r0, r0

sess_loop:
    // go to the long int boundary of session id to be served
    add     r1, offsetof(d, valid_session_bitmap), RR_SESSION_ID_TOP_BITS, CAPRI_LOG_SIZEOF_U64_BITS

    // read the long int under examination
    tblrdp  r3, r1, 0, CAPRI_SIZEOF_U64_BITS

    // look for a bit that is set starting from the bit corresponding to
    // rr_session_id_to_be_served. for the first iteration, it could start from a 
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
    b       sess_loop
    cmov    RR_SESSION_ID, c2, 0, RR_SESSION_ID//BD Slot

sess_found:
    //calculate sessprodcb address = 
    // txsessprodcb_base + ((ns->sess_prodcb_start + rr_session_id) << sizeof(txsessprodcb))
    add     r5, RR_SESSION_ID, d.sess_prodcb_start

    // store the next session to be served
    mincr   RR_SESSION_ID, LOG_MAX_SESSIONS, 1
    tblwr.f d.rr_session_id_to_be_served, RR_SESSION_ID

    addui   r6, r0, hiword(nvme_txsessprodcb_base)
    addi    r6, r6, loword(nvme_txsessprodcb_base)

    add     r6, r6, r5, LOG_TXSESSPRODCB_SIZE

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS,
                              CAPRI_TABLE_SIZE_0_BITS,
                              nvme_req_tx_sessprodcb_process,
                              r6)

    addui   r6, r0, hiword(nvme_resourcecb_addr)
    addi    r6, r6, loword(nvme_resourcecb_addr)

    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS,
                              CAPRI_TABLE_SIZE_0_BITS,
                              nvme_req_tx_resourcecb_process,
                              r6)
                            
    phvwrpair   p.cmd_ctxt_log_lba_size, d.log_lba_size, \
                p.cmd_ctxt_log_host_page_size, k.to_s2_info_log_host_page_size
  
    phvwrpair   p.pdu_ctxt0_log_lba_size, d.log_lba_size, \
                p.pdu_ctxt0_log_host_page_size, k.to_s2_info_log_host_page_size

    //logic to download prps

    //calculate prp1_offset
    add         r1, r0, k.t0_s2s_sqe_to_nscb_info_prp1
    mincr       r1, k.to_s2_info_log_host_page_size, r0
    phvwrpair   p.cmd_ctxt_prp1_offset, r1, \
                p.{cmd_ctxt_key_index...cmd_ctxt_sec_key_index}, d.{key_index...sec_key_index}
    phvwrpair   p.pdu_ctxt0_prp1_offset, r1, \
                p.{pdu_ctxt0_key_index...pdu_ctxt0_sec_key_index}, d.{key_index...sec_key_index}

    //store backend_ns_id so that PDU is sent with this NSID
    phvwr       p.pdu_ctxt0_nsid, d.backend_ns_id

    // add it to nlb * lba_size
    sll         r2, k.to_s2_info_nlb, d.log_lba_size
    add         r2, r1, r2
    
    // find out number of prps
    srl         r3, r2, k.to_s2_info_log_host_page_size   
    mincr       r2, k.to_s2_info_log_host_page_size, r0
    seq         c1, r2, r0
    add.!c1     r3, r3, 1

    // r3 now has number of prps and r4 has equivalient bytes
    sll         r4, r3, LOG_NUM_PRP_BYTES

    // if the number of prps are <=2, we only need one dma
    phvwrpair   p.pdu_ctxt0_num_prps, r3, \
                p.pdu_ctxt0_session_id, r5
    sle         c1, r3, 2
    bcf         [c1], one_dma 
    phvwrpair   p.cmd_ctxt_session_id, r5, \
                p.cmd_ctxt_num_prps, r3 //BD Slot

    // since number of prps are more than 2, we have at least one prp list, 
    
    //calculate prp2_offset
    add         r1, r0, k.t0_s2s_sqe_to_nscb_info_prp2
    mincr       r1, k.to_s2_info_log_host_page_size, r0
    
    //subtract from host page size
    sll         r2, 1, k.to_s2_info_log_host_page_size
    sub         r2, r2, r1
    // now r2 has the number of bytes in prp list 

    //take off prp1 worth of bytes
    sub         r4, r4, 1, LOG_NUM_PRP_BYTES

    //did the prp list cover the left over prp bytes ?
    sle         c1, r4, r2
    bcf         [c1], two_dmas
    // only prp1 is valid in base cmd
    // store prp pointers in pdu_ctxt in little endian format
    phvwr       p.prp1_ptr, k.{t0_s2s_sqe_to_nscb_info_prp1}.dx //BD Slot

three_dmas:
    sub         r2, r2, 1, LOG_NUM_PRP_BYTES
    sub         r4, r4, r2

    DMA_CMD_BASE_GET(DMA_CMD_BASE, prp2_src_dma)
    DMA_HOST_MEM2MEM_SRC_SETUP(DMA_CMD_BASE, r2, k.t0_s2s_sqe_to_nscb_info_prp2)

    //use table 2 to read the last ptr in the prp list, which points to
    //the rest of the prps
    phvwr       p.t2_s2s_nscb_to_sqe_prp_info_prp3_dma_bytes, r4
    add         r6, r2, k.t0_s2s_sqe_to_nscb_info_prp2
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS,
                              CAPRI_TABLE_SIZE_64_BITS,
                              nvme_req_tx_sqe_prp_process,
                              r6)

    phvwrpair.e p.to_s5_info_prp1_dma_bytes, 8, \
                p.to_s5_info_prp2_dma_bytes, r2
    // prp1_dma_valid = 1, prp2_dma_valid = 1, prp3_dma_valid = 1
    phvwrpair   p.{to_s5_info_prp1_dma_valid...to_s5_info_prp3_dma_valid}, 7, \
                p.to_s5_info_prp3_dma_bytes, r4     //Exit Slot
    
two_dmas:
    DMA_CMD_BASE_GET(DMA_CMD_BASE, prp2_src_dma)
    DMA_HOST_MEM2MEM_SRC_SETUP(DMA_CMD_BASE, r4, k.t0_s2s_sqe_to_nscb_info_prp2)
    phvwr.e     p.to_s5_info_prp1_dma_bytes, 8
    // prp1_dma_valid = 1, prp2_dma_valid = 1, prp3_dma_valid = 0
    phvwrpair   p.{to_s5_info_prp1_dma_valid...to_s5_info_prp3_dma_valid}, 6, \
                p.to_s5_info_prp2_dma_bytes, r4     //Exit Slot


one_dma:
    // if only one prp dma command needed, always copy prp1/prp2, 
    // anyway num_prps field in pdu_ctxt going to guide whether prp2 is valid
    // or not
    // store prp pointers in pdu_ctxt in little endian format
    phvwr       p.prp1_ptr, k.{t0_s2s_sqe_to_nscb_info_prp1}.dx
    phvwr.e     p.prp2_ptr, k.{t0_s2s_sqe_to_nscb_info_prp2}.dx
    // prp1_dma_valid = 1, prp2_dma_valid = 0, prp3_dma_valid = 0
    phvwrpair   p.{to_s5_info_prp1_dma_valid...to_s5_info_prp3_dma_valid}, 4, \
                p.to_s5_info_prp1_dma_bytes, r4     //Exit Slot

    
exit:
    CAPRI_SET_TABLE_0_VALID_CE(c0, 0)
    nop //Exit Slot

sess_not_found:
    //TODO: raise an alram. We do not have even a single valid session 
    // for this namespace !!
    b       exit
    phvwr   p.p4_intr_global_drop, 1        //BD Slot

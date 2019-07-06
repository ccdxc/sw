#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t0_nvme_req_rx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s1_t0_nvme_req_rx_k_ k;
struct s1_t0_nvme_req_rx_rqe_process_d d;

#define IN_P             t0_s2s_rqcb_to_rqe_info
#define OUT_P            t0_s2s_rqe_to_pdu_hdr_info
#define TO_S_WB_P        to_s5_info
#define K_SEG_OFFSET     CAPRI_KEY_FIELD(IN_P, segment_offset)
#define K_PDU_OFFSET     CAPRI_KEY_FIELD(IN_P, pdu_offset)

#define RX_HDR_LEN       28

#define CURR_PDU_LEN     r1
#define SEG_PDU_OFFSET   r2
#define PDU_HDR_ADDR     r6

%%
    .param  nvme_req_rx_pdu_hdr_process

.align
nvme_req_rx_rqe_process:

    /* This wqe has page_ptr, len and offset
       Here, the goal is to check if we have received the PDU header completely, 
       including CH, PSH and header digest, which is the first 28 bytes of the PDU (in case of both Response capsule PDU and C2H data)
       Check whether or not (len - segment_offset + pdu_offset) >= 28 and pass this info to the next stage
       Load pdu_hdr (page_ptr + page_offset + seg_offset)
     */
     
    CAPRI_SET_FIELD2(TO_S_WB_P, page_ptr, d.A0)
    CAPRI_SET_FIELD2(OUT_P, rqe_len, d.L0)
    CAPRI_SET_FIELD_RANGE2(OUT_P, segment_offset, pdu_offset, CAPRI_KEY_RANGE(IN_P, segment_offset, pdu_offset))

    add         PDU_HDR_ADDR, d.A0, d.O0        
    add         PDU_HDR_ADDR, PDU_HDR_ADDR, K_SEG_OFFSET
 
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS,
                                CAPRI_TABLE_SIZE_512_BITS,
                                nvme_req_rx_pdu_hdr_process,
                                PDU_HDR_ADDR) // Exit Slot

#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s3_t0_nvme_sessprexts_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s3_t0_nvme_sessprexts_tx_k_ k;
struct s3_t0_nvme_sessprexts_tx_cb_writeback_process_d d;

#define CURR_LBA_OFFSET             r1
#define NEW_LBA_OFFSET              r2
#define LOG_MAX_NUM_LBAS_IN_PAGE    r3
#define LBA_SIZE                    r4
#define DMA_CMD_BASE                r5
#define ABS_LBA                     r5

#define DB_ADDR                     r5
#define DB_DATA                     r6

#define F_FIRST_PAGE                c1
#define F_LAST_LBA                  c2
#define F_END_OF_PAGE               c3
#define F_FIRST_LBA_IN_PAGE         c4

#define K_SLBA                  k.t0_s2s_pdu_ctxt_to_writeback_info_slba
#define K_NLB                   k.t0_s2s_pdu_ctxt_to_writeback_info_nlb
#define K_LOG_LBA_SIZE          k.t0_s2s_pdu_ctxt_to_writeback_info_log_lba_size
#define K_PRP1_OFFSET           k.t0_s2s_pdu_ctxt_to_writeback_info_prp1_offset
#define K_LOG_HOST_PAGE_SIZE    k.t0_s2s_pdu_ctxt_to_writeback_info_log_host_page_size
#define K_PDU_CTXT_P            k.to_s3_info_pdu_ctxt_ptr
#define K_PAGE_PTR              k.to_s3_info_page_ptr
#define K_PAGE_PTR_LE           k.{to_s3_info_page_ptr}.dx

#define D_NXT_LBA_OFFSET    d.nxt_lba_offset

%%
    .param      nvme_sessprexts_tx_ip_desc_process

.align
nvme_sessprexts_tx_cb_writeback_process:
    // assumption is that: in stable state, nxt_lba_offset is 0.
    add         CURR_LBA_OFFSET, r0, D_NXT_LBA_OFFSET
    
    // if (nlb == nxt_lba_offset +1) then it is last lba
    add         NEW_LBA_OFFSET, CURR_LBA_OFFSET, 1
    seq         F_LAST_LBA, NEW_LBA_OFFSET, K_NLB

    sub         LOG_MAX_NUM_LBAS_IN_PAGE, NVME_LOG_MAX_DATA_SIZE_IN_PAGE, K_LOG_LBA_SIZE

    bcf         [!F_LAST_LBA], non_last_lba
    tblmincri   d.wb_r0_busy, 1, 1  //BD Slot

last_lba:
    //for last lba, reset page_ptr, nxt_lba_offset variables to 0
    //increment cindex
    tblwr       d.page_ptr, r0
    tblwr       D_NXT_LBA_OFFSET, r0
    tblmincri.f SESSPREXTSTX_C_INDEX, d.log_num_entries, 1  //Flush

    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_SET_PINDEX, DB_SCHED_WR_EVAL_RING, \
                        K_GLOBAL_LIF, NVME_QTYPE_SESS, DB_ADDR)
    //use the incremented cindex of R0 as the pindex of R1
    CAPRI_SETUP_DB_DATA(K_GLOBAL_QID, SESSPOSTXTS_TX_RING_ID, \
                        SESSPREXTSTX_C_INDEX, DB_DATA) 

    b           post_tbl_updates
    phvwrpair   p.xts_desc_doorbell_address, DB_ADDR.dx, \
                p.xts_desc_doorbell_data, DB_DATA.dx   //BD Slot

non_last_lba:
    //store the new lba offset
    tblwr       D_NXT_LBA_OFFSET, NEW_LBA_OFFSET

    // we store integral multiple of LBAs in each page
    // see if the post-incremented lba needs a new page
    // if so, we need to reset the page_ptr in the cb.
    mincr       NEW_LBA_OFFSET, LOG_MAX_NUM_LBAS_IN_PAGE, 0
    
    seq         F_END_OF_PAGE, NEW_LBA_OFFSET, r0
    cmov        r5, F_END_OF_PAGE, r0, K_PAGE_PTR
    tblwr.f     d.page_ptr, r5  //Flush

    //xts engine was mandating a valid doorbell address even for intermediate
    //lbas. Hence setting up a doorbell to do nothing. This wastes doorbell 
    //machine bandwidth, but we have to live with this as this seems to be a 
    //limitation imposed by barco.

    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_NO_UPDATE, DB_NO_SCHED_WR, \
                        K_GLOBAL_LIF, NVME_QTYPE_SESS, DB_ADDR)
    CAPRI_SETUP_DB_DATA(K_GLOBAL_QID, SESSPOSTXTS_TX_RING_ID, \
                        r0, DB_DATA) 
    phvwrpair   p.xts_desc_doorbell_address, DB_ADDR.dx, \
                p.xts_desc_doorbell_data, DB_DATA.dx   //BD Slot
    //fall thru

post_tbl_updates:
    // see which prp start byte of curr lba falls into
    sll         r5, CURR_LBA_OFFSET, K_LOG_LBA_SIZE
    add         r5, K_PRP1_OFFSET, r5
    srl         r6, r5, K_LOG_HOST_PAGE_SIZE

    // see which prp last byte of curr lba falls into
    //TBD: see scope for optimization
    sll         LBA_SIZE, 1, K_LOG_LBA_SIZE
    add         r7, r5, LBA_SIZE 
    sub         r7, r7, 1
    srl         r2, r7, K_LOG_HOST_PAGE_SIZE

    // if start_prp == end_prp, LBA resides in a single host page, 
    // else it is stradled across two host pages. 
    // For now, assume one LBA can't stradle across more than 2 host pages
    seq         c4, r6, r2

    // calculate offset of end byte of lba in the prp
    mincr       r7, K_LOG_HOST_PAGE_SIZE, r0
    //convert offsets to number of bytes (by adding 1)
    add         r7, r7, 1
    add         r2, r0, LBA_SIZE
    sub.!c4     r2, LBA_SIZE, r7

    phvwrpair.!c4   p.to_s4_info_prp2_bytes, r7, \
                    p.to_s4_info_prp2_valid, 1

    //calculate offset of start byte of lba in the prp
    mincr       r5, K_LOG_HOST_PAGE_SIZE, r0
    // is this the very first prp ?
    seq         c5, r6, r0
    sub.c5      r5, r5, K_PRP1_OFFSET

    phvwrpair   p.to_s4_info_prp1_offset, r5, \
                p.to_s4_info_prp1_bytes, r2

    add         r2, K_PDU_CTXT_P, r6, LOG_NUM_PRP_BYTES
    add         r2, r2, NVME_PDU_CTXT_PRP_LIST_OFFSET

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS,
                              CAPRI_TABLE_SIZE_128_BITS,    //2 adjacent prps
                              nvme_sessprexts_tx_ip_desc_process,
                              r2)

    //compute the offset at which current lba needs to be copied to 
    //in the data page
    sll         r6, 1, LOG_MAX_NUM_LBAS_IN_PAGE
    add         r5, CURR_LBA_OFFSET, r0
    slt         F_FIRST_PAGE, r5, r6
    mincr       r5, LOG_MAX_NUM_LBAS_IN_PAGE, 0
    seq         F_FIRST_LBA_IN_PAGE, r5, r0
    sll         r5, r5, K_LOG_LBA_SIZE
    add         r5, K_PAGE_PTR, r5
    add         r5, r5, PKT_DESC_OVERHEAD
    add.F_FIRST_PAGE r5, r5, NVME_O_TCP_CMD_CAPSULE_DGST_EN_HDR_SIZE

    //populate output desc
    //due to PHV being initialized to 0, O0 value is anyway 0 and doesn't 
    //need to be set explicitly
    phvwr       p.op_desc_A0, r5.dx
    phvwr       p.op_desc_L0, LBA_SIZE.wx

    bcf         [!F_FIRST_LBA_IN_PAGE], skip_page_ptr_len_update

    //calculate number of lbas that will eventually go in this page
    //curr_lba_offset is multiple of LOG_MAX_NUM_LBAS_IN_PAGE by the time we
    //enter here
    sub         r5, K_NLB, CURR_LBA_OFFSET
    sle         c5, r5, r6
    cmov        r5, c5, r5, r6
    //r5 should have number of LBAs that will eventually go to this page

    //convert to bytes and any header/trailer etc.
    sll                 r5, r5, K_LOG_LBA_SIZE

    phvwr               p.dgst_one_aol_L0, r5.wx
    add                 r6, K_PAGE_PTR, PKT_DESC_OVERHEAD
    add.F_FIRST_PAGE    r6, r6, NVME_O_TCP_CMD_CAPSULE_DGST_EN_HDR_SIZE
    phvwr               p.dgst_one_aol_A0, r6.dx

    add.F_FIRST_PAGE    r5, r5, NVME_O_TCP_CMD_CAPSULE_DGST_EN_HDR_SIZE
    add.c5              r5, r5, NVME_O_TCP_DDGST_SIZE

    add                 r6, r5, PKT_DESC_OVERHEAD

    //encode the length in page_ptr
    or                  r7, K_PAGE_PTR, r6, NVME_PAGE_DATA_LEN_SHIFT
    phvwr               p.page_ptr_ptr, r7

    phvwr               p.pkt_desc_one_aol_A0, K_PAGE_PTR_LE
    add                 r7, PKT_DESC_OVERHEAD, r0
    phvwrpair           p.pkt_desc_one_aol_O0, r7.wx, \
                        p.pkt_desc_one_aol_L0, r5.wx
    add                 r7, K_PAGE_PTR, PKT_DESC_SCRATCH_OVERHEAD
    
    DMA_CMD_BASE_GET(DMA_CMD_BASE, pkt_desc_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, pkt_desc_one_aol_A0, pkt_desc_one_aol_L0, r7)
    
skip_page_ptr_len_update:

    //get the next lba absolute value
    add     ABS_LBA, K_SLBA, CURR_LBA_OFFSET

    //store it in iv according to customer  req (check)
    phvwr       p.iv_iv_0, ABS_LBA
    phvwr       p.iv_iv_1, r0

    addi.e      r3, r0, HW_CMD_XTS_ENCRYPT
    phvwr       p.xts_desc_command, r3.wx  //Exit Slot

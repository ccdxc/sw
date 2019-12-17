#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct req_rx_s1_t0_k k;

#define RRQWQE_TO_SGE_P t0_s2s_rrqwqe_to_sge_info

#define IN_P       t0_s2s_sqcb1_to_sge_recirc_info
#define IN_TO_S_P  to_s1_recirc_info

#define K_RRQ_CINDEX                  CAPRI_KEY_FIELD(IN_P, rrq_cindex)
#define K_LOG_PAGE_SIZE               CAPRI_KEY_FIELD(IN_P, log_page_size)
#define K_RRQWQE_SGE_LIST_ADDR        CAPRI_KEY_RANGE(IN_TO_S_P, rrqwqe_sge_list_addr_sbit0_ebit23, rrqwqe_sge_list_addr_sbit56_ebit63)
#define K_CUR_SGE_ID                  CAPRI_KEY_FIELD(IN_TO_S_P, cur_sge_id)
#define K_CUR_SGE_OFFSET              CAPRI_KEY_RANGE(IN_TO_S_P, cur_sge_offset_sbit0_ebit7, cur_sge_offset_sbit24_ebit31)
#define K_NUM_VALID_SGES              CAPRI_KEY_FIELD(IN_TO_S_P, num_valid_sges)
#define K_REMAINING_PAYLOAD_BYTES     CAPRI_KEY_RANGE(IN_TO_S_P, remaining_payload_bytes_sbit0_ebit7, remaining_payload_bytes_sbit8_ebit13)

%%
    .param    req_rx_rrqsge_process

.align
req_rx_sqcb1_recirc_sge_process:
    seq            c1, CAPRI_KEY_FIELD(IN_TO_S_P, sge_opt), 1
    add            r1, K_RRQWQE_SGE_LIST_ADDR, K_CUR_SGE_ID, LOG_SIZEOF_SGE_T
    // If SGE length encoding is there, go past to the start of SGES[]
    add.c1            r1, r1, TXWQE_SGE_LEN_ENC_SIZE

    // if log_rq_page_size = 0, rq is in hbm and page boundary check is not needed
    phvwr           CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, end_of_page), 0

    seq             c3, K_LOG_PAGE_SIZE, 0
    sub.c3          r1, r1, 2, LOG_SIZEOF_SGE_T
    bcf             [c3], page_boundary_check_done

    srl            r3, r1, K_LOG_PAGE_SIZE //BD Slot
    add            r4, r1, (RRQWQE_SGE_TABLE_READ_SIZE - 1)
    srl            r4, r4, K_LOG_PAGE_SIZE
    sne            c3, r4, r3
    // move addr_to_load back by sizeof 2 SGE's
    sub.!c3         r1, r1, 2, LOG_SIZEOF_SGE_T
    // sge addr + 16 > end_of_page_addr, move addr_to_load back by sizeof 3 SGE's
    phvwr.c3        CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, end_of_page), 1
    sub.c3          r1, r1, 3, LOG_SIZEOF_SGE_T

page_boundary_check_done:
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_rx_rrqsge_process, r1)

    CAPRI_RESET_TABLE_0_ARG()
    phvwrpair      CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, remaining_payload_bytes), K_REMAINING_PAYLOAD_BYTES, \
                   CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, cur_sge_id), K_CUR_SGE_ID
    phvwrpair.e    CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, cur_sge_offset), K_CUR_SGE_OFFSET, \
                   CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, dma_cmd_eop), CAPRI_KEY_FIELD(IN_TO_S_P, dma_cmd_eop)
    phvwrpair      CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, num_valid_sges), K_NUM_VALID_SGES, \
                   CAPRI_PHV_FIELD(RRQWQE_TO_SGE_P, dma_cmd_start_index), \
                   (REQ_RX_RDMA_PAYLOAD_DMA_CMDS_START + (MAX_PYLD_DMA_CMDS_PER_SGE * 2))


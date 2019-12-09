#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct rdma_atomic_resource_t d;
struct resp_rx_s1_t1_k k;

#define RKEY_INFO_P t1_s2s_rkey_info
#define R_KEY r4
#define KT_BASE_ADDR r6
#define KEY_ADDR r2

#define DMA_CMD_BASE r5
#define DB_ADDR r4
#define DB_DATA r1
#define GLOBAL_FLAGS r7

#define FILL_PATTERN_W      0x01010101
#define FILL_PATTERN_H      0x0101   
#define FILL_PATTERN_B      0x01

#define IN_P        t1_s2s_rqcb_to_read_atomic_rkey_info
#define IN_TO_S_P   to_s1_atomic_info
#define TO_S_WB1_P  to_s5_wb1_info
#define TO_S_CQCB_P to_s6_cqcb_info

#define K_RSQWQE_PTR CAPRI_KEY_RANGE(IN_TO_S_P, rsqwqe_ptr_sbit0_ebit7, rsqwqe_ptr_sbit40_ebit63)
#define K_ATOMIC_RKEY_INFO_VA CAPRI_KEY_RANGE(IN_P, va_sbit0_ebit7, va_sbit8_ebit63)
#define K_LEN CAPRI_KEY_RANGE(IN_P, len_sbit0_ebit7, len_sbit24_ebit31)
#define K_RSQ_PINDEX CAPRI_KEY_RANGE(IN_P, rsq_p_index_sbit0_ebit7, rsq_p_index_sbit8_ebit15)
#define K_PRIV_OPER_ENABLE CAPRI_KEY_FIELD(IN_P, priv_oper_enable)
#define K_RKEY CAPRI_KEY_FIELD(IN_P, r_key)

%%
    .param  rdma_atomic_resource_addr
    .param  rdma_pcie_atomic_base_addr
    .param  resp_rx_recirc_mpu_only_process
    .param  resp_rx_rqrkey_process
    .param  resp_rx_rqcb1_write_back_mpu_only_process
    .param  resp_rx_rqrkey_rsvd_rkey_process

.align
resp_rx_atomic_resource_process:

    // code to grab one out of 32 global atomic resources
    // each resource is identified by a byte in d-vector
    // value of byte 1 means resource taken, 0 means free
    // goal is to find first free byte out of 32 bytes 
    
    add         r1, r0, offsetof(struct rdma_atomic_resource_t, data3)
    addui       r7, r0, FILL_PATTERN_W
    addi        r7, r7, FILL_PATTERN_W
    addi        r6, r0, FILL_PATTERN_W

loop:
    tblrdp      r2, r1, 0, 64
    // r2 has 8 bytes of information
    // check if all are taken in these 8 bytes
    beq         r2, r7, next
    seq         c1, r2[31:0], r6 //BD Slot
    bcf         [c1], check_uw
    add.c1      r1, r1, 32      //BD Slot

check_lw:
    seq         c1, r2[15:0], FILL_PATTERN_H
    bcf         [c1], check_lw_uh
    add.c1      r1, r1, 16      //BD Slot

check_lw_lh:
    seq         c1, r2[7:0], FILL_PATTERN_B
    b           loop_exit
    add.c1      r1, r1, 8       //BD Slot

check_lw_uh:
    seq         c1, r2[23:16], FILL_PATTERN_B
    b           loop_exit
    add.c1      r1, r1, 8       //BD Slot

check_uw:
    seq         c1, r2[47:32], FILL_PATTERN_H
    bcf         [c1], check_uw_uh
    add.c1      r1, r1, 16      //BD Slot

check_uw_lh:
    seq         c1, r2[39:32], FILL_PATTERN_B
    b           loop_exit
    add.c1      r1, r1, 8       //BD Slot

check_uw_uh:
    seq         c1, r2[55:48], FILL_PATTERN_B
    b           loop_exit
    add.c1      r1, r1, 8       //BD Slot
    
next:
    // did we reach end ? 
    // since we are comparing r1 before incrementing by 64, check with 448
    beqi        r1, 448, nak_rnr
    add         r1, r1, 64  //BD Slot
    b           loop
    nop         //BD Slot
    
loop_exit:
    
    //r1 has the offset where byte value is 0, claim the resource
    tblwrp.f    r1, 0, 8, FILL_PATTERN_B
    
    //calculate the DMA address for this byte
    // base_addr + 63 - (r1 >> 3)
    srl         r1, r1, LOG_BITS_PER_BYTE
    sub         r1, 63, r1
    // r1 now has the atomic resource number
    addui       r2, r0, hiword(rdma_atomic_resource_addr)
    addi        r2, r2, loword(rdma_atomic_resource_addr)
    // r2 now has atomic resource base_addr
    add          r2, r2, r1
    // r2 now has DMA address of the byte
    
    addui       r3, r0, hiword(rdma_pcie_atomic_base_addr)
    addi        r3, r3, loword(rdma_pcie_atomic_base_addr)
    // atomic_base_addr + (chosen_pcie_resource << 6)
    add         r3, r3, r1, 6 

    // DMA command to write to acquired atomic resource 
    // NOTE: Below DMA command couldn't be done using mem2mem_phv2mem as it is restricted to only 15 bytes or less,
    // where as this pcie register is of 64 Bytes which need to be written in a single shot. So phv2mem is the only
    // choice here. Note that pcie has a reordering logic where even if READ on pcie register is received before
    // write, it will wait for write to arrive and this ensures ordering.
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_ATOMIC_RESOURCE_WR)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, pcie_atomic, pcie_atomic, r3)
    // DMA commands to read from acquired atomic resource and
    // write to rsqwqe_p
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_ATOMIC_RESOURCE_RD)
    DMA_HBM_MEM2MEM_SRC_SETUP(DMA_CMD_BASE, 8, r3)

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_ATOMIC_RESOURCE_TO_RSQWQE)
    add     r3, K_RSQWQE_PTR, RSQWQE_ORIG_DATA_OFFSET
    DMA_HBM_MEM2MEM_DST_SETUP(DMA_CMD_BASE, 8, r3)

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_RSQWQE)
    DMA_HBM_MEM2MEM_PHV2MEM_SETUP(DMA_CMD_BASE, rsqwqe.read_or_atomic, rsqwqe.psn, K_RSQWQE_PTR)

    add     r3, K_RSQWQE_PTR, RSQWQE_R_KEY_OFFSET
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_RSQWQE_R_KEY_VA)
    DMA_HBM_MEM2MEM_PHV2MEM_SETUP(DMA_CMD_BASE, rsqwqe.atomic.r_key, rsqwqe.atomic.va, r3)
      
    // DMA for releasing atomic resource
    // atomic_release_byte value is 0 as phv gets initialized to 0.
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_RELEASE_ATOMIC_RESOURCE)
    DMA_HBM_MEM2MEM_PHV2MEM_SETUP(DMA_CMD_BASE, s1.atomic_release_byte, s1.atomic_release_byte, r2)
    //DMA_SET_WR_FENCE(DMA_CMD_MEM2MEM_T, DMA_CMD_BASE)

    CAPRI_RESET_TABLE_1_ARG()
    phvwrpair   CAPRI_PHV_FIELD(RKEY_INFO_P, va), K_ATOMIC_RKEY_INFO_VA, \
                CAPRI_PHV_FIELD(RKEY_INFO_P, len), K_LEN

    add     R_KEY, r0, K_RKEY

    seq     c5, R_KEY, RDMA_RESERVED_LKEY_ID
    // c5: rsvd key
    bcf     [!c5], skip_priv_oper
    seq.c5  c5, K_PRIV_OPER_ENABLE, 1 // BD Slot
    // c5: rsvd key + priv oper enabled
    phvwr.!c5   CAPRI_PHV_FIELD(RKEY_INFO_P, rsvd_key_err), 1

skip_priv_oper:
    KT_BASE_ADDR_GET2(KT_BASE_ADDR, r1)
    KEY_ENTRY_ADDR_GET(KEY_ADDR, KT_BASE_ADDR, R_KEY)

    // dma_cmd_start_index: 2, tbl_id: 1, acc_ctrl: ATOMIC_READ, cmdeop: 0
    CAPRI_SET_FIELD_RANGE2(RKEY_INFO_P, dma_cmd_start_index, dma_cmdeop, ((RESP_RX_DMA_CMD_PYLD_BASE << 12) | (TABLE_1 << 9) | (ACC_CTRL_REMOTE_ATOMIC << 1) | 0 ))
    // set write back related params incr_nxt_to_go_token_id: 1
    phvwr       CAPRI_PHV_FIELD(TO_S_WB1_P, incr_nxt_to_go_token_id), 1

    CAPRI_SET_FIELD2(RKEY_INFO_P, user_key, R_KEY[7:0])
    // invoke rqrkey 
    //CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqrkey_process, KEY_ADDR)
    CAPRI_NEXT_TABLE1_READ_PC_C(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqrkey_rsvd_rkey_process, resp_rx_rqrkey_process, KEY_ADDR, c5)

    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_SET_PINDEX, DB_SCHED_WR_EVAL_RING, K_GLOBAL_LIF, K_GLOBAL_QTYPE, DB_ADDR)
    CAPRI_SETUP_DB_DATA(K_GLOBAL_QID, RSQ_RING_ID, K_RSQ_PINDEX, DB_DATA)
    // store db_data in LE format
    phvwr   p.db_data1, DB_DATA.dx

    // DMA for RSQ DoorBell
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_RSQ_DB)
    DMA_HBM_MEM2MEM_PHV2MEM_SETUP(DMA_CMD_BASE, db_data1, db_data1, DB_ADDR)
    DMA_SET_WR_FENCE(DMA_CMD_MEM2MEM_T, DMA_CMD_BASE)
    DMA_SET_END_OF_CMDS(DMA_CMD_MEM2MEM_T, DMA_CMD_BASE)

exit:
    nop.e
    nop

//#if 0
nak_rnr:
    /* When atomic resources are not available,
       fire an mpu only program which will eventually set table 0 valid bit to 1 prior to recirc
    */
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_recirc_mpu_only_process, r0)

    CAPRI_SET_TABLE_1_VALID(0)

    phvwr.e     p.common.p4_intr_recirc, 1
    phvwr       p.common.rdma_recirc_recirc_reason, CAPRI_RECIRC_REASON_ATOMIC_RNR
//#endif

error_completion:

    // set error disable flag 
    // turn on ACK req bit when error disabling QP
    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS
    or          GLOBAL_FLAGS, GLOBAL_FLAGS, RESP_RX_FLAG_ERR_DIS_QP | RESP_RX_FLAG_ACK_REQ
    CAPRI_SET_FIELD_RANGE2(phv_global_common, _ud, _error_disable_qp, GLOBAL_FLAGS)

    phvwr       CAPRI_PHV_FIELD(TO_S_WB1_P, async_or_async_error_event), 1
    phvwr       CAPRI_PHV_FIELD(TO_S_CQCB_P, async_error_event), 1
    phvwrpair   p.s1.eqwqe.code, EQE_CODE_QP_ERR, p.s1.eqwqe.type, EQE_TYPE_QP
    phvwr       p.s1.eqwqe.qid, K_GLOBAL_QID

    phvwr       p.s1.ack_info.syndrome, AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_OP_ERR)
    phvwrpair   p.cqe.status, CQ_STATUS_LOCAL_QP_OPER_ERR, p.cqe.error, 1

    CAPRI_SET_TABLE_1_VALID(0)
    // invoke mpu only program since we are in stage 2, and wb is loaded in stage 5
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)

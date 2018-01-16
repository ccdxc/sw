#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_rx_phv_t p;
struct rdma_atomic_resource_t d;
struct resp_rx_read_atomic_process_k_t k;

#define RKEY_INFO_T struct resp_rx_key_info_t
#define R_KEY r2
#define KT_BASE_ADDR r6
#define KEY_ADDR r2
#define RAW_TABLE_PC r3

#define DMA_CMD_BASE r5
#define DB_ADDR r4
#define DB_DATA r1

#define FILL_PATTERN_W      0x01010101
#define FILL_PATTERN_H      0x0101   
#define FILL_PATTERN_B      0x01
%%
    .param  resp_rx_rqlkey_process
    .param  rdma_atomic_resource_addr
    .param  rdma_pcie_atomic_base_addr

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
    add         r1, r1, 64
    beqi        r1, 512, nak_rnr
    nop         //BD Slot
    
loop_exit:
    
    // skip ring rsq dbell ?
    seq    c2, k.args.skip_rsq_dbell, 1

    //r1 has the offset where byte value is 0, claim the resource
    tblwrp      r1, 0, 8, FILL_PATTERN_B
    
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
    add     r3, k.to_stage.s1.atomic.rsqwqe_ptr, RSQWQE_ORIG_DATA_OFFSET
    DMA_HBM_MEM2MEM_DST_SETUP(DMA_CMD_BASE, 8, r3)
      
    // DMA for releasing atomic resource
    // atomic_release_byte value is 0 as phv gets initialized to 0.
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_RELEASE_ATOMIC_RESOURCE)
    DMA_HBM_MEM2MEM_PHV2MEM_SETUP(DMA_CMD_BASE, atomic_release_byte, atomic_release_byte, r2)

    CAPRI_GET_TABLE_1_ARG(resp_rx_phv_t, r4)
    CAPRI_SET_FIELD_RANGE(r4, RKEY_INFO_T, va, len, k.{args.va...args.len})

    add     R_KEY, r0, k.args.r_key

    KT_BASE_ADDR_GET(KT_BASE_ADDR, r1)
    KEY_ENTRY_ADDR_GET(KEY_ADDR, KT_BASE_ADDR, R_KEY)

    CAPRI_SET_FIELD(r4, RKEY_INFO_T, dma_cmd_start_index, RESP_RX_DMA_CMD_PYLD_BASE)

    // tbl_id: 1, acc_ctrl: ATOMIC_READ, cmdeop: 0, nak_code: REM_ACC_ERR
    CAPRI_SET_FIELD_RANGE(r4, RKEY_INFO_T, tbl_id, nak_code, ((TABLE_1 << 17) | (ACC_CTRL_REMOTE_ATOMIC << 9) | (0 << 8) | (AETH_NAK_SYNDROME_INLINE_GET(NAK_CODE_REM_ACC_ERR))))

    // set write back related params
    // incr_nxt_to_go_token_id: 1, incr_c_index: 0, 
    // skip_pt: 0, invoke_writeback: 1
    CAPRI_SET_FIELD_RANGE(r4, RKEY_INFO_T, incr_nxt_to_go_token_id, invoke_writeback, (1<<3 | 0 << 2 | 0 << 1 | 1))

    CAPRI_GET_TABLE_1_K(resp_rx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_rx_rqlkey_process)
    // Initiate next table lookup with 32 byte Key address (so avoid whether keyid 0 or 1)
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, RAW_TABLE_PC, KEY_ADDR)

    // skip ring rsq dbell if reqd
    bcf    [c2], exit
    DMA_SET_END_OF_CMDS_C(DMA_CMD_MEM2MEM_T, DMA_CMD_BASE, c2) //BD Slot

    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_SET_PINDEX, DB_SCHED_WR_EVAL_RING, k.global.lif, k.global.qtype, DB_ADDR)
    CAPRI_SETUP_DB_DATA(k.global.qid, RSQ_RING_ID, k.args.rsq_p_index, DB_DATA)
    // store db_data in LE format
    phvwr   p.db_data1, DB_DATA.dx

    // DMA for RSQ DoorBell
    //TODO: for some reason, rining doorbell using mem2mem_phv2mem is not working.
    //      check with helen and fix it.
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_RD_ATOMIC_START_FLIT_ID, RESP_RX_DMA_CMD_RSQ_DB)
    DMA_HBM_MEM2MEM_PHV2MEM_SETUP(DMA_CMD_BASE, db_data1, db_data1, DB_ADDR)
    DMA_SET_WR_FENCE(DMA_CMD_MEM2MEM_T, DMA_CMD_BASE)
    DMA_SET_END_OF_CMDS(DMA_CMD_MEM2MEM_T, DMA_CMD_BASE)

exit:
    nop.e
    nop

nak_rnr:
    //TBD
    nop.e
    nop

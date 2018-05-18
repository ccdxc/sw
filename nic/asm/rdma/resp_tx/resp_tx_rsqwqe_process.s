#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct rsqwqe_d_t d;
struct resp_tx_s2_t0_k k;

#define RKEY_INFO_P t0_s2s_rsqwqe_to_rkey_info

#define CURR_PSN            r1
#define BYTES_SENT          r2
#define XFER_BYTES          r3
#define PAD                 r4

#define BTH_OPCODE          r5
#define XFER_VA             r6

#define PMTU                r6

#define KT_BASE_ADDR        r6
#define KEY_ADDR            r2

#define RQCB1_ADDR          r2
#define DMA_CMD_BASE        r6

#define IN_P t0_s2s_rqcb2_to_rsqwqe_info

%%
    .param      resp_tx_rsqrkey_process
    .param      resp_tx_dcqcn_enforce_process

resp_tx_rsqwqe_process:

    seq         c1, d.read_or_atomic, RSQ_OP_TYPE_READ
    bcf         [c1], process_read
    seq         c2, CAPRI_KEY_FIELD(IN_P, read_rsp_in_progress), 1 //BD Slot
    
process_atomic:
    add         BTH_OPCODE, RDMA_PKT_OPC_ATOMIC_ACK, CAPRI_KEY_FIELD(IN_P, serv_type), BTH_OPC_SVC_SHIFT
    phvwrpair   p.bth.opcode, BTH_OPCODE, p.bth.psn, d.psn

   // phv_p->bth.pkey = 0xffff
   phvwr       p.bth.pkey, 0xffff  
    
    // prepare atomicaeth header
    phvwr       p.atomicaeth.orig_data, d.{atomic.orig_data}.dx

    // header template
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_HDR_TEMPLATE)
    sll         r5, CAPRI_KEY_FIELD(IN_P, header_template_addr), HDR_TEMP_ADDR_SHIFT
    DMA_HBM_MEM2PKT_SETUP(DMA_CMD_BASE, CAPRI_KEY_FIELD(IN_P, header_template_size), r5)

    // bth
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_BTH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, bth, bth)

    // aeth
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_AETH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, aeth, aeth)

    // atomicaeth
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_ATOMICAETH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, atomicaeth, atomicaeth)

    // For ICRC. PAD is always 0 for atomic.
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_PAD_ICRC)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, icrc, icrc)

    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)

    // invoke MPU only dcqcn in table 1.
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_tx_dcqcn_enforce_process, r0)

    nop.e
    // invalidate table 0
    CAPRI_SET_TABLE_0_VALID(0)  //Exit slot


process_read:
    // is it zero length read request ?
    seq         c3, d.read.len, 0
    
    cmov        CURR_PSN, c2, CAPRI_KEY_RANGE(IN_P, curr_read_rsp_psn_sbit0_ebit7, curr_read_rsp_psn_sbit16_ebit23), d.psn

    /*
      bytes_sent = (cur_psn - rsqwqe_p->psn) * 
        (1 << rqcb1_to_rsqwqe_info_p->log_pmtu);
    */
    sub         r2, CURR_PSN, d.psn
    sll         BYTES_SENT, r2, CAPRI_KEY_FIELD(IN_P, log_pmtu)
    sub         XFER_BYTES, d.read.len, BYTES_SENT

    sll         PMTU, 1, CAPRI_KEY_FIELD(IN_P, log_pmtu)

    // transfer_bytes <= PMTU ?
    sle         c5, XFER_BYTES, PMTU
    // curr_psn == rsqwqe_p->psn ?
    seq         c4, CURR_PSN, d.psn

    sll         BTH_OPCODE, CAPRI_KEY_FIELD(IN_P, serv_type), BTH_OPC_SVC_SHIFT
    CAPRI_RESET_TABLE_0_ARG()

    .csbegin
    cswitch     [c5, c4]
    nop

    // 00
    // transfer_bytes > pmtu &&  curr_psn != rsqwqe_p->psn
    .brcase 0
    // mid packet doesn't have pad
    CAPRI_SET_FIELD2(RKEY_INFO_P, transfer_bytes, PMTU)
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_PAD_ICRC)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, icrc, icrc)
    b           next
    or          BTH_OPCODE, BTH_OPCODE, RDMA_PKT_OPC_RDMA_READ_RESP_MID //BD Slot

    // 01
    // transfer_bytes > pmtu &&  curr_psn == rsqwqe_p->psn
    .brcase 1
    // first packet doesn't have pad
    phvwrpair   CAPRI_PHV_FIELD(RKEY_INFO_P, send_aeth), 1, \
                CAPRI_PHV_FIELD(RKEY_INFO_P, transfer_bytes), PMTU //BD Slot
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_PAD_ICRC)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, icrc, icrc)
    b           next
    or          BTH_OPCODE, BTH_OPCODE, RDMA_PKT_OPC_RDMA_READ_RESP_FIRST //BD Slot

    // 10
    // transfer_bytes <= pmtu &&  curr_psn != rsqwqe_p->psn
    .brcase 2
    // calculate pad
    sub         PAD, 4, XFER_BYTES[1:0]
    phvwr       p.bth.pad, PAD[1:0]
    add         PAD, PAD[1:0], 4
    // add crc bytes to pad bytes
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_PAD_ICRC)
    DMA_PHV2PKT_END_LEN_SETUP(DMA_CMD_BASE, r7, icrc, PAD)
    phvwrpair   CAPRI_PHV_RANGE(RKEY_INFO_P, send_aeth, last_or_only), (1<<1)|1, \
                CAPRI_PHV_FIELD(RKEY_INFO_P, transfer_bytes), XFER_BYTES
    b           next
    or          BTH_OPCODE, BTH_OPCODE, RDMA_PKT_OPC_RDMA_READ_RESP_LAST //BD Slot

    // 11
    // transfer_bytes <= pmtu &&  curr_psn == rsqwqe_p->psn
    .brcase 3
    // calculate pad
    sub         PAD, 4, XFER_BYTES[1:0]
    phvwr       p.bth.pad, PAD[1:0]
    // add crc bytes to pad bytes
    add         PAD, PAD[1:0], 4
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_PAD_ICRC)
    DMA_PHV2PKT_END_LEN_SETUP(DMA_CMD_BASE, r7, icrc, PAD)
    phvwrpair   CAPRI_PHV_RANGE(RKEY_INFO_P, send_aeth, last_or_only), (1<<1)|1, \
                CAPRI_PHV_FIELD(RKEY_INFO_P, transfer_bytes), XFER_BYTES
    b           next
    or          BTH_OPCODE, BTH_OPCODE, RDMA_PKT_OPC_RDMA_READ_RESP_ONLY //BD Slot
    
    .csend

next:
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, DMA_CMD_BASE)

    // phv_p->bth.pkey = 0xffff
    phvwr       p.bth.pkey, 0xffff  
    
    phvwrpair   p.bth.opcode, BTH_OPCODE, p.bth.psn, CURR_PSN

    add         XFER_VA, d.read.va, BYTES_SENT
    phvwrpair   CAPRI_PHV_FIELD(RKEY_INFO_P, transfer_va), \
                XFER_VA, \
                CAPRI_PHV_FIELD(RKEY_INFO_P, header_template_addr), \
                CAPRI_KEY_FIELD(IN_P, header_template_addr) 

    phvwrpair   CAPRI_PHV_FIELD(RKEY_INFO_P, curr_read_rsp_psn), CURR_PSN, \
                CAPRI_PHV_FIELD(RKEY_INFO_P, log_pmtu), CAPRI_KEY_FIELD(IN_P, log_pmtu)

    CAPRI_SET_FIELD2(RKEY_INFO_P, header_template_size, CAPRI_KEY_FIELD(IN_P, header_template_size))

    // for zero length read request, skip rkey and directly invoke dcqcn
    bcf         [c3], skip_rkey

    KT_BASE_ADDR_GET2(KT_BASE_ADDR, r1) //BD Slot
    
    // key_addr = hbm_addr_get(PHV_GLOBAL_KT_BASE_ADDR_GET()) +
    // ((sge_p->l_key & KEY_INDEX_MASK) * sizeof(key_entry_t));
    add         r2, r0, d.read.r_key

    KEY_ENTRY_ADDR_GET(KEY_ADDR, KT_BASE_ADDR, r2)

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, resp_tx_rsqrkey_process, KEY_ADDR)

exit:
    nop.e
    nop

skip_rkey:

    CAPRI_SET_FIELD2(RKEY_INFO_P, skip_rkey, 1)
    // invoke rkey program as mpu only
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_tx_rsqrkey_process, r0)

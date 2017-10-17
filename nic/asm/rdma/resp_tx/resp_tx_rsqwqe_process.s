#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_tx_phv_t p;
struct rsqwqe_d_t d;
struct resp_tx_rsqwqe_process_k_t k;

#define RKEY_INFO_T struct resp_tx_rsqwqe_to_rkey_info_t

#define CURR_PSN            r1
#define BYTES_SENT          r2
#define XFER_BYTES          r3

#define BTH_OPCODE          r5
#define XFER_VA             r6

#define PMTU                r6

#define KT_BASE_ADDR        r6
#define KEY_ADDR            r2
#define ALIGNED_KEY_ADDR    r6
#define KEY_ID              r2

#define RAW_TABLE_PC        r7

%%
    .param      resp_tx_rsqrkey_process

resp_tx_rsqwqe_process:

    seq         c1, d.read_or_atomic, RSQ_OP_TYPE_READ
    bcf         [c1], process_read
    seq         c2, k.args.read_rsp_in_progress, 1 //BD Slot
    
process_atomic:
        // TODO


process_read:
    cmov        CURR_PSN, c2, k.args.curr_read_rsp_psn, d.psn

    /*
      bytes_sent = (cur_psn - rsqwqe_p->psn) * 
        (1 << rqcb1_to_rsqwqe_info_p->log_pmtu);
    */
    sub         r2, CURR_PSN, d.psn
    add         r3, r0, k.args.log_pmtu
    sllv        BYTES_SENT, r2, r3
    sub         XFER_BYTES, d.read.len, BYTES_SENT

    add         r6, r0, k.args.log_pmtu
    sllv        PMTU, 1, r6

    // transfer_bytes <= PMTU ?
    sle         c5, XFER_BYTES, PMTU
    // curr_psn == rsqwqe_p->psn ?
    seq         c4, CURR_PSN, d.psn

    sll         BTH_OPCODE, k.args.serv_type, BTH_OPC_SVC_SHIFT
    CAPRI_GET_TABLE_0_ARG(resp_tx_phv_t, r4)

    .csbegin
    cswitch     [c5, c4]
    nop

    // 00
    // transfer_bytes > pmtu &&  curr_psn != rsqwqe_p->psn
    .brcase 0
    or          BTH_OPCODE, BTH_OPCODE, RDMA_PKT_OPC_RDMA_READ_RESP_MID
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, transfer_bytes, PMTU)
    //CAPRI_SET_FIELD(r4, RKEY_INFO_T, last_or_only, 0)
    b           next
    nop

    // 01
    // transfer_bytes > pmtu &&  curr_psn == rsqwqe_p->psn
    .brcase 1
    or          BTH_OPCODE, BTH_OPCODE, RDMA_PKT_OPC_RDMA_READ_RESP_FIRST
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, send_aeth, 1)
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, transfer_bytes, PMTU)
    //CAPRI_SET_FIELD(r4, RKEY_INFO_T, last_or_only, 0)
    b           next
    nop

    // 10
    // transfer_bytes <= pmtu &&  curr_psn != rsqwqe_p->psn
    .brcase 2
    or          BTH_OPCODE, BTH_OPCODE, RDMA_PKT_OPC_RDMA_READ_RESP_LAST
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, send_aeth, 1)
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, transfer_bytes, XFER_BYTES)
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, last_or_only, 1)
    b           next
    nop

    // 11
    // transfer_bytes <= pmtu &&  curr_psn == rsqwqe_p->psn
    .brcase 3
    or          BTH_OPCODE, BTH_OPCODE, RDMA_PKT_OPC_RDMA_READ_RESP_ONLY
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, send_aeth, 1)
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, transfer_bytes, XFER_BYTES)
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, last_or_only, 1)
    b           next
    nop
    
    .csend

next:

    phvwr       p.bth.opcode, BTH_OPCODE
    phvwr       p.bth.psn, CURR_PSN

    add         XFER_VA, d.read.va, BYTES_SENT
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, transfer_va, XFER_VA)
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, log_pmtu, k.args.log_pmtu)
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, header_template_addr, k.args.header_template_addr)
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, curr_read_rsp_psn, CURR_PSN)
    CAPRI_SET_FIELD(r4, RKEY_INFO_T, new_rsq_c_index, k.args.new_rsq_c_index)

    KT_BASE_ADDR_GET(KT_BASE_ADDR, r1)
    
    // key_addr = hbm_addr_get(PHV_GLOBAL_KT_BASE_ADDR_GET()) +
    // ((sge_p->l_key & KEY_INDEX_MASK) * sizeof(key_entry_t));
    add         r2, r0, d.read.r_key

    //andi        r2, r2, KEY_INDEX_MASK
    //sll         r2, r2, LOG_SIZEOF_KEY_ENTRY_T
    //add         KEY_ADDR, r2, KT_BASE_ADDR

    KEY_ENTRY_ADDR_GET(KEY_ADDR, KT_BASE_ADDR, r2)

    //aligned_key_addr = key_addr & ~HBM_CACHE_LINE_MASK;
    //and         r6, KEY_ADDR, HBM_CACHE_LINE_SIZE_MASK
    //sub         ALIGNED_KEY_ADDR, KEY_ADDR, r6

    KEY_ENTRY_ALIGNED_ADDR_GET(ALIGNED_KEY_ADDR, KEY_ADDR)

    //key_id = (key_addr % HBM_CACHE_LINE_SIZE) / sizeof(key_entry_t);
    // compute (key_addr - aligned_key_addr) >> log_key_entry_t
    //sub         KEY_ID, KEY_ADDR, ALIGNED_KEY_ADDR
    //srl         KEY_ID, KEY_ID, LOG_SIZEOF_KEY_ENTRY_T

    KEY_ID_GET(KEY_ID, KEY_ADDR)

    CAPRI_SET_FIELD(r4, RKEY_INFO_T, key_id, KEY_ID)

    CAPRI_GET_TABLE_0_K(resp_tx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_tx_rsqrkey_process)
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, ALIGNED_KEY_ADDR)

exit:
    nop.e
    nop

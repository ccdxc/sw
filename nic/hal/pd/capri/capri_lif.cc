/*
 * capri_lif_rw.cc
 * Neel Patel (Pensando Systems)
 */

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <map>
//#include <base.h>

#include <base.h>
#include <p4pd.h>
#include <p4pd_api.hpp>
#include <capri_loader.h>
#include <capri_lif.hpp>
#include <lib_model_client.h>

#undef HAL_GTEST
#ifndef HAL_GTEST
#include <cap_blk_reg_model.h>
#include <cap_top_csr.h>
#include <cap_psp_csr.h>
#include <cap_wa_csr.h>
#include <cpp_int_helper.h>
#include <capri_hbm.hpp>
#endif


#define LIF_QSTATE_SIZE                         (4096)  /* Total size of all Qstate entries */
#define LIF_QSTATE_BASE(base_addr, hw_lif_id)   (base_addr + (hw_lif_id * NUM_QSTATE_PER_LIF * LIF_QSTATE_SIZE))


struct qstate {

    uint8_t     pc_offset;
    uint8_t     rsvd0;
    uint8_t     cosA : 4;
    uint8_t     cosB : 4;
    uint8_t     cos_sel;
    uint8_t     eval_last;
    uint8_t     total_rings : 4;
    uint8_t     host_rings : 4;
    uint16_t    pid;

    struct {
        uint16_t p_index;
        uint16_t c_index;
    } ring[8] __PACK__ ;

} __PACK__;

hal_ret_t capri_lif_qstate_create(uint32_t hw_lif_id) {

    int rv;

    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_psp_csr_t &psp = cap0.pt.pt.psp;
    cap_psp_csr_t &npv = cap0.pr.pr.psp;
    cap_wa_csr_t &db = cap0.db.wa;

    uint64_t base_addr = (uint64_t)get_start_offset((char *)"qstate") + (hw_lif_id * LIF_QSTATE_SIZE);

    if(hw_lif_id == 1001) {
        base_addr = (uint64_t)get_start_offset((char *)"tcpcb");
        HAL_TRACE_DEBUG("*************Creating 10001 lif with base address: 0x%lx ", base_addr);
    }

    struct qstate qstate_arr[8];

    printf("Creating QSTATE for lif = %u, base_addr = 0x%lx\n", hw_lif_id, base_addr);

    /*
     *  All descriptors are 64B in size
     *
     *  QID0 => RXQ     (P4 steers to QID0 so we must treat it as RXQ)
     *  QID1 => RX-CQ
     *  QID2 => TXQ
     *  QID3 => TX-CQ
     */
    psp.dhs_lif_qstate_map.entry[hw_lif_id].vld(1);
    psp.dhs_lif_qstate_map.entry[hw_lif_id].qstate_base(base_addr >> 12);

#if 1
    // HACK
    if(hw_lif_id == 1001) {
        printf("xxx: creating lif qstate for hw_lif_id = %d\n", hw_lif_id);
        psp.dhs_lif_qstate_map.entry[hw_lif_id].length0(10); // # entries 2^10
        psp.dhs_lif_qstate_map.entry[hw_lif_id].size0(4); //
        psp.dhs_lif_qstate_map.entry[hw_lif_id].length1(0);
        psp.dhs_lif_qstate_map.entry[hw_lif_id].size1(0);
        psp.dhs_lif_qstate_map.entry[hw_lif_id].length2(0);
        psp.dhs_lif_qstate_map.entry[hw_lif_id].size2(0);
        psp.dhs_lif_qstate_map.entry[hw_lif_id].length3(0);
        psp.dhs_lif_qstate_map.entry[hw_lif_id].size3(0);
    } else {
#endif
        printf("creating lif qstate for hw_lif_id = %d\n", hw_lif_id);
        psp.dhs_lif_qstate_map.entry[hw_lif_id].length0(1);
        psp.dhs_lif_qstate_map.entry[hw_lif_id].size0(1);
        psp.dhs_lif_qstate_map.entry[hw_lif_id].length1(1);
        psp.dhs_lif_qstate_map.entry[hw_lif_id].size1(1);
        psp.dhs_lif_qstate_map.entry[hw_lif_id].length2(1);
        psp.dhs_lif_qstate_map.entry[hw_lif_id].size2(1);
        psp.dhs_lif_qstate_map.entry[hw_lif_id].length3(1);
        psp.dhs_lif_qstate_map.entry[hw_lif_id].size3(1);
#if 1
    // HACK
    }
#endif
    psp.dhs_lif_qstate_map.entry[hw_lif_id].write();


    npv.dhs_lif_qstate_map.entry[hw_lif_id].vld(1);
    npv.dhs_lif_qstate_map.entry[hw_lif_id].qstate_base(base_addr >> 12);
    npv.dhs_lif_qstate_map.entry[hw_lif_id].length0(1);
    npv.dhs_lif_qstate_map.entry[hw_lif_id].size0(1);
    npv.dhs_lif_qstate_map.entry[hw_lif_id].length1(1);
    npv.dhs_lif_qstate_map.entry[hw_lif_id].size1(1);
    npv.dhs_lif_qstate_map.entry[hw_lif_id].length2(1);
    npv.dhs_lif_qstate_map.entry[hw_lif_id].size2(1);
    npv.dhs_lif_qstate_map.entry[hw_lif_id].length3(1);
    npv.dhs_lif_qstate_map.entry[hw_lif_id].size3(1);
    npv.dhs_lif_qstate_map.entry[hw_lif_id].write();

    db.dhs_lif_qstate_map.entry[hw_lif_id].vld(1);
    db.dhs_lif_qstate_map.entry[hw_lif_id].qstate_base(base_addr >> 12);
    db.dhs_lif_qstate_map.entry[hw_lif_id].length0(1);
    db.dhs_lif_qstate_map.entry[hw_lif_id].size0(1);
    db.dhs_lif_qstate_map.entry[hw_lif_id].length1(1);
    db.dhs_lif_qstate_map.entry[hw_lif_id].size1(1);
    db.dhs_lif_qstate_map.entry[hw_lif_id].length2(1);
    db.dhs_lif_qstate_map.entry[hw_lif_id].size2(1);
    db.dhs_lif_qstate_map.entry[hw_lif_id].length3(1);
    db.dhs_lif_qstate_map.entry[hw_lif_id].size3(1);
    db.dhs_lif_qstate_map.entry[hw_lif_id].write();

    memset(&qstate_arr[0], 0, sizeof(qstate_arr));
    for (int i = 0; i < 8; i++) {
        qstate_arr[i].host_rings = 1;
        qstate_arr[i].total_rings = 1;
    }

    if(hw_lif_id == 1001) {
        HAL_TRACE_DEBUG("Skipping Qstate programming for LIF 1001");
        return HAL_RET_OK;
    }


    if(hw_lif_id == 1001) {
        HAL_TRACE_DEBUG("*************Skipping Qstate programming for 10001 LIF ");
        return HAL_RET_OK;
    }


    uint64_t offset;

    char rxdma_prog[] = "rxdma_stage0.bin";
    char txdma_prog[] = "txdma_stage0.bin";
    char eth_rx_label[] = "eth_rx_stage0";
    char eth_tx_label[] = "eth_tx_stage0";

    rv = capri_program_label_to_offset("p4plus", rxdma_prog, eth_rx_label, &offset);
    if (rv < 0) {
        printf("ERROR: Failed to resolve handle p4plus prog %s label %s\n", rxdma_prog, eth_rx_label);
        return HAL_RET_HW_FAIL;
    }
    qstate_arr[0].pc_offset = offset;

    rv = capri_program_label_to_offset("p4plus", txdma_prog, eth_tx_label, &offset);
    if (rv < 0) {
        printf("ERROR: Failed to resolve handle p4plus prog %s label %s\n", txdma_prog, eth_tx_label);
        return HAL_RET_HW_FAIL;
    }
    qstate_arr[2].pc_offset = offset;

    printf("Qstate array size %lu\n", sizeof(qstate_arr));

    rv = write_mem(base_addr, (uint8_t *)&qstate_arr, sizeof(qstate_arr));
    if (rv != true) {
        printf("ERROR: Failed to write QSTATE for lif %d\n", hw_lif_id);
        return HAL_RET_HW_FAIL;
    }

    return HAL_RET_OK;
}

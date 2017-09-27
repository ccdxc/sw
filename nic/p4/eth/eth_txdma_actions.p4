/*
    NOTE: Do not move these defines, because these need to be defined
          before including the common p4 defines.
*/

#include "../common-p4+/common_txdma_dummy.p4"

#define common_p4plus_stage0_app_header_table_action_dummy eth_tx_fetch_desc
#define tx_table_s0_t0_action eth_tx_fetch_desc
#define tx_table_s1_t0_action eth_tx_packet


#include "../common-p4+/common_txdma.p4"
#include "eth_txdma.p4"
#include "defines.h"

/******************************************************************************
 * Action functions
 * - These action functions are used to generate k+i and d structures.
 *****************************************************************************/

action eth_tx_dummy(data0, data1, data2, data3, data4, data5, data6, data7)
{
    SCRATCH_METADATA_INIT_7(scratch_metadata0)
}

action eth_tx_fetch_desc(
        pc, rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid,
        p_index0, c_index0, p_index1, c_index1, p_index2, c_index2, p_index3, c_index3, 
        p_index4, c_index4, p_index5, c_index5, p_index6, c_index6, p_index7, c_index7, 
        enable, ring_base, ring_size, cq_ring_base)
{
    // For K+I struct generation
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    //modify_field(eth_tx_global_scratch.lif, eth_tx_global.lif);
    //modify_field(eth_tx_global_scratch.qtype, eth_tx_global.qtype);
    //modify_field(eth_tx_global_scratch.qid, eth_tx_global.qid); 

    // For D-struct generation
    MODIFY_QSTATE_INTRINSIC(eth_tx_qstate)
    modify_field(eth_tx_qstate.enable, enable);
    modify_field(eth_tx_qstate.ring_base, ring_base);
    modify_field(eth_tx_qstate.ring_size, ring_size);
    modify_field(eth_tx_qstate.cq_ring_base, cq_ring_base);
}

action eth_tx_packet(addr, len)
{
    // For K+I struct generation

    // Global K+I
    modify_field(eth_tx_global_scratch.lif, eth_tx_global.lif);
    modify_field(eth_tx_global_scratch.qtype, eth_tx_global.qtype);
    modify_field(eth_tx_global_scratch.qid, eth_tx_global.qid);

    // To-Stage K+I
    modify_field(eth_tx_to_s1_scratch.cq_desc_addr, eth_tx_to_s1.cq_desc_addr);

    // Stage-to-Stage K+I

    // For D-struct generation
    modify_field(eth_tx_desc.addr, addr);
    modify_field(eth_tx_desc.len, len);
}

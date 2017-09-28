/*
    NOTE: Do not move these defines, because these need to be defined
          before including the common p4 defines.
*/

#include "../common-p4+/common_txdma_dummy.p4"

#define common_p4plus_stage0_app_header_table_action_dummy adminq_fetch_desc
#define tx_table_s0_t0_action adminq_fetch_desc
#define tx_table_s1_t0_action adminq_process_desc


#include "../common-p4+/common_txdma.p4"
#include "adminq_txdma.p4"
#include "defines.h"

/******************************************************************************
 * Action functions
 * - These action functions are used to generate k+i and d structures.
 *****************************************************************************/

action adminq_fetch_desc(
        rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid,
        p_index0, c_index0, p_index1, c_index1,
        enable, ring_base, ring_size)

{
    // For K+I struct generation
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

// Touch all fields -- write a macro
    modify_field(adminq_qstate.rsvd, rsvd);
    modify_field(adminq_qstate.cosA, cosA);
    modify_field(adminq_qstate.cosB, cosB);
    modify_field(adminq_qstate.cos_sel, cos_sel);
    modify_field(adminq_qstate.eval_last, eval_last);
    modify_field(adminq_qstate.host, host);
    modify_field(adminq_qstate.total, total);
    modify_field(adminq_qstate.pid, pid);
    modify_field(adminq_qstate.p_index0, p_index0);
    modify_field(adminq_qstate.c_index0, c_index0);
    modify_field(adminq_qstate.p_index1, p_index1);
    modify_field(adminq_qstate.c_index1, c_index1);
    modify_field(adminq_qstate.enable, enable);
    modify_field(adminq_qstate.ring_base, ring_base);
    modify_field(adminq_qstate.ring_size, ring_size);
}

action adminq_process_desc(opcode, rsvd, cmd_data0, cmd_data1, cmd_data2, cmd_data3,
        cmd_data4, cmd_data5, cmd_data6, cmd_data7, cmd_data8, cmd_data9, cmd_data10,
        cmd_data11, cmd_data12, cmd_data13, cmd_data14, cmd_data15)
{
    // For D-struct generation
    modify_field(adminq_desc.opcode, opcode);
    modify_field(adminq_desc.rsvd, rsvd);
    modify_field(adminq_desc.cmd_data0, cmd_data0);
    modify_field(adminq_desc.cmd_data1, cmd_data1);
    modify_field(adminq_desc.cmd_data2, cmd_data2);
    modify_field(adminq_desc.cmd_data3, cmd_data3);
    modify_field(adminq_desc.cmd_data4, cmd_data4);
    modify_field(adminq_desc.cmd_data5, cmd_data5);
    modify_field(adminq_desc.cmd_data6, cmd_data6);
    modify_field(adminq_desc.cmd_data7, cmd_data7);
    modify_field(adminq_desc.cmd_data8, cmd_data8);
    // Prepare cq descriptors - temp fill the same data
    modify_field(adminq_to_s1_scratch.cq_ring_base, adminq_to_s1.cq_ring_base);

    modify_field(adminq_s2s_scratch.cmd_status, adminq_s2s.cmd_status);
    modify_field(adminq_s2s_scratch.cpl_id, adminq_s2s.cpl_id);
    modify_field(adminq_s2s_scratch.rsvd, adminq_s2s.rsvd);
    modify_field(adminq_s2s_scratch.color, adminq_s2s.color);
    modify_field(adminq_s2s_scratch.cmd_data0, adminq_s2s.cmd_data0);
    modify_field(adminq_s2s_scratch.cmd_data1, adminq_s2s.cmd_data1);
    modify_field(adminq_s2s_scratch.cmd_data2, adminq_s2s.cmd_data2);
}

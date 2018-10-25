

#define MODIFY_QSTATE_INTRINSIC(QSTATE) \
    modify_field(QSTATE.rsvd, rsvd); \
    modify_field(QSTATE.cosA, cosA); \
    modify_field(QSTATE.cosB, cosB); \
    modify_field(QSTATE.cos_sel, cos_sel); \
    modify_field(QSTATE.eval_last, eval_last); \
    modify_field(QSTATE.host, host); \
    modify_field(QSTATE.total, total);  \
    modify_field(QSTATE.pid, pid); \
    modify_field(QSTATE.p_index0, p_index0); \
    modify_field(QSTATE.c_index0, c_index0); \
    modify_field(QSTATE.comp_index, comp_index); \
    modify_field(QSTATE.ci_fetch, ci_fetch);

#define PARAMS_ADMINQ_QSTATE \
    pc, rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, \
    p_index0, c_index0, comp_index, ci_fetch, \
    color, spec_miss, rsvd1, \
    enable, host_queue, intr_enable, rsvd2, \
    ring_base, ring_size, cq_ring_base, intr_assert_index, \
    nicmgr_qstate_addr

#define MODIFY_ADMINQ_QSTATE \
    MODIFY_QSTATE_INTRINSIC(adminq_qstate) \
    modify_field(adminq_qstate.color, color); \
    modify_field(adminq_qstate.spec_miss, spec_miss); \
    modify_field(adminq_qstate.rsvd1, rsvd1); \
    modify_field(adminq_qstate.enable, enable); \
    modify_field(adminq_qstate.host_queue, host_queue); \
    modify_field(adminq_qstate.intr_enable, intr_enable); \
    modify_field(adminq_qstate.rsvd2, rsvd2); \
    modify_field(adminq_qstate.ring_base, ring_base); \
    modify_field(adminq_qstate.ring_size, ring_size); \
    modify_field(adminq_qstate.cq_ring_base, cq_ring_base); \
    modify_field(adminq_qstate.intr_assert_index, intr_assert_index); \
    modify_field(adminq_qstate.nicmgr_qstate_addr, nicmgr_qstate_addr);

#define PARAMS_NICMGR_QSTATE \
    pc, rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, \
    p_index0, c_index0, comp_index, ci_fetch, \
    color, rsvd1, \
    enable, intr_enable, rsvd2, \
    ring_base, ring_size, cq_ring_base, intr_assert_index

#define MODIFY_NICMGR_QSTATE \
    MODIFY_QSTATE_INTRINSIC(nicmgr_qstate) \
    modify_field(nicmgr_qstate.color, color); \
    modify_field(nicmgr_qstate.rsvd1, rsvd1); \
    modify_field(nicmgr_qstate.enable, enable); \
    modify_field(nicmgr_qstate.intr_enable, intr_enable); \
    modify_field(nicmgr_qstate.rsvd2, rsvd2); \
    modify_field(nicmgr_qstate.ring_base, ring_base); \
    modify_field(nicmgr_qstate.ring_size, ring_size); \
    modify_field(nicmgr_qstate.cq_ring_base, cq_ring_base); \
    modify_field(nicmgr_qstate.intr_assert_index, intr_assert_index); \

/*
 * ADMINQ
 */

#define MODIFY_ADMINQ_GLOBAL    \
    modify_field(adminq_global_scratch.dma_cur_index, adminq_global.dma_cur_index);

#define MODIFY_ADMINQ_T0_S2S    \
    modify_field(adminq_t0_s2s_scratch.lif, adminq_t0_s2s.lif); \
    modify_field(adminq_t0_s2s_scratch.qtype, adminq_t0_s2s.qtype); \
    modify_field(adminq_t0_s2s_scratch.qid, adminq_t0_s2s.qid); \
    modify_field(adminq_t0_s2s_scratch.adminq_qstate_addr, adminq_t0_s2s.adminq_qstate_addr); \
    modify_field(adminq_t0_s2s_scratch.nicmgr_qstate_addr, adminq_t0_s2s.nicmgr_qstate_addr); \
    modify_field(adminq_t0_s2s_scratch.host_queue, adminq_t0_s2s.host_queue);

#define MODIFY_ADMINQ_TO_S1

#define MODIFY_ADMINQ_TO_S2     \
    modify_field(adminq_to_s2_scratch.nicmgr_db_data, adminq_to_s2.nicmgr_db_data); \
    modify_field(adminq_to_s2_scratch.intr_assert_data, adminq_to_s2.intr_assert_data);

#define MODIFY_ADMINQ_TO_S3     \
    modify_field(adminq_to_s3_scratch.adminq_ci, adminq_to_s3.adminq_ci);

/*
 * NICMGR
 */
#define MODIFY_NICMGR_GLOBAL   \
    modify_field(nicmgr_global_scratch.dma_cur_index, nicmgr_global.dma_cur_index);

#define MODIFY_NICMGR_T0_S2S    \
    modify_field(nicmgr_t0_s2s_scratch.lif, nicmgr_t0_s2s.lif); \
    modify_field(nicmgr_t0_s2s_scratch.qtype, nicmgr_t0_s2s.qtype); \
    modify_field(nicmgr_t0_s2s_scratch.qid, nicmgr_t0_s2s.qid); \
    modify_field(nicmgr_t0_s2s_scratch.comp_index, nicmgr_t0_s2s.comp_index); \
    modify_field(nicmgr_t0_s2s_scratch.nicmgr_qstate_addr, nicmgr_t0_s2s.nicmgr_qstate_addr); \
    modify_field(nicmgr_t0_s2s_scratch.host_queue, nicmgr_t0_s2s.host_queue);

#define MODIFY_NICMGR_T1_S2S

#define MODIFY_NICMGR_TO_S1

#define MODIFY_NICMGR_TO_S2     \
    modify_field(nicmgr_to_s2_scratch.nicmgr_db_data, nicmgr_to_s2.nicmgr_db_data); \
    modify_field(nicmgr_to_s2_scratch.intr_assert_data, nicmgr_to_s2.intr_assert_data);

#define MODIFY_NICMGR_TO_S3     \
    modify_field(nicmgr_to_s3_scratch.nicmgr_ci, nicmgr_to_s3.nicmgr_ci);

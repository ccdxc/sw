

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
    modify_field(QSTATE.comp_index, comp_index);

#define PARAMS_EDMA_QSTATE \
    pc, rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, \
    p_index0, c_index0, comp_index, \
    color, rsvd1, \
    enable, intr_enable, rsvd2, \
    ring_base, ring_size, cq_ring_base, intr_assert_index

#define MODIFY_EDMA_QSTATE \
    MODIFY_QSTATE_INTRINSIC(edma_qstate) \
    modify_field(edma_qstate.color, color); \
    modify_field(edma_qstate.rsvd1, rsvd1); \
    modify_field(edma_qstate.enable, enable); \
    modify_field(edma_qstate.intr_enable, intr_enable); \
    modify_field(edma_qstate.rsvd2, rsvd2); \
    modify_field(edma_qstate.ring_base, ring_base); \
    modify_field(edma_qstate.ring_size, ring_size); \
    modify_field(edma_qstate.cq_ring_base, cq_ring_base); \
    modify_field(edma_qstate.intr_assert_index, intr_assert_index); \

#define MODIFY_EDMA_GLOBAL \
    modify_field(edma_global_scratch.dma_cur_index, edma_global.dma_cur_index); \
    modify_field(edma_global_scratch.intr_enable, edma_global.intr_enable);

#define MODIFY_EDMA_T0_S2S \
    modify_field(edma_t0_s2s_scratch.cq_desc_addr, edma_t0_s2s.cq_desc_addr); \
    modify_field(edma_t0_s2s_scratch.intr_assert_index, edma_t0_s2s.intr_assert_index); \
    modify_field(edma_t0_s2s_scratch.intr_assert_data, edma_t0_s2s.intr_assert_data);

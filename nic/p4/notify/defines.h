

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
    modify_field(QSTATE.c_index0, c_index0);

#define PARAMS_NOTIFY_QSTATE \
    pc, rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, \
    p_index0, c_index0, host_pindex, \
    rsvd1, \
    enable, host_queue, intr_enable, rsvd2, \
    ring_base, ring_size, \
    host_ring_base, host_ring_size, host_intr_assert_index

#define MODIFY_NOTIFY_QSTATE \
    MODIFY_QSTATE_INTRINSIC(notify_qstate) \
    modify_field(notify_qstate.host_pindex, host_pindex); \
    modify_field(notify_qstate.rsvd1, rsvd1); \
    modify_field(notify_qstate.enable, enable); \
    modify_field(notify_qstate.host_queue, host_queue); \
    modify_field(notify_qstate.intr_enable, intr_enable); \
    modify_field(notify_qstate.rsvd2, rsvd2); \
    modify_field(notify_qstate.ring_base, ring_base); \
    modify_field(notify_qstate.ring_size, ring_size); \
    modify_field(notify_qstate.host_ring_base, host_ring_base); \
    modify_field(notify_qstate.host_ring_size, host_ring_size); \
    modify_field(notify_qstate.host_intr_assert_index, host_intr_assert_index);

#define MODIFY_NOTIFY_GLOBAL \
    modify_field(notify_global_scratch.dma_cur_index, notify_global.dma_cur_index); \
    modify_field(notify_global_scratch.host_queue, notify_global.host_queue); \
    modify_field(notify_global_scratch.intr_enable, notify_global.intr_enable);

#define MODIFY_NOTIFY_T0_S2S \
    modify_field(notify_t0_s2s_scratch.host_desc_addr, notify_t0_s2s.host_desc_addr); \
    modify_field(notify_t0_s2s_scratch.intr_assert_index, notify_t0_s2s.intr_assert_index); \
    modify_field(notify_t0_s2s_scratch.intr_assert_data, notify_t0_s2s.intr_assert_data);

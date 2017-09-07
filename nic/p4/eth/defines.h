
// HACK: Commented out as workaround for NCC action_id bug
// modify_field(QSTATE.pc, pc); \


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
    modify_field(QSTATE.p_index1, p_index1); \
    modify_field(QSTATE.c_index1, c_index1); \
    modify_field(QSTATE.p_index2, p_index2); \
    modify_field(QSTATE.c_index2, c_index2); \
    modify_field(QSTATE.p_index3, p_index3); \
    modify_field(QSTATE.c_index3, c_index3); \
    modify_field(QSTATE.p_index4, p_index4); \
    modify_field(QSTATE.c_index4, c_index4); \
    modify_field(QSTATE.p_index5, p_index5); \
    modify_field(QSTATE.c_index5, c_index5); \
    modify_field(QSTATE.p_index6, p_index6); \
    modify_field(QSTATE.c_index6, c_index6); \
    modify_field(QSTATE.p_index7, p_index7); \
    modify_field(QSTATE.c_index7, c_index7);

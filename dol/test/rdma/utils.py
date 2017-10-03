def VerifyFieldModify(tc, pre_state, post_state, field_name, incr):
    pre_val = getattr(pre_state, field_name)
    post_val = getattr(post_state, field_name)
    tc.info("%s pre: %d  post(actual): %d expected: %d" \
                    %(field_name, pre_val, post_val, pre_val+incr))
    cmp = (pre_val+incr == post_val)
    tc.info('    Match: %s ' %cmp)
    return cmp

def VerifyFieldMaskModify(tc, pre_state, post_state, field_name, mask, incr):
    pre_val = getattr(pre_state, field_name)
    post_val = getattr(post_state, field_name)
    tc.info("%s pre: %d  post(actual): %d expected: %d" \
                    %(field_name, pre_val, post_val, ((pre_val+incr) & mask)))
    cmp = (((pre_val+incr) & mask) == post_val)
    tc.info('    Match: %s ' %cmp)
    return cmp

def VerifyFieldAbsolute(tc, state, field_name, exp_val):
    val = getattr(state, field_name)
    tc.info("%s actual: %d expected: %d" \
             %(field_name, val, exp_val))
    cmp = (val == exp_val)
    tc.info('    Match: %s ' %cmp)
    return cmp

############     CQ VALIDATIONS #################
def ValidateRespRxCQChecks(tc):
    rs = tc.config.rdmasession
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_post_qstate = rs.lqp.rq_cq.qstate.data
    # verify that p_index is incremented by 1, as cqwqe is posted
    if not VerifyFieldModify(tc, tc.pvtdata.rq_cq_pre_qstate, tc.pvtdata.rq_cq_post_qstate, 'p_index0', 1):
        return False

    # verify that color bit in CQWQE and CQCB are same
    #tc.info('Color from Exp CQ Descriptor: %d' % tc.descriptors.Get('EXP_CQ_DESC').color)
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_cq_post_qstate, 'color', tc.descriptors.Get('EXP_CQ_DESC').color):
        return False

    return True


def ValidateNoCQChanges(tc):
    rs = tc.config.rdmasession
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_post_qstate = rs.lqp.rq_cq.qstate.data
    # verify that no change to p_index
    if not VerifyFieldModify(tc, tc.pvtdata.rq_cq_pre_qstate, tc.pvtdata.rq_cq_post_qstate, 'p_index0', 0):
        return False

    return True



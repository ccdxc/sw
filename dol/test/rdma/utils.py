import math

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

def VerifyFieldsEqual(tc, state1, field_name1, state2, field_name2):
    val = getattr(state1, field_name1)
    exp_val = getattr(state2, field_name2)
    tc.info("%s actual: %d %s expected: %d" \
             %(field_name1, val, field_name2, exp_val))
    cmp = (val == exp_val)
    tc.info('    Match: %s ' %cmp)
    return cmp


############     CQ VALIDATIONS #################
def ValidateRespRxCQChecks(tc):
    rs = tc.config.rdmasession
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_post_qstate = rs.lqp.rq_cq.qstate.data
    log_num_cq_wqes = getattr(tc.pvtdata.rq_cq_post_qstate, 'log_num_wqes')
    ring0_mask = (2 ** log_num_cq_wqes) - 1

    # verify that p_index is incremented by 1, as cqwqe is posted
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_cq_pre_qstate, tc.pvtdata.rq_cq_post_qstate, 'p_index0', ring0_mask, 1):
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
    log_num_cq_wqes = getattr(tc.pvtdata.rq_cq_post_qstate, 'log_num_wqes')
    ring0_mask = (2 ** log_num_cq_wqes) - 1

    # verify that no change to p_index
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_cq_pre_qstate, tc.pvtdata.rq_cq_post_qstate, 'p_index0', ring0_mask, 0):
        return False

    # verify that no change to c_index
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_cq_pre_qstate, tc.pvtdata.rq_cq_post_qstate, 'c_index0', ring0_mask, 0):
        return False

    return True

############     EQ VALIDATIONS #################
def ValidateRespRxEQChecks(tc):
    rs = tc.config.rdmasession
    rs.lqp.eq.qstate.Read()
    tc.pvtdata.eq_post_qstate = rs.lqp.eq.qstate.data
    log_num_eq_wqes = getattr(tc.pvtdata.eq_post_qstate, 'log_num_wqes')
    ring0_mask = (2 ** log_num_eq_wqes) - 1

    # verify that p_index is incremented by 1, as eqwqe is posted
    if not VerifyFieldMaskModify(tc, tc.pvtdata.eq_pre_qstate, tc.pvtdata.eq_post_qstate, 'p_index0', ring0_mask, 1):
        return False

    # verify that c_index is incremented by 1, as eqwqe is consumed
    if not VerifyFieldMaskModify(tc, tc.pvtdata.eq_pre_qstate, tc.pvtdata.eq_post_qstate, 'c_index0', ring0_mask, 1):
        return False

    # verify that color bit in EQWQE and EQCB are same
    #TODO Comment below one before the commit
    tc.info('Color from Exp EQ Descriptor: %d' % tc.descriptors.Get('EXP_EQ_DESC').color)
    if not VerifyFieldAbsolute(tc, tc.pvtdata.eq_post_qstate, 'color', tc.descriptors.Get('EXP_EQ_DESC').color):
        return False

    return True


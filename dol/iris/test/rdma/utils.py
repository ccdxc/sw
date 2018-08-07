import math
import copy
from infra.common.logging import logger as logger
from infra.common.logging import logger as logger
def VerifyFieldModify(tc, pre_state, post_state, field_name, incr):
    pre_val = getattr(pre_state, field_name)
    post_val = getattr(post_state, field_name)
    logger.info("%s pre: %d  post(actual): %d expected: %d" \
                    %(field_name, pre_val, post_val, pre_val+incr))
    cmp = (pre_val+incr == post_val)
    logger.info('    Match: %s ' %cmp)
    return cmp

def VerifyFieldMaskModify(tc, pre_state, post_state, field_name, mask, incr):
    pre_val = getattr(pre_state, field_name)
    post_val = getattr(post_state, field_name)
    logger.info("%s pre: %d  post(actual): %d expected: %d" \
                    %(field_name, pre_val, post_val, ((pre_val+incr) & mask)))
    cmp = (((pre_val+incr) & mask) == post_val)
    logger.info('    Match: %s ' %cmp)
    return cmp

def VerifyFieldAbsolute(tc, state, field_name, exp_val):
    val = getattr(state, field_name)
    logger.info("%s actual: %d expected: %d" \
             %(field_name, val, exp_val))
    cmp = (val == exp_val)
    logger.info('    Match: %s ' %cmp)
    return cmp

def VerifyFieldsEqual(tc, state1, field_name1, state2, field_name2):
    val = getattr(state1, field_name1)
    exp_val = getattr(state2, field_name2)
    logger.info("%s actual: %d %s expected: %d" \
             %(field_name1, val, field_name2, exp_val))
    cmp = (val == exp_val)
    logger.info('    Match: %s ' %cmp)
    return cmp

def VerifyErrQState(tc):
    #sq
    #TODO: 

    #rq
    # verify that rqcb1 state is moved to ERR (2)
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'cb1_state', 2):
        return False

    return True

def PopulatePreQStates(tc):
    rs = tc.config.rdmasession

    #sq
    rs.lqp.sq.qstate.Read()
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    #rq
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = copy.deepcopy(rs.lqp.rq.qstate.data)
    #sq_cq
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_pre_qstate = rs.lqp.sq_cq.qstate.data
    #rq_cq
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_pre_qstate = rs.lqp.rq_cq.qstate.data
    return

def PopulatePostQStates(tc):
    rs = tc.config.rdmasession

    #sq
    rs.lqp.sq.qstate.Read()
    tc.pvtdata.sq_post_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    #rq
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_post_qstate = copy.deepcopy(rs.lqp.rq.qstate.data)
    #sq_cq
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_post_qstate = rs.lqp.sq_cq.qstate.data
    #rq_cq
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_post_qstate = rs.lqp.rq_cq.qstate.data
    return

def PostToPreCopyQStates(tc):
    rs = tc.config.rdmasession
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.rq_pre_qstate = copy.deepcopy(rs.lqp.rq.qstate.data)
    tc.pvtdata.sq_cq_pre_qstate = copy.deepcopy(rs.lqp.sq_cq.qstate.data)
    tc.pvtdata.rq_cq_pre_qstate = copy.deepcopy(rs.lqp.rq_cq.qstate.data)
    return

def ResetErrQState(tc):
    rs = tc.config.rdmasession

    #Reset sq
    rs.lqp.sq.qstate.Read()
    rs.lqp.sq.qstate.data.busy = 0;
    rs.lqp.sq.qstate.data.cb1_busy = 0;
    rs.lqp.sq.qstate.data.in_progress = 0;
    rs.lqp.sq.qstate.data.state = 4 # QP_STATE_RTS
    rs.lqp.sq.qstate.data.p_index1 = ((rs.lqp.sq.qstate.data.p_index1 - 1) & 0xffff)
    rs.lqp.sq.qstate.WriteWithDelay()

    #Reset Rq
    rs.lqp.rq.qstate.Read()
    rs.lqp.rq.qstate.data.cb0_state = rs.lqp.rq.qstate.data.cb1_state = 4 # QP_STATE_RTS
    rs.lqp.rq.qstate.WriteWithDelay()

    return

############     CQ VALIDATIONS #################
def ValidateCQCompletions(tc, num_sq_completions, num_rq_completions):
    rs = tc.config.rdmasession

    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_post_qstate = rs.lqp.sq_cq.qstate.data
    log_num_cq_wqes = getattr(tc.pvtdata.sq_cq_post_qstate, 'log_num_wqes')
    sq_ring0_mask = (2 ** log_num_cq_wqes) - 1

    if rs.lqp.sq_cq == rs.lqp.rq_cq:
        #shared completion q between sq/rq
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_cq_pre_qstate, tc.pvtdata.sq_cq_post_qstate, 'proxy_pindex', sq_ring0_mask, (num_sq_completions + num_rq_completions)):
            return False
    else:
        #check completions separately for sq/rq
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_cq_pre_qstate, tc.pvtdata.sq_cq_post_qstate, 'proxy_pindex', sq_ring0_mask, num_sq_completions):
            return False
        
        rs.lqp.rq_cq.qstate.Read()
        tc.pvtdata.rq_cq_post_qstate = rs.lqp.rq_cq.qstate.data
        log_num_cq_wqes = getattr(tc.pvtdata.rq_cq_post_qstate, 'log_num_wqes')
        rq_ring0_mask = (2 ** log_num_cq_wqes) - 1

        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_cq_pre_qstate, tc.pvtdata.rq_cq_post_qstate, 'proxy_pindex', rq_ring0_mask, num_rq_completions):
            return False
        
    return True

def ValidateRespRxCQChecks(tc):
    rs = tc.config.rdmasession
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_post_qstate = rs.lqp.rq_cq.qstate.data
    log_num_cq_wqes = getattr(tc.pvtdata.rq_cq_post_qstate, 'log_num_wqes')
    ring0_mask = (2 ** log_num_cq_wqes) - 1

    # verify that p_index is incremented by 1, as cqwqe is posted
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_cq_pre_qstate, tc.pvtdata.rq_cq_post_qstate, 'proxy_pindex', ring0_mask, 1):
        return False

    # verify that color bit in CQWQE and CQCB are same
    #logger.info('Color from Exp CQ Descriptor: %d' % tc.descriptors.Get('EXP_CQ_DESC').color)
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_cq_post_qstate, 'color', tc.descriptors.Get('EXP_CQ_DESC').color):
        return False

    return True

def ValidateRespRx2CQChecks(tc):
    rs = tc.config.rdmasession
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_post_qstate = rs.lqp.rq_cq.qstate.data
    log_num_cq_wqes = getattr(tc.pvtdata.rq_cq_post_qstate, 'log_num_wqes')
    ring0_mask = (2 ** log_num_cq_wqes) - 1

    # verify that p_index is incremented by 2, as cqwqe is posted
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_cq_pre_qstate, tc.pvtdata.rq_cq_post_qstate, 'proxy_pindex', ring0_mask, 2):
        return False

    # verify that color bit in CQWQE and CQCB are same
    #logger.info('Color from Exp CQ Descriptor: %d' % tc.descriptors.Get('EXP_CQ_DESC').color)
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_cq_post_qstate, 'color', tc.descriptors.Get('EXP_CQ_DESC_2').color):
        return False

    return True

def ValidateReqRxCQChecks(tc, desc_name, num_completions = 1):
    rs = tc.config.rdmasession
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_post_qstate = rs.lqp.sq_cq.qstate.data
    log_num_cq_wqes = getattr(tc.pvtdata.sq_cq_post_qstate, 'log_num_wqes')
    ring0_mask = (2 ** log_num_cq_wqes) - 1

    # verify that p_index is incremented by 1, as cqwqe is posted
    if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_cq_pre_qstate, tc.pvtdata.sq_cq_post_qstate, 'proxy_pindex', ring0_mask, num_completions):
        return False

    # verify that color bit in CQWQE and CQCB are same
    #logger.info('Color from Exp CQ Descriptor: %d' % tc.descriptors.Get('EXP_CQ_DESC').color)
    if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_cq_post_qstate, 'color', tc.descriptors.Get(desc_name).color):
        return False

    return True

#this should be used after driver/DOL synced cindex to HW, by ringing CQ doorbell(ring 0) with set_cindex option
def ValidatePostSyncCQChecks(tc):
    rs = tc.config.rdmasession
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_post_qstate = rs.lqp.sq_cq.qstate.data

    # verify that c_index_0 is set to the same as proxy_pindex
    if not VerifyFieldsEqual(tc, tc.pvtdata.sq_cq_post_qstate, 'c_index0', tc.pvtdata.sq_cq_post_qstate, 'proxy_pindex'):
        return False

#   # verify that p_index_0 is set equal to that of c_index_0
#   if not VerifyFieldsEqual(tc, tc.pvtdata.sq_cq_post_qstate, 'c_index0', tc.pvtdata.sq_cq_post_qstate, 'p_index0'):
#       return False

    # verify that p_index1 is set to that of cindex1 //ARM
    if not VerifyFieldsEqual(tc, tc.pvtdata.sq_cq_post_qstate, 'p_index1', tc.pvtdata.sq_cq_post_qstate, 'c_index1'):
        return False

    # verify that p_index2 is set to that of cindex2 //SARM
    if not VerifyFieldsEqual(tc, tc.pvtdata.sq_cq_post_qstate, 'p_index2', tc.pvtdata.sq_cq_post_qstate, 'c_index2'):
        return False

    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_post_qstate = rs.lqp.rq_cq.qstate.data

    # verify that c_index_0 is set to the same as proxy_pindex
    if not VerifyFieldsEqual(tc, tc.pvtdata.rq_cq_post_qstate, 'c_index0', tc.pvtdata.rq_cq_post_qstate, 'proxy_pindex'):
        return False

#   # verify that p_index_0 is set equal to that of c_index_0
#   if not VerifyFieldsEqual(tc, tc.pvtdata.rq_cq_post_qstate, 'c_index0', tc.pvtdata.rq_cq_post_qstate, 'p_index0'):
#       return False

    # verify that p_index1 is set to that of cindex1 //ARM
    if not VerifyFieldsEqual(tc, tc.pvtdata.rq_cq_post_qstate, 'p_index1', tc.pvtdata.rq_cq_post_qstate, 'c_index1'):
        return False

    # verify that p_index2 is set to that of cindex2 //SARM
    if not VerifyFieldsEqual(tc, tc.pvtdata.rq_cq_post_qstate, 'p_index2', tc.pvtdata.rq_cq_post_qstate, 'c_index2'):
        return False

    return True


def ValidateNoCQChanges(tc):
    rs = tc.config.rdmasession
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_post_qstate = rs.lqp.rq_cq.qstate.data
    log_num_cq_wqes = getattr(tc.pvtdata.rq_cq_post_qstate, 'log_num_wqes')
    ring0_mask = (2 ** log_num_cq_wqes) - 1

    # verify that no change to p_index
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_cq_pre_qstate, tc.pvtdata.rq_cq_post_qstate, 'proxy_pindex', ring0_mask, 0):
        return False
#
#   # verify that no change to c_index
#   if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_cq_pre_qstate, tc.pvtdata.rq_cq_post_qstate, 'c_index0', ring0_mask, 0):
#       return False

    return True

def ValidateTxNoCQChanges(tc):
    rs = tc.config.rdmasession
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_post_qstate = rs.lqp.sq_cq.qstate.data
    log_num_cq_wqes = getattr(tc.pvtdata.sq_cq_post_qstate, 'log_num_wqes')
    ring0_mask = (2 ** log_num_cq_wqes) - 1

    # verify that no change to p_index
    if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_cq_pre_qstate, tc.pvtdata.sq_cq_post_qstate, 'proxy_pindex', ring0_mask, 0):
        return False
#
#   # verify that no change to c_index
#   if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_cq_pre_qstate, tc.pvtdata.sq_cq_post_qstate, 'c_index0', ring0_mask, 0):
#       return False

    return True



############     EQ VALIDATIONS #################
def ValidateEQChecks(tc):
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
    #logger.info('Color from Exp EQ Descriptor: %d' % tc.descriptors.Get('EXP_EQ_DESC').color)
    if not VerifyFieldAbsolute(tc, tc.pvtdata.eq_post_qstate, 'color', tc.descriptors.Get('EXP_EQ_DESC').color):
        return False

    return True


def ValidateNoEQChanges(tc):
    rs = tc.config.rdmasession
    rs.lqp.eq.qstate.Read()
    tc.pvtdata.eq_post_qstate = rs.lqp.eq.qstate.data
    log_num_eq_wqes = getattr(tc.pvtdata.eq_post_qstate, 'log_num_wqes')
    ring0_mask = (2 ** log_num_eq_wqes) - 1

    # verify that no change to p_index
    if not VerifyFieldMaskModify(tc, tc.pvtdata.eq_pre_qstate, tc.pvtdata.eq_post_qstate, 'p_index0', ring0_mask, 0):
        return False

    # verify that no change to c_index
    if not VerifyFieldMaskModify(tc, tc.pvtdata.eq_pre_qstate, tc.pvtdata.eq_post_qstate, 'c_index0', ring0_mask, 0):
        return False

    return True



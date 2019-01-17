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
    # verify that sqcb0 state is now moved to ERR (2)
    if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'state', 2):
        return False

    # verify that sqcb1 state is now moved to ERR (2)
    if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'sqcb1_state', 2):
        return False

    # verify that rqcb0 state is now moved to ERR (2)
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'cb0_state', 2):
        return False

    # verify that rqcb1 state is now moved to ERR (2)
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'cb1_state', 2):
        return False

    return True

def VerifyErrStatistics(tc):
    #verify that qp_err_disabled is set to 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'qp_err_disabled', 1):
        return False

    #verify that tx_qp_err_disabled is set to 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'tx_qp_err_disabled', 1):
        return False

    #verify that tx_qp_err_dis_resp_rx is set to 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'tx_qp_err_dis_resp_rx', 1):
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
    #async eq
    rs.lqp.pd.ep.intf.lif.async_eq.qstate.Read()
    tc.pvtdata.async_eq_pre_qstate = rs.lqp.pd.ep.intf.lif.async_eq.qstate.data
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
    #async eq
    rs.lqp.pd.ep.intf.lif.async_eq.qstate.Read()
    tc.pvtdata.async_eq_post_qstate = rs.lqp.pd.ep.intf.lif.async_eq.qstate.data
    return

def PostToPreCopyQStates(tc):
    rs = tc.config.rdmasession
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.rq_pre_qstate = copy.deepcopy(rs.lqp.rq.qstate.data)
    tc.pvtdata.sq_cq_pre_qstate = copy.deepcopy(rs.lqp.sq_cq.qstate.data)
    tc.pvtdata.rq_cq_pre_qstate = copy.deepcopy(rs.lqp.rq_cq.qstate.data)
    tc.pvtdata.async_eq_pre_qstate = copy.deepcopy(rs.lqp.pd.ep.intf.lif.async_eq.qstate.data)
    return

def ResetErrQState(tc):
    rs = tc.config.rdmasession

    #Reset sq
    rs.lqp.sq.qstate.Read()
    rs.lqp.sq.qstate.data.busy = 0;
    rs.lqp.sq.qstate.data.cb1_busy = 0;
    rs.lqp.sq.qstate.data.in_progress = 0;
    rs.lqp.sq.qstate.data.state = rs.lqp.sq.qstate.data.sqcb1_state = 4 # QP_STATE_RTS
    rs.lqp.sq.qstate.data.sq_cindex = rs.lqp.sq.qstate.data.spec_sq_cindex = rs.lqp.sq.qstate.data.c_index0 = rs.lqp.sq.qstate.data.p_index0;
    rs.lqp.sq.qstate.data.c_index1 = rs.lqp.sq.qstate.data.p_index1;
    rs.lqp.sq.qstate.data.c_index3 = rs.lqp.sq.qstate.data.p_index3;
    rs.lqp.sq.qstate.data.c_index4 = rs.lqp.sq.qstate.data.p_index4
    rs.lqp.sq.qstate.data.sqcb2_c_index4 = rs.lqp.sq.qstate.data.p_index4
    rs.lqp.sq.qstate.data.rrq_spec_cindex = rs.lqp.sq.qstate.data.p_index4
    rs.lqp.sq.qstate.data.rexmit_psn = rs.lqp.sq.qstate.data.tx_psn
    rs.lqp.sq.qstate.data.max_tx_psn = rs.lqp.sq.qstate.data.tx_psn
    rs.lqp.sq.qstate.data.sqcb1_tx_psn = rs.lqp.sq.qstate.data.tx_psn
    rs.lqp.sq.qstate.data.sqcb2_rexmit_psn = rs.lqp.sq.qstate.data.tx_psn
    rs.lqp.sq.qstate.data.msg_psn = 0
    rs.lqp.sq.qstate.data.rrqwqe_num_sges = 0
    rs.lqp.sq.qstate.data.rrqwqe_cur_sge_id = 0
    rs.lqp.sq.qstate.data.rrqwqe_cur_sge_offset = 0
    rs.lqp.sq.qstate.data.msn = ((rs.lqp.sq.qstate.data.ssn - 1) & 0xFFFFFF)
    rs.lqp.sq.qstate.data.max_ssn = rs.lqp.sq.qstate.data.ssn
    rs.lqp.sq.qstate.data.timer_on = 0
    rs.lqp.sq.qstate.WriteWithDelay()

    #Reset Rq
    rs.lqp.rq.qstate.Read()
    rs.lqp.rq.qstate.data.cb0_state = rs.lqp.rq.qstate.data.cb1_state = 4 # QP_STATE_RTS
    rs.lqp.rq.qstate.data.busy = 0
    rs.lqp.rq.qstate.data.cb1_busy = 0
    rs.lqp.rq.qstate.data.in_progress = 0
    rs.lqp.rq.qstate.data.rsq_pindex = rs.lqp.rq.qstate.data.p_index1
    rs.lqp.rq.qstate.data.proxy_cindex = rs.lqp.rq.qstate.data.p_index0
    rs.lqp.rq.qstate.data.spec_cindex = rs.lqp.rq.qstate.data.p_index0
    rs.lqp.rq.qstate.data.token_id = rs.lqp.rq.qstate.data.nxt_to_go_token_id
    rs.lqp.rq.qstate.data.error_disable_qp = 0
    rs.lqp.rq.qstate.data.work_not_done_recirc_cnt = 0
    rs.lqp.rq.qstate.data.max_recirc_cnt_err = 0
    rs.lqp.rq.qstate.data.next_op_type = 0
    rs.lqp.rq.qstate.data.next_pkt_type = 0
    rs.lqp.rq.qstate.data.qp_err_disabled = 0
    rs.lqp.rq.qstate.data.qp_err_dis_svc_type_err = 0
    rs.lqp.rq.qstate.data.qp_err_dis_pyld_len_err = 0
    rs.lqp.rq.qstate.data.qp_err_dis_last_pkt_len_err = 0
    rs.lqp.rq.qstate.data.qp_err_dis_pmtu_err = 0
    rs.lqp.rq.qstate.data.qp_err_dis_opcode_err = 0
    rs.lqp.rq.qstate.data.qp_err_dis_wr_only_zero_len_err = 0
    rs.lqp.rq.qstate.data.qp_err_dis_unaligned_atomic_va_err = 0
    rs.lqp.rq.qstate.data.qp_err_dis_insuff_sge_err = 0
    rs.lqp.rq.qstate.data.qp_err_dis_max_sge_err = 0
    rs.lqp.rq.qstate.data.qp_err_dis_rsvd_key_err = 0
    rs.lqp.rq.qstate.data.qp_err_dis_key_state_err = 0
    rs.lqp.rq.qstate.data.qp_err_dis_key_pd_mismatch = 0
    rs.lqp.rq.qstate.data.qp_err_dis_key_acc_ctrl_err = 0
    rs.lqp.rq.qstate.data.qp_err_dis_key_va_err = 0
    rs.lqp.rq.qstate.data.qp_err_dis_inv_rkey_rsvd_key_err = 0
    rs.lqp.rq.qstate.data.qp_err_dis_ineligible_mr_err = 0
    rs.lqp.rq.qstate.data.qp_err_dis_inv_rkey_state_err = 0
    rs.lqp.rq.qstate.data.qp_err_dis_type1_mw_inv_err = 0
    rs.lqp.rq.qstate.data.qp_err_dis_type2a_mw_qp_mismatch = 0
    rs.lqp.rq.qstate.data.qp_err_dis_mr_mw_pd_mismatch = 0
    rs.lqp.rq.qstate.data.qp_err_dis_mr_state_invalid = 0
    rs.lqp.rq.qstate.data.qp_err_dis_mr_cookie_mismatch = 0
    rs.lqp.rq.qstate.data.last_bth_opcode = 0
    rs.lqp.rq.qstate.data.tx_qp_err_disabled = 0
    rs.lqp.rq.qstate.data.tx_qp_err_dis_rsvd_rkey_err = 0
    rs.lqp.rq.qstate.data.tx_qp_err_dis_rkey_state_err = 0
    rs.lqp.rq.qstate.data.tx_qp_err_dis_rkey_pd_mismatch = 0
    rs.lqp.rq.qstate.data.tx_qp_err_dis_rkey_acc_ctrl_err = 0
    rs.lqp.rq.qstate.data.tx_qp_err_dis_rkey_va_err = 0
    rs.lqp.rq.qstate.data.tx_qp_err_dis_type2a_mw_qp_mismatch = 0
    rs.lqp.rq.qstate.data.tx_qp_err_dis_resp_rx = 0
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

def ValidateAdminCQChecks(tc, desc_name, num_completions = 1):
    tc.pvtdata.aq.cq.qstate.Read()
    tc.pvtdata.aq_cq_post_qstate = tc.pvtdata.aq.cq.qstate.data
    log_num_cq_wqes = getattr(tc.pvtdata.aq_cq_post_qstate, 'log_num_wqes')
    ring0_mask = (2 ** log_num_cq_wqes) - 1

    # verify that p_index is incremented by num_completions, as cqwqe is posted
    if not VerifyFieldMaskModify(tc, tc.pvtdata.aq_cq_pre_qstate, tc.pvtdata.aq_cq_post_qstate, 'proxy_pindex', ring0_mask, num_completions):
        return False

    # verify that color bit in CQWQE and CQCB are same
    #logger.info('Color from Exp CQ Descriptor: %d' % tc.descriptors.Get('EXP_CQ_DESC').color)
    if not VerifyFieldAbsolute(tc, tc.pvtdata.aq_cq_post_qstate, 'color', tc.descriptors.Get(desc_name).color):
        return False

    return True

#these should be used after driver/DOL synced cindex to HW, by ringing CQ doorbell(ring 0) with set_cindex option
def ValidatePostSyncAdminCQChecks(tc):
    tc.pvtdata.aq.cq.qstate.Read()
    tc.pvtdata.aq_cq_post_qstate = tc.pvtdata.aq.cq.qstate.data

    # verify that c_index_0 is set to the same as proxy_pindex
    if not VerifyFieldsEqual(tc, tc.pvtdata.aq_cq_post_qstate, 'c_index0', tc.pvtdata.aq_cq_post_qstate, 'proxy_pindex'):
        return False

    # verify that p_index1 is set to that of cindex1 //ARM
    if not VerifyFieldsEqual(tc, tc.pvtdata.aq_cq_post_qstate, 'p_index1', tc.pvtdata.aq_cq_post_qstate, 'c_index1'):
        return False

    # verify that p_index2 is set to that of cindex2 //SARM
    if not VerifyFieldsEqual(tc, tc.pvtdata.aq_cq_post_qstate, 'p_index2', tc.pvtdata.aq_cq_post_qstate, 'c_index2'):
        return False

    return True

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

def ValidateAsyncEQChecks(tc, num_wqes=1):
    rs = tc.config.rdmasession
    rs.lqp.pd.ep.intf.lif.async_eq.qstate.Read()
    tc.pvtdata.async_eq_post_qstate = rs.lqp.pd.ep.intf.lif.async_eq.qstate.data
    log_num_eq_wqes = getattr(tc.pvtdata.async_eq_post_qstate, 'log_num_wqes')
    ring0_mask = (2 ** log_num_eq_wqes) - 1

    # verify that p_index is incremented by num_wqes(default:1), as eqwqe is posted
    if not VerifyFieldMaskModify(tc, tc.pvtdata.async_eq_pre_qstate, tc.pvtdata.async_eq_post_qstate, 'p_index0', ring0_mask, num_wqes):
        return False

    # verify that c_index is incremented by num_wqes(default:1), as eqwqe is consumed
    if not VerifyFieldMaskModify(tc, tc.pvtdata.async_eq_pre_qstate, tc.pvtdata.async_eq_post_qstate, 'c_index0', ring0_mask, num_wqes):
        return False

    return True

def PopulateAdminPreQStates(tc):
    lif = tc.config.root
    tc.pvtdata.aq = lif.aq

    #aq
    lif.aq.aq.qstate.Read()
    tc.pvtdata.aq_pre_qstate = copy.deepcopy(lif.aq.aq.qstate.data)

    #aq_cq
    lif.aq.cq.qstate.Read()
    tc.pvtdata.aq_cq_pre_qstate = copy.deepcopy(lif.aq.cq.qstate.data)

    #aq_eq
    lif.aq.eq.qstate.Read()
    tc.pvtdata.eq_pre_qstate = copy.deepcopy(lif.aq.eq.qstate.data)

    #async_eq
    lif.async_eq.qstate.Read()
    tc.pvtdata.async_eq_pre_qstate = lif.async_eq.qstate.data
    return

def PopulateAdminPostQStates(tc):
    lif = tc.config.root
    tc.pvtdata.aq = lif.aq

    #aq
    lif.aq.aq.qstate.Read()
    tc.pvtdata.aq_post_qstate = copy.deepcopy(lif.aq.aq.qstate.data)

    #aq_cq
    lif.aq.cq.qstate.Read()
    tc.pvtdata.aq_cq_post_qstate = copy.deepcopy(lif.aq.cq.qstate.data)

    #async_eq
    lif.async_eq.qstate.Read()
    tc.pvtdata.async_eq_post_qstate = lif.async_eq.qstate.data
    return

def ValidateAdminAsyncEQChecks(tc, num_wqes=1):
    lif = tc.config.root
    lif.async_eq.qstate.Read()
    tc.pvtdata.async_eq_post_qstate = lif.async_eq.qstate.data
    log_num_eq_wqes = getattr(tc.pvtdata.async_eq_post_qstate, 'log_num_wqes')
    ring0_mask = (2 ** log_num_eq_wqes) - 1

    # verify that p_index is incremented by num_wqes(default:1), as eqwqe is posted
    if not VerifyFieldMaskModify(tc, tc.pvtdata.async_eq_pre_qstate, tc.pvtdata.async_eq_post_qstate, 'p_index0', ring0_mask, num_wqes):
        return False

    # verify that c_index is incremented by num_wqes(default:1), as eqwqe is consumed
    if not VerifyFieldMaskModify(tc, tc.pvtdata.async_eq_pre_qstate, tc.pvtdata.async_eq_post_qstate, 'c_index0', ring0_mask, num_wqes):
        return False

    return True

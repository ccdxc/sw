#! /usr/bin/python3

from test.rdma.utils import *
import pdb
import copy
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger

def Setup(infra, module):
    if GlobalOptions.perf:
        module.testspec.selectors.SetMaxRdmaSessions(8)
    iterelem = module.iterator.Get()

    logger.info("Iterator Selectors")

    module.pvtdata.payloadsize = 32 # for inline data
    if iterelem:
        if 'rdmasession' in iterelem.__dict__:
            if 'base' in iterelem.rdmasession.__dict__:
                logger.info("- rdmasession.base: %s" % iterelem.rdmasession.base)
                module.testspec.selectors.rdmasession.base.Extend(iterelem.rdmasession.base)

        if 'payloadsize' in iterelem.__dict__:
            module.pvtdata.payloadsize = iterelem.payloadsize
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    gftflow = tc.config.rdmasession.session.iflow.GetGftFlow()
    
    rs = tc.config.rdmasession


    # TX SIDE STUFF

    # Read local QP SQ pre state
    rs.lqp.sq.qstate.Read()
    #overload timestamp with flow-index only for GFT case.
    if (gftflow != None): 
        rs.lqp.sq.qstate.data.timestamp = gftflow.flow_index 
        rs.lqp.sq.qstate.WriteWithDelay()
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.msn = (tc.pvtdata.sq_pre_qstate.msn + 1)

    # DO NOT ARM CQ and Set EQ's CI=PI for EQ enablement
    #rs.lqp.sq_cq.qstate.ArmCq()
    #rs.lqp.eq.qstate.reset_cindex(0)

    # Read CQ pre state
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_pre_qstate = rs.lqp.sq_cq.qstate.data

    # RX SIDE STUFF

    # Read RQ pre state
    rs.rqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = rs.rqp.rq.qstate.data

    # Read CQ pre state
    rs.rqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_pre_qstate = rs.rqp.rq_cq.qstate.data


    # Read payloadsize from module into tc pvtdata, which is needed 
    # for non-inline tests to sweep across diff pkt sizes for latency RTL tests
    tc.pvtdata.payloadsize = tc.module.pvtdata.payloadsize
    tc.pvtdata.num_total_bytes = tc.pvtdata.payloadsize
    logger.info("RDMA TestCaseSetup() Module payloadsize: %d" % tc.pvtdata.payloadsize)

    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    logger.info("RDMA TestCaseVerify() Implementation.")
    return True

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    ring0_mask = (rs.lqp.num_sq_wqes - 1)
    tc.pvtdata.sq_post_qstate = rs.lqp.sq.qstate.data

    if step.step_id == 0:
        logger.info("RDMA TestCaseVerify() Step 0 : Nothing to be done")

    elif step.step_id == 1:
        logger.info("RDMA TestCaseVerify() Step 1")
        num_pkts = (int)(((tc.pvtdata.payloadsize - 1) / rs.lqp.pmtu) + 1)
        logger.info("KSMURTY num_pkts: %d" % num_pkts)

        # verify that tx_psn is incremented by num_pkts
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', num_pkts):
            return False
        
        # verify that p_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'p_index0', ring0_mask,  1):
            return False
        
        # verify that c_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'c_index0', ring0_mask, 1):
            return False
        
        # verify that ssn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'ssn', 1):
            return False
        
        # verify that lsn is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'lsn', 127, 1):
            return False
        
        # verify that p_index of rrq is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index5', tc.pvtdata.sq_post_qstate, 'p_index5'):
            return False

        # verify that c_index of rrq is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'c_index5', tc.pvtdata.sq_post_qstate, 'c_index5'):
            return False

        msn = tc.pvtdata.sq_pre_qstate.ssn
        # verify that msn is incremented to that of ssn of this msg
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'msn', msn):
            return False

        # verify rexmit_psn is incremented to that of tx_psn
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_post_qstate, 'rexmit_psn', tc.pvtdata.sq_post_qstate, 'tx_psn'):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False

        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'token_id', 1):
            return False

        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'nxt_to_go_token_id', 1):
            return False

        if not ValidateReqRxCQChecks(tc, 'EXP_CQ_DESC'):
            return False 
        ############     SKIP EQ VALIDATIONS #################
        #if not ValidateEQChecks(tc):
        #    return False

        # RQ Validations
        rs.rqp.rq.qstate.Read()
        ring0_mask = (rs.rqp.num_rq_wqes - 1)
        tc.pvtdata.rq_post_qstate = rs.rqp.rq.qstate.data

        ############     RQ VALIDATIONS #################
        # verify that e_psn is incremented by num_pkts
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', num_pkts):
            return False

        # verify that proxy_cindex is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 1):
            return False

        # verify that token_id is incremented by num_pkts
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', num_pkts):
            return False

        # verify that nxt_to_go_token_id is incremented by num_pkts
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', num_pkts):
            return False

        ############     RQ STATS VALIDATIONS #################
        # verify that num_pkts is incremented by num_pkts
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_pkts', num_pkts):
            return False

        # verify that num_bytes is incremented by pvtdata.num_total_bytes
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_bytes', tc.pvtdata.num_total_bytes):
            return False

        # verify that num_send_msgs is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_send_msgs', 1):
            return False

        # verify that num_pkts_in_cur_msg is num_pkts
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'num_pkts_in_cur_msg', num_pkts):
            return False

        # verify that max_pkts_in_any_msg is num_pkts
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'max_pkts_in_any_msg', max([num_pkts, tc.pvtdata.rq_pre_qstate.max_pkts_in_any_msg])):
            return False

        ############     CQ VALIDATIONS #################
        if not ValidateRespRxCQChecks(tc):
            return False

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    return

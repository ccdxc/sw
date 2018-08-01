#! /usr/bin/python3

from test.rdma.utils import *
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger
from config.objects.rdma.keytable import *

def Setup(infra, module):
    iterelem = module.iterator.Get()

    logger.info("Iterator Selectors")

    if iterelem:
        if 'base' in iterelem.rdmasession.__dict__:
            logger.info("- rdmasession.base: %s" % iterelem.rdmasession.base)
            module.testspec.selectors.rdmasession.base.Extend(iterelem.rdmasession.base)
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession

    tc.pvtdata.num_total_bytes = 0x40

    # Read RQ pre state
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data

    # Read CQ pre state
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_pre_qstate = rs.lqp.rq_cq.qstate.data

    tc.pvtdata.slab = rs.lqp.pd.ep.GetNewSlab()
    tc.pvtdata.mr = rs.lqp.pd.mrs.Get('MR-' + tc.pvtdata.slab.GID())
    tc.pvtdata.l_key = tc.pvtdata.mr.lkey
    kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.l_key)
    # set the state of the MR l_key to invalid
    kt_entry.data.state = 0 
    kt_entry.WriteWithDelay();
    logger.info("RDMA TestCaseSetup(): Lkey state for SLAB of hw_lif %d qp %s lkey: %d state: %d" %
            (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.l_key, kt_entry.data.state))
   
    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_post_qstate = rs.lqp.rq.qstate.data

    if step.step_id == 0:
    
        ############     RQ VALIDATIONS #################
        # verify that e_psn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 1):
            return False
    
        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 1):
            return False
    
        # verify that nxt_to_go_token_id is NOT incremented
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 0):
            return False
    
        # verify that state is now moved to ERR (2)
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'cb1_state', 2):
            return False
    
        ############     RQ STATS VALIDATIONS #################
        # verify that num_pkts is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_pkts', 1):
            return False
    
        # verify that num_bytes is incremented by pvtdata.num_total_bytes
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_bytes', tc.pvtdata.num_total_bytes):
            return False
    
        # verify that num_send_msgs is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_send_msgs', 1):
            return False
    
        # verify that num_pkts_in_cur_msg is 1
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'num_pkts_in_cur_msg', 1):
            return False
    
        # verify that max_pkts_in_any_msg is 1
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'max_pkts_in_any_msg', max([1, tc.pvtdata.rq_pre_qstate.max_pkts_in_any_msg])):
            return False
    
        ############     CQ VALIDATIONS #################
        if not ValidateRespRxCQChecks(tc):
            return False

    elif step.step_id == 1:

        if not ValidatePostSyncCQChecks(tc):
            return False 

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    rs = tc.config.rdmasession
    logger.info("Setting proxy_cindex/spec_cindex equal to p_index0\n")
    rs.lqp.rq.qstate.data.proxy_cindex = tc.pvtdata.rq_post_qstate.p_index0;
    rs.lqp.rq.qstate.data.spec_cindex = tc.pvtdata.rq_post_qstate.p_index0;
    rs.lqp.rq.qstate.data.cb0_state = rs.lqp.rq.qstate.data.cb1_state = 6;
    rs.lqp.rq.qstate.data.token_id = rs.lqp.rq.qstate.data.nxt_to_go_token_id;
    rs.lqp.rq.qstate.WriteWithDelay();
    return

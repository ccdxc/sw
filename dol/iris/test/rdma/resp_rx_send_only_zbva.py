#! /usr/bin/python3

from iris.test.rdma.utils import *
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger
from iris.config.objects.rdma.keytable import *

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
    PopulatePreQStates(tc)

    rs = tc.config.rdmasession
    tc.pvtdata.num_total_bytes = 0x40
    
    tc.pvtdata.slab = rs.lqp.pd.ep.GetNewSlab()
    tc.pvtdata.mr = rs.lqp.pd.mrs.Get('MR-' + tc.pvtdata.slab.GID())
    tc.pvtdata.l_key = tc.pvtdata.mr.lkey
    tc.pvtdata.r_key = tc.pvtdata.mr.rkey

    tc.pvtdata.user_key = 132

    kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.l_key)
    # change the flags of the MR lkey to have zbva
    kt_entry.data.flags = kt_entry.data.flags | 4
    kt_entry.WriteWithDelay();

    tc.pvtdata.base_va = kt_entry.data.base_va
    # set zbva
    tc.pvtdata.mr_zbva = tc.pvtdata.slab.address - tc.pvtdata.base_va
    # va is base_va + zbva
    tc.pvtdata.mr_va = tc.pvtdata.base_va + tc.pvtdata.mr_zbva
    logger.info("RDMA TestCaseVerify(): base_va: %d slab_address %d zbva: %d" %
                    (tc.pvtdata.base_va, tc.pvtdata.slab.address, tc.pvtdata.mr_zbva))

    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    ring0_mask = (rs.lqp.num_rq_wqes - 1)
    PopulatePostQStates(tc)

    if step.step_id == 0:
    
        ############     RQ VALIDATIONS #################
        # verify that e_psn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 1):
            return False
    
        # verify that proxy_cindex is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 1):
            return False
    
        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 1):
            return False
    
        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 1):
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
    return

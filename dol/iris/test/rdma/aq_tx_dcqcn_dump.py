#! /usr/bin/python3

from iris.test.rdma.utils import *
from iris.config.objects.qp import RdmaDCQCNstate
import pdb
import copy
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger

def Setup(infra, module):
    pass

def Teardown(infra, module):
    pass

def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession

    tc.pvtdata.lif = rs.lqp.pd.ep.intf.lif
    tc.pvtdata.aq = tc.pvtdata.lif.aq
    PopulateAdminPreQStates(tc)

    rs.lqp.ReadDcqcnCb()
    tc.pvtdata.pre_dcqcn_cb = copy.deepcopy(rs.lqp.dcqcn_data)

    rs.lqp.dcqcn_data = RdmaDCQCNstate(bytes(range(64)))
    rs.lqp.WriteDcqcnCb()

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")

def TestCaseVerify(tc):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")

    dump = tc.buffers.Get('DUMP_BUF')
    dump.Read()

    VerifyEqual(tc, 'dump data', bytes(dump.data[:64]), bytes(range(64)))

    return True

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")

    tc.pvtdata.aq.aq.qstate.Read()
    ring0_mask = (tc.pvtdata.aq.num_aq_wqes - 1)
    tc.pvtdata.aq_post_qstate = tc.pvtdata.aq.aq.qstate.data
    if step.step_id == 1:
        # verify that token_id is equal to next_token_id
        if not VerifyFieldsEqual(tc, tc.pvtdata.aq_post_qstate, 'token_id', tc.pvtdata.aq_post_qstate, 'next_token_id'):
            return False

        # verify that p_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.aq_pre_qstate, tc.pvtdata.aq_post_qstate, 'p_index0', ring0_mask,  1):
            return False

        # verify that c_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.aq_pre_qstate, tc.pvtdata.aq_post_qstate, 'c_index0', ring0_mask, 1):
            return False

        if not ValidateAdminCQChecks(tc, 'EXP_CQ_DESC'):
            return False

    elif step.step_id == 2:

        if not ValidatePostSyncAdminCQChecks(tc):
            return False

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")

    rs = tc.config.rdmasession

    rs.lqp.dcqcn_data = tc.pvtdata.pre_dcqcn_cb
    rs.lqp.WriteDcqcnCb()

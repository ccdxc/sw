#! /usr/bin/python3

from iris.test.rdma.utils import *
import pdb
import copy
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger

def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession

    tc.pvtdata.lif = rs.lqp.pd.ep.intf.lif
    tc.pvtdata.aq = tc.pvtdata.lif.aq
    PopulateAdminPreQStates(tc)

    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")

    stat_hdrs = tc.buffers.Get('AQ_STATS_BUF')
    stat_hdrs.Read()

    def stat_hdr(stat_i):
        start = stat_i * 32
        end = start + 32
        return bytes(stat_hdrs.data[start:end])

    hw_stats_count = 41

    if not VerifyEqual(tc, 'first stat', stat_hdr(0),
            b'\x40\x00\x01\x80tx_rdma_ucast_bytes\0\0\0\0\0\0\0\0\0'):
        return False

    if not VerifyEqual(tc, 'last stat', stat_hdr(hw_stats_count - 1),
            b'\x40\x00\x03\xf0resp_tx_loc_sgl_inv_err\0\0\0\0\0'):
        return False

    if not VerifyEqual(tc, 'sentry', stat_hdr(hw_stats_count), b'\0' * 32):
        return False

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
    return

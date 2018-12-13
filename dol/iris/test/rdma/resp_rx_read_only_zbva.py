#! /usr/bin/python3

import copy
from iris.test.rdma.utils import *
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger
from iris.config.objects.rdma.keytable import *

def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    PopulatePreQStates(tc)

    rs = tc.config.rdmasession
    tc.pvtdata.num_total_bytes = 0x40

    tc.pvtdata.dst_qp = tc.config.rdmasession.rqp.id
    tc.pvtdata.wrid = 0x0905

    tc.pvtdata.slab = rs.lqp.pd.ep.GetNewSlab()
    tc.pvtdata.mr = rs.lqp.pd.mrs.Get('MR-' + tc.pvtdata.slab.GID())
    tc.pvtdata.l_key = tc.pvtdata.mr.lkey
    tc.pvtdata.r_key = rs.lqp.pd.GetNewType2MW().rkey
    tc.pvtdata.mw_kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.r_key)
    tc.pvtdata.mr_kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, (tc.pvtdata.l_key))

    tc.pvtdata.user_key = 132

    tc.pvtdata.mw_va = 0

    kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.r_key)
    # set zbva 
    tc.pvtdata.mw_zbva = 223

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
    PopulatePostQStates(tc)

    rs = tc.config.rdmasession
    ring0_mask = (rs.lqp.num_rq_wqes - 1)

    if step.step_id == 0:
        kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.r_key)
        # change the acc_ctrl of the MW rkey to have zbva
        kt_entry.data.acc_ctrl = kt_entry.data.acc_ctrl |0x20 #ACC_CTRL_ZERO_BASED
        kt_entry.WriteWithDelay();
        logger.info("RDMA TestCaseVerify(): base_va: %d slab_address %d zbva: %d" %
                        (kt_entry.data.base_va, tc.pvtdata.slab.address, tc.pvtdata.mw_zbva))

    elif step.step_id == 1:

        ############     RQ VALIDATIONS #################
        # verify that e_psn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 1):
            return False

        # verify that p_index is not incremented
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'p_index0', ring0_mask,  0):
            return False

        # verify that proxy_cindex is not incremented
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask,  0):
            return False

        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 1):
            return False

        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 1):
            return False

        ############     RSQ VALIDATIONS #################
        ring1_mask = (rs.lqp.num_rsq_wqes - 1)
        # verify that p_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'p_index1', ring1_mask,  1):
            return False

        # verify that c_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'c_index1', ring1_mask,  1):
            return False

        ############     RQ STATS VALIDATIONS #################
        # verify that num_pkts is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_pkts', 1):
            return False

        # verify that num_bytes is incremented by 0
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_bytes', 0):
            return False

        # verify that num_read_req_msgs is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_read_req_msgs', 1):
            return False

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    if (GlobalOptions.dryrun): return
    rs = tc.config.rdmasession
    kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.l_key)
    kt_entry.data = tc.pvtdata.mr_kt_entry.data
    kt_entry.WriteWithDelay()

    kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, (tc.pvtdata.r_key & 0xFFFFFF))
    kt_entry.data = tc.pvtdata.mw_kt_entry.data
    kt_entry.WriteWithDelay()
    return

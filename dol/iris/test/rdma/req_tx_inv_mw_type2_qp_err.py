#! /usr/bin/python3

from iris.test.rdma.utils import *
import pdb
import copy
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger
from iris.config.objects.rdma.keytable import *
import infra.common.objects as objects

            
def Setup(infra, module): 
    return

def Teardown(infra, module):
    return      
        
def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    PopulatePreQStates(tc)

    rs = tc.config.rdmasession
    tc.pvtdata.dst_qp = tc.config.rdmasession.rqp.id
    tc.pvtdata.wrid = 0x0905

    tc.pvtdata.slab = rs.lqp.pd.ep.GetNewSlab()
    tc.pvtdata.mr = rs.lqp.pd.mrs.Get('MR-' + tc.pvtdata.slab.GID())
    tc.pvtdata.l_key = tc.pvtdata.mr.lkey
    tc.pvtdata.r_key = rs.lqp.pd.GetNewType2MW().rkey
    tc.pvtdata.mw_kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, (tc.pvtdata.r_key & 0xFFFFFF))
    tc.pvtdata.mr_kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, (tc.pvtdata.l_key))
    tc.pvtdata.user_key = 132
        
    if (GlobalOptions.dryrun):
        tc.pvtdata.mw_va = 0
        return True
    else:
        tc.pvtdata.mw_va = tc.pvtdata.slab.address + 1089

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
    ring0_mask = (rs.lqp.num_sq_wqes - 1)
    
    if step.step_id == 0:
        # verify that tx_psn is not incremented 
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', 0):
            return False

        # verify that p_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'p_index0', ring0_mask,  1):
            return False

        # verify that c_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'c_index0', ring0_mask, 1):
            return False

        # verify that ssn is not incremented
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'ssn', 0):
            return False

        # verify that lsn is not incremented
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'lsn', 0):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False

        # validate cqcb pindex and color
        if not ValidateReqRxCQChecks(tc, 'EXP_CQ_DESC'):
            return False

        ###########   Key Invalidation checks ##########
        # read the key table entry for rkey and verify if its valid
        mw_kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, (tc.pvtdata.r_key & 0xFFFFFF))
        mr_kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, (tc.pvtdata.l_key & 0xFFFFFF))

        if ((mw_kt_entry.data.state != 2) or
            (mw_kt_entry.data.type != 2) or
            (mw_kt_entry.data.pt_base != mr_kt_entry.data.pt_base) or
            (mw_kt_entry.data.base_va != tc.pvtdata.mw_va) or
            (mw_kt_entry.data.log_page_size != mr_kt_entry.data.log_page_size) or
            (mw_kt_entry.data.user_key != tc.pvtdata.user_key) or
            (mw_kt_entry.data.qp != rs.lqp.id) or
            (mw_kt_entry.data.mr_l_key != tc.pvtdata.l_key) or
            (mw_kt_entry.data.mr_cookie != mr_kt_entry.data.mr_cookie)) :
            logger.info("RDMA TestCaseVerify(): Bind MW Rkey fails for hw_lif %d qp %s rkey %d " %
                    (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.r_key))
            return False

        logger.info("RDMA TestCaseVerify(): Rkey is bound for hw_lif %d qp %s rkey %d" %
                    (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.r_key))

        # Set QP of MW to invalid value to simulate qp-check-failure.
        kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.r_key)
        kt_entry.data.qp = 0xFFFFFF
        kt_entry.WriteWithDelay()


    elif step.step_id == 1:

        # verify that tx_psn is not incremented 
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', 0):
            return False

        # verify that p_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'p_index0', ring0_mask,  1):
            return False

        # verify that c_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'c_index0', ring0_mask, 1):
            return False

        # verify that ssn is not incremented
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'ssn', 0):
            return False

        # verify that lsn is not incremented
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'lsn', 0):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False

        # There will be two completions. One in sq_cq for actual error and another in
        # rq_cq for flush error
        if not ValidateCQCompletions(tc, 1, 1):
            return False

        # verify that state is now moved to ERR (2)
        if not VerifyErrQState(tc):
            return False

        ###########   Key Invalidation checks ##########
        # read the key table entry for rkey and verify if its valid
        mw_kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, (tc.pvtdata.r_key & 0xFFFFFF))
        mr_kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, (tc.pvtdata.l_key & 0xFFFFFF))

        if ((mw_kt_entry.data.state != 2) or
            (mw_kt_entry.data.type != 2) or
            (mw_kt_entry.data.pt_base != mr_kt_entry.data.pt_base) or
            (mw_kt_entry.data.base_va != tc.pvtdata.mw_va) or
            (mw_kt_entry.data.log_page_size != mr_kt_entry.data.log_page_size) or
            (mw_kt_entry.data.user_key != tc.pvtdata.user_key) or
            (mw_kt_entry.data.mr_l_key != tc.pvtdata.l_key) or
            (mw_kt_entry.data.mr_cookie != mr_kt_entry.data.mr_cookie)) :
            logger.info("RDMA TestCaseVerify(): Bind MW Rkey state changed after LI for hw_lif %d qp %s rkey %d " %
                    (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.r_key))
            return False

        logger.info("RDMA TestCaseVerify(): Rkey state not changed after LI for hw_lif %d qp %s rkey %d" %
                (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.r_key))

    elif step.step_id == 2:
        if not ValidatePostSyncCQChecks(tc):
            return False

    #TODO check for error-disabled QP
    
    PostToPreCopyQStates(tc)
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
    ResetErrQState(tc)
    return

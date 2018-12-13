#! /usr/bin/python3

from iris.test.rdma.utils import *
import pdb
import copy
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
    tc.pvtdata.dst_qp = tc.config.rdmasession.rqp.id
    tc.pvtdata.wrid = 0x0504
    
    tc.pvtdata.r_key = rs.lqp.pd.GetNewType2MW().rkey
    tc.pvtdata.mw_kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, (tc.pvtdata.r_key & 0xFFFFFF))

    return

def TestCasePreTrigger(tc):
    if (GlobalOptions.dryrun): return
    rs = tc.config.rdmasession
    kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.r_key)
    logger.info("RDMA TestCaseSetup(): Lkey state for SLAB0 of hw_lif %d qp %s rkey: %d state: %d" %
            (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.r_key, kt_entry.data.state))
    # Make sure that rkey is in free state before the test
    assert (kt_entry.data.state == 1) # KEY_STATE_FREE = 1

     # Set PD of rkey to invalid value to simulate 'invalidation via qp belonging to different PD'.
    kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.r_key)
    kt_entry.data.pd = 0xFFFFFFFF
    kt_entry.WriteWithDelay()

    logger.info("RDMA TestCasePreTrigger() Implementation.")
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
    tc.pvtdata.sq_post_qstate = rs.lqp.sq.qstate.data

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

        # verify that lsn is not incremented for LI
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
        # read the key table entry for the lkey being invalidated 
        kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.r_key)

        if not (kt_entry.data.state == 1): # KEY_STATE_FREE = 1
            logger.info("RDMA TestCaseVerify(): Rkey invalidated fails for hw_lif %d qp %s rkey %d " %
                    (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.r_key))
            logger.info("RDMA TestCaseVerify(): Invalidated Rkey is not in Free state: state %d" %
                    kt_entry.data.state)
            return False

        logger.info("RDMA TestCaseVerify(): Rkey is invalidated for hw_lif %d qp %s rkey %d" %
                (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.r_key))

        #TODO check for error-disabled QP

    elif step.step_id == 1:
        if not ValidatePostSyncCQChecks(tc):
            return False

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    if (GlobalOptions.dryrun): return
    rs = tc.config.rdmasession
    kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.r_key)
    kt_entry.data = tc.pvtdata.mw_kt_entry.data
    kt_entry.WriteWithDelay()
    ResetErrQState(tc)
    return

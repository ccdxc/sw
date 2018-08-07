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
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.dst_qp = tc.config.rdmasession.rqp.id
    tc.pvtdata.wrid_1 = 0x0807
    tc.pvtdata.wrid_2 = 0x0102

    # Read CQ pre state
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_pre_qstate = rs.lqp.sq_cq.qstate.data

    return

def TestCasePreTrigger(tc):
    if (GlobalOptions.dryrun): return
    rs = tc.config.rdmasession
    slab_id = tc.buffers.Get('BUF').slab_id
    tc.pvtdata.inv_l_key = tc.config.rdmasession.lqp.pd.mrs.Get('MR-' + slab_id.GID()).lkey
    kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.inv_l_key)
    logger.info("RDMA TestCaseSetup(): Lkey state for SLAB0 of hw_lif %d qp %s rkey: %d state: %d" %
            (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.inv_l_key, kt_entry.data.state))
    # Make sure that lkey is valid before the test
    assert (kt_entry.data.state == 2) # KEY_STATE_VALID = 2

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
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    ring0_mask = (rs.lqp.num_sq_wqes - 1)
    tc.pvtdata.sq_post_qstate = rs.lqp.sq.qstate.data

    if step.step_id == 0:

       # verify that tx_psn is not incremented 
       if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', 0):
           return False

       # verify that p_index is incremented by 2
       if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'p_index0', ring0_mask,  2):
           return False

       # verify that c_index is incremented by 2
       if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'c_index0', ring0_mask, 2):
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

       # verify that timestamp is not incremented since LI fence check has not failed coz tx_psn = rexmit_psn
       if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'timestamp', 0):
           return False

       # verify that li_fence bit is 0
       if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'li_fence', 0):
           return False

       # verify that fence_done is set
       if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'fence_done', 1):
           return False

       # CQ checks       
       if not ValidateReqRxCQChecks(tc, 'EXP_CQ_DESC_1', 2):
           return False

       if not ValidateReqRxCQChecks(tc, 'EXP_CQ_DESC_2', 2):
           return False

       ###########   Key Invalidation checks ##########
       # read the key table entry for the lkey being invalidated 
       kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.inv_l_key)

       if not (kt_entry.data.state == 1): # KEY_STATE_FREE = 1
           logger.info("RDMA TestCaseVerify(): Lkey invalidated fails for hw_lif %d qp %s lkey %d " %
                   (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.inv_l_key))
           logger.info("RDMA TestCaseVerify(): Invalidated Lkey is not in Free state: state %d" %
                   kt_entry.data.state)
           return False

       logger.info("RDMA TestCaseVerify(): Lkey is invalidated for hw_lif %d qp %s lkey %d" %
               (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.inv_l_key))


    elif step.step_id == 1:

        if not ValidatePostSyncCQChecks(tc):
            return False

    # update current as pre_qstate ... so next step_id can use it as pre_qstate
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.sq_cq_pre_qstate = copy.deepcopy(rs.lqp.sq_cq.qstate.data)

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    return

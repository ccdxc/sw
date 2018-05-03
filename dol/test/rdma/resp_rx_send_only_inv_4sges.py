#! /usr/bin/python3

from test.rdma.utils import *

from config.objects.rdma.keytable import *                                                                    
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger
def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession

    # Read RQ pre state
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data
    tc.pvtdata.send_only_psn = tc.pvtdata.rq_pre_qstate.e_psn

    # Read CQ pre state
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_pre_qstate = rs.lqp.rq_cq.qstate.data

    # Invalidate Rkey: Invalidate the Lkey of the RQ slab (lkey is in MR)
    tc.pvtdata.inv_r_key = rs.lqp.pd.mrs.Get('MR-SLAB0000').rkey
    kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.inv_r_key)
    logger.info("RDMA TestCaseSetup(): Lkey state for SLAB0 of hw_lif %d qp %s rkey: %d state: %d" %
            (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.inv_r_key, kt_entry.data.state))
    if (GlobalOptions.dryrun): return
    # Make sure that lkey is valid before the test
    assert (kt_entry.data.state == 2) # KEY_STATE_VALID = 2
    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.Read()
    ring0_mask = (rs.lqp.num_rq_wqes - 1)
    tc.pvtdata.rq_post_qstate = rs.lqp.rq.qstate.data

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

    ############     CQ VALIDATIONS #################
    if not ValidateRespRxCQChecks(tc):
        return False

    ###########   Key Invadlidation checks ### #####
    # read the key table entry for the rkey being invalidated 
    kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.inv_r_key)

    if not (kt_entry.data.state == 1): # KEY_STATE_FREE = 1
        logger.info("RDMA TestCaseVerify(): Rkey invalidated fails for hw_lif %d qp %s rkey %d " %
                (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.inv_r_key))
        logger.info("RDMA TestCaseVerify(): Invalidated rkey is not in Free state: state %d" %
                kt_entry.data.state)
        return False

    logger.info("RDMA TestCaseVerify(): Rkey is invalidated for hw_lif %d qp %s rkey %d" %
            (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.inv_r_key))
    # validate the key again for further tests
    kt_entry.data.state = 2
    kt_entry.WriteWithDelay()
    if not (kt_entry.data.state == 2): # KEY_STATE_VALID = 2
        logger.info("RDMA TestCaseVerify(): Unable to set Rkey to valid state for hw_lif %d qp %s rkey %d " %
                (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.inv_r_key))
        logger.info("RDMA TestCaseVerify(): Rkey current state: state %d" %
                kt_entry.data.state)
        return False


    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    return

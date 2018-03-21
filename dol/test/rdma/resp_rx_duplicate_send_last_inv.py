#! /usr/bin/python3

from test.rdma.utils import *
from infra.common.glopts import GlobalOptions
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

    #Set the aeth and ack_nak_psn fields in rqcb as if these are the
    #previously recorded aeth values
    rs.lqp.rq.qstate.data.ack_nak_psn = (rs.lqp.rq.qstate.data.e_psn - 5) & 0xFFFFFF
    rs.lqp.rq.qstate.data.aeth_msn = (rs.lqp.rq.qstate.data.msn - 3) & 0xFFFFFF
    rs.lqp.rq.qstate.data.aeth_syndrome = (0 << 5) | 13
    rs.lqp.rq.qstate.WriteWithDelay();

    # Read RQ pre state
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data

    # ARM CQ and Set EQ's CI=PI for EQ enablement
    rs.lqp.rq_cq.qstate.ArmCq()
    rs.lqp.eq.qstate.reset_cindex(0)

    # Read CQ pre state
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_pre_qstate = rs.lqp.rq_cq.qstate.data

    # Read EQ pre state
    rs.lqp.eq.qstate.Read()
    tc.pvtdata.eq_pre_qstate = rs.lqp.eq.qstate.data

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
    # verify that e_psn is NOT incremented
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 0):
        return False

    # verify that msn is NOT incremented
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'msn', 0):
        return False

    # verify that proxy_cindex is NOT incremented  (i.e., receive buffer is not checked out)
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 0):
        return False

    # verify that token_id is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 1):
        return False

    # verify that nxt_to_go_token_id is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 1):
        return False

    # verify that busy is 0
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'busy', 0):
        return False

    ############     CQ VALIDATIONS #################
    rs.lqp.rq_cq.qstate.Read()
    ring0_mask = (rs.lqp.num_rq_wqes - 1)
    tc.pvtdata.rq_cq_post_qstate = rs.lqp.rq_cq.qstate.data
    log_num_cq_wqes = getattr(tc.pvtdata.rq_cq_post_qstate, 'log_num_wqes')
    ring0_mask = (2 ** log_num_cq_wqes) - 1

    # verify that pindex is NOT incremented  (i.e., No completion posted)
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_cq_pre_qstate, tc.pvtdata.rq_cq_post_qstate, 'p_index0', ring0_mask, 0):
        return False

    ###########   Key Invadlidation checks ### #####
    # read the key table entry for the rkey being invalidated 
    kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.inv_r_key)

    if not (kt_entry.data.state == 2): # KEY_STATE_VALID = 2
        logger.info("RDMA TestCaseVerify(): hw_lif %d qp %s rkey %d state %d" %
                (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.inv_r_key, kt_entry.data.state))
        logger.info("RDMA TestCaseVerify(): rkey state should not change for duplicate invalidate request")
        return False

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    if (GlobalOptions.dryrun): return
    rs = tc.config.rdmasession
    logger.info("Setting proxy_cindex/spec_cindex equal to p_index0\n")
    rs.lqp.rq.qstate.data.proxy_cindex = tc.pvtdata.rq_post_qstate.p_index0;
    rs.lqp.rq.qstate.data.spec_cindex = tc.pvtdata.rq_post_qstate.p_index0;
    rs.lqp.rq.qstate.WriteWithDelay();
    return

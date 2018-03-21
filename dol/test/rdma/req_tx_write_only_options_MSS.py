#! /usr/bin/python3

from test.rdma.utils import *
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
    #rs.lqp.sq.qstate.Read()
    #tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.va = 0x0102030405060708;
    tc.pvtdata.r_key = 0x0A0B0C0D;

    tc.pvtdata.roce_opt_ts_list = [0x00, 0x00, 0x33, 0x44, 0x55, 0x66, 0x00, 0x00]
    #Couldn't find 64 Bits for TS in SQCB. Hence living with 32 bits
    tc.pvtdata.roce_opt_ts_value = 0x00003344
    tc.pvtdata.roce_opt_ts_echo = 0x55660000
    tc.pvtdata.roce_opt_mss = 0x1719

    # Read SQ pre state
    rs.lqp.sq.qstate.Read()

    # Clear & Write back roce_opt values in SQCB1
    #rs.lqp.sq.qstate.data.roce_opt_ts_enable = 1
    rs.lqp.sq.qstate.data.roce_opt_mss_enable = 1
    rs.lqp.sq.qstate.data.timestamp = tc.pvtdata.roce_opt_ts_value
    rs.lqp.sq.qstate.data.timestamp_echo = tc.pvtdata.roce_opt_ts_echo >> 16
    rs.lqp.sq.qstate.data.mss = tc.pvtdata.roce_opt_mss

    rs.lqp.sq.qstate.Write()

    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)

    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    ring0_mask = (rs.lqp.num_sq_wqes - 1)
    tc.pvtdata.sq_post_qstate = rs.lqp.sq.qstate.data

    # verify that tx_psn is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', 1):
        return False

    # verify that p_index is incremented by 1
    if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'p_index0', ring0_mask,  1):
        return False

    # verify that c_index is incremented by 1
    if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'c_index0', ring0_mask, 1):
        return False

    # verify that ssn is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'ssn', 1):
        return False

    # verify that lsn is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'lsn', 1):
        return False

    # verify that busy is 0
    if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
        return False

    # verify that in_progress is 0
    if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
        return False

    return True

def TestCaseTeardown(tc):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseTeardown() Implementation.")
    rs = tc.config.rdmasession
    logger.info("Clearing Options related state in SQCB\n")
    rs.lqp.sq.qstate.data.roce_opt_ts_enable = 0
    rs.lqp.sq.qstate.data.roce_opt_mss_enable = 0
    rs.lqp.sq.qstate.data.timestamp = 0
    rs.lqp.sq.qstate.data.timestamp_echo = 0
    rs.lqp.sq.qstate.data.roce_opt_mss = 0
    rs.lqp.sq.qstate.Write()
    return

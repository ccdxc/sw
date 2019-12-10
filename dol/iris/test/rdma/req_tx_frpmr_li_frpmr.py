
#! /usr/bin/python3

from iris.test.rdma.utils import *
import pdb
import copy
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger
from iris.config.objects.rdma.keytable import *
import infra.common.objects as objects
import math

def Setup(infra, module): 
    return

def Teardown(infra, module):
    return      

def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession

    # Enable FRPMR on QP.
    rs.lqp.sq.qstate.Read()
    rs.lqp.sq.qstate.data.priv_oper_enable = 1;
    rs.lqp.sq.qstate.WriteWithDelay()

    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.dst_qp = tc.config.rdmasession.rqp.id

    tc.pvtdata.wrid = 0x1234
    SetIterPrivOperEnable(tc)

    # Get new key object for FRPMR.
    tc.pvtdata.key  = rs.lqp.pd.GetNewKey()
    tc.pvtdata.l_key = tc.pvtdata.key.lkey
    tc.pvtdata.mr_kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, (tc.pvtdata.l_key))

    tc.pvtdata.user_key = 192

    # Read CQ pre state
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_pre_qstate = rs.lqp.sq_cq.qstate.data

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

    tc.pvtdata.slab = tc.buffers.Get('BUF1').slab_id

    tc.pvtdata.len = tc.pvtdata.slab.size
    tc.pvtdata.log_pg_size = int(math.log(tc.pvtdata.slab.page_size,2.0))
    tc.pvtdata.base_va = tc.pvtdata.slab.address


    tc.pvtdata.slab_2 = tc.buffers.Get('BUF2').slab_id
            
    tc.pvtdata.len_2 = tc.pvtdata.slab_2.size
    tc.pvtdata.log_pg_size_2 = int(math.log(tc.pvtdata.slab_2.page_size,2.0))
    tc.pvtdata.base_va_2 = tc.pvtdata.slab_2.address

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

        # verify that frpmr_reset_spec_cindex is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'frpmr_in_progress', 0):
            return False


        # validate cqcb pindex and color
        if not ValidateReqRxCQChecks(tc, 'EXP_FRPMR_CQ_DESC_1'):
            return False

        ###########   FRPMR checks ##########
        # read the key table entry for lkey and verify if its valid
        frpmr_kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, (tc.pvtdata.l_key & 0xFFFFFF))

        if ((frpmr_kt_entry.data.state != 2) or
            (frpmr_kt_entry.data.type != 0) or
            (frpmr_kt_entry.data.log_page_size != tc.pvtdata.log_pg_size) or
            (frpmr_kt_entry.data.user_key != tc.pvtdata.user_key) or
            (frpmr_kt_entry.data.len != tc.pvtdata.len) or
            (frpmr_kt_entry.data.acc_ctrl != 0x1f) or # (ACC_CTRL_MW_BIND | ACC_CTRL_REMOTE_WRITE | ACC_CTRL_REMOTE_READ | ACC_CTRL_REMOTE_ATOMIC | ACC_CTRL_LOCAL_WRITE)
            (frpmr_kt_entry.data.flags != 0xa0) or # (MR_FLAG_INV_EN | MR_FLAG_UKEY_EN)
            (frpmr_kt_entry.data.base_va != tc.pvtdata.base_va)):
                logger.info("RDMA TestCaseVerify(): state: %d (exp: 2) type %d (exp: 0) log_page_size %d (pvt: %d) user_key: %d (exp: %d) "
                            "len: %d (exp: %d) acc_ctrl: 0x%x (exp: 0x1f) flags: 0x%x (exp: 0xa0) base_va: 0x%x (exp: 0x%x)" %
                            (frpmr_kt_entry.data.state, frpmr_kt_entry.data.type, frpmr_kt_entry.data.log_page_size, tc.pvtdata.log_pg_size,
                             frpmr_kt_entry.data.user_key, tc.pvtdata.user_key, frpmr_kt_entry.data.len, tc.pvtdata.len,
                             frpmr_kt_entry.data.acc_ctrl, frpmr_kt_entry.data.flags, frpmr_kt_entry.data.base_va, tc.pvtdata.base_va))
                logger.info("RDMA TestCaseVerify(): FRPMR fails for hw_lif %d qp %s lkey %d " %
                    (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.l_key))
                return False
        logger.info("RDMA TestCaseVerify(): Memory Slab %s is fast registered for hw_lif %d qp %s lkey %d" %
            (tc.pvtdata.slab.GID(), rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.l_key))

    elif step.step_id == 1:

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

        # verify that lsn is not incremented for send
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'lsn', 0):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False

        # verify that p_index of rrq is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index4', tc.pvtdata.sq_post_qstate, 'p_index4'):
            return False

    elif step.step_id == 2:
        msn = tc.pvtdata.sq_pre_qstate.ssn - 1

        # verify that msn is incremented to that of ssn of this msg
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'msn', msn):
            return False

        # verify that c_index of rrq is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'c_index4', tc.pvtdata.sq_post_qstate, 'c_index4'):
            return False

        # verify rexmit_psn is incremented to that of tx_psn
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_post_qstate, 'rexmit_psn', tc.pvtdata.sq_post_qstate, 'tx_psn'):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False

        # verify that tx_psn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'tx_psn', tc.pvtdata.sq_post_qstate, 'tx_psn'):
            return False

        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'token_id', 1):
            return False

        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'nxt_to_go_token_id', 1):
            return False

        if not ValidateReqRxCQChecks(tc, 'EXP_SEND_CQ_DESC_1'):
            return False
        ############     SKIP EQ VALIDATIONS #################
        #if not ValidateEQChecks(tc):
        #    return False

    if step.step_id == 3:

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
           
       # validate cqcb pindex and color
       if not ValidateReqRxCQChecks(tc, 'EXP_LI_CQ_DESC'):
           return False

       ###########   Key Invalidation checks ##########
       # read the key table entry for the lkey being invalidated 
       kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.l_key)

       if not (kt_entry.data.state == 1): # KEY_STATE_FREE = 1
           logger.info("RDMA TestCaseVerify(): Lkey invalidated fails for hw_lif %d qp %s lkey %d " %
                   (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.l_key))
           logger.info("RDMA TestCaseVerify(): Invalidated Lkey is not in Free state: state %d" %
                   kt_entry.data.state)
           return False
           
       logger.info("RDMA TestCaseVerify(): Lkey is invalidated for hw_lif %d qp %s lkey %d" %
               (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.l_key))


    if step.step_id == 4:
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
        
        # verify that frpmr_reset_spec_cindex is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'frpmr_in_progress', 0):
            return False
        
            
        # validate cqcb pindex and color
        if not ValidateReqRxCQChecks(tc, 'EXP_FRPMR_CQ_DESC_2'):
            return False

        ###########   FRPMR checks ##########
        # read the key table entry for lkey and verify if its valid
        frpmr_kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, (tc.pvtdata.l_key & 0xFFFFFF))
            
        if ((frpmr_kt_entry.data.state != 2) or
            (frpmr_kt_entry.data.type != 0) or
            (frpmr_kt_entry.data.log_page_size != tc.pvtdata.log_pg_size_2) or
            (frpmr_kt_entry.data.user_key != tc.pvtdata.user_key) or
            (frpmr_kt_entry.data.len != tc.pvtdata.len_2) or
            (frpmr_kt_entry.data.acc_ctrl != 0x1f) or # (ACC_CTRL_MW_BIND | ACC_CTRL_REMOTE_WRITE | ACC_CTRL_REMOTE_READ | ACC_CTRL_REMOTE_ATOMIC | ACC_CTRL_LOCAL_WRITE)
            (frpmr_kt_entry.data.flags != 0xa0) or # (MR_FLAG_INV_EN | MR_FLAG_UKEY_EN)
            (frpmr_kt_entry.data.base_va != tc.pvtdata.base_va_2)):
                logger.info("RDMA TestCaseVerify(): state: %d (exp: 2) type %d (exp: 0) log_page_size %d (pvt: %d) user_key: %d (exp: %d) "
                            "len: %d (exp: %d) acc_ctrl: 0x%x (exp: 0x1f) flags: 0x%x (exp: 0xa0) base_va: 0x%x (exp: 0x%x)" %
                            (frpmr_kt_entry.data.state, frpmr_kt_entry.data.type, frpmr_kt_entry.data.log_page_size, tc.pvtdata.log_pg_size_2,
                             frpmr_kt_entry.data.user_key, tc.pvtdata.user_key, frpmr_kt_entry.data.len, tc.pvtdata.len_2,
                             frpmr_kt_entry.data.acc_ctrl, frpmr_kt_entry.data.flags, frpmr_kt_entry.data.base_va, tc.pvtdata.base_va_2))
                logger.info("RDMA TestCaseVerify(): FRPMR fails for hw_lif %d qp %s lkey %d " %
                    (rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.l_key))
                return False
        logger.info("RDMA TestCaseVerify(): Memory Slab %s is fast registered for hw_lif %d qp %s lkey %d" %
            (tc.pvtdata.slab.GID(), rs.lqp.pd.ep.intf.lif.hw_lif_id, rs.lqp.GID(), tc.pvtdata.l_key))

            
    elif step.step_id == 5:
        
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

        # verify that lsn is not incremented for send
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'lsn', 0):
            return False
            
        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False
            
        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False
            
        # verify that p_index of rrq is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index4', tc.pvtdata.sq_post_qstate, 'p_index4'):
            return False

    elif step.step_id == 6:
        msn = tc.pvtdata.sq_pre_qstate.ssn - 1
        
        # verify that msn is incremented to that of ssn of this msg
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'msn', msn):
            return False

        # verify that c_index of rrq is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'c_index4', tc.pvtdata.sq_post_qstate, 'c_index4'):
            return False
            
        # verify rexmit_psn is incremented to that of tx_psn
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_post_qstate, 'rexmit_psn', tc.pvtdata.sq_post_qstate, 'tx_psn'):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False

        # verify that tx_psn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'tx_psn', tc.pvtdata.sq_post_qstate, 'tx_psn'):
            return False

        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'token_id', 1):
            return False

        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'nxt_to_go_token_id', 1):
            return False
            
        if not ValidateReqRxCQChecks(tc, 'EXP_SEND_CQ_DESC_2'):
            return False
        ############     SKIP EQ VALIDATIONS #################
        #if not ValidateEQChecks(tc):
        #    return False

    elif step.step_id == 7:
        if not ValidatePostSyncCQChecks(tc):
            return False

    # update current as pre_qstate ... so next step_id can use it as pre_qstate
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.sq_cq_pre_qstate = copy.deepcopy(rs.lqp.sq_cq.qstate.data)

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    if (GlobalOptions.dryrun): return
    rs = tc.config.rdmasession
    kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.l_key)
    kt_entry.data = tc.pvtdata.mr_kt_entry.data
    kt_entry.WriteWithDelay()

    rs.lqp.sq.qstate.Read()
    rs.lqp.sq.qstate.data.priv_oper_enable = 0;
    rs.lqp.sq.qstate.WriteWithDelay()

    return

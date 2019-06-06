import math
import copy
from infra.common.logging import logger as logger
from infra.common.logging import logger as logger
import binascii
from iris.config.objects.nvme.gbl import NvmeGlobalObject 
from iris.config.store import Store

def VerifyEqual(tc, name, val, exp_val):
    logger.info("%s actual: %s expected: %s" \
             %(name, repr(val), repr(exp_val)))
    cmp = val == exp_val
    logger.info('    Match: %s ' %cmp)
    return cmp

def VerifyFieldModify(tc, pre_state, post_state, field_name, incr):
    pre_val = getattr(pre_state, field_name)
    post_val = getattr(post_state, field_name)
    logger.info("%s pre: %d  post(actual): %d expected: %d" \
                    %(field_name, pre_val, post_val, pre_val+incr))
    cmp = (pre_val+incr == post_val)
    logger.info('    Match: %s ' %cmp)
    return cmp

def VerifyFieldMaskModify(tc, pre_state, post_state, field_name, mask, incr):
    pre_val = getattr(pre_state, field_name)
    post_val = getattr(post_state, field_name)
    logger.info("%s pre: %d  post(actual): %d expected: %d" \
                    %(field_name, pre_val, post_val, ((pre_val+incr) & mask)))
    cmp = (((pre_val+incr) & mask) == post_val)
    logger.info('    Match: %s ' %cmp)
    return cmp

def VerifyFieldMincrModify(tc, pre_state, post_state, field_name, log_entries_field_name, incr):
    pre_val = getattr(pre_state, field_name)
    post_val = getattr(post_state, field_name)
    log_entries = getattr(pre_state, log_entries_field_name)
    mask = (1 << log_entries)-1
    logger.info("%s pre: %d  post(actual): %d expected: %d" \
                    %(field_name, pre_val, post_val, ((pre_val+incr) & mask)))
    cmp = (((pre_val+incr) & mask) == post_val)
    logger.info('    Match: %s ' %cmp)
    return cmp

def VerifyFieldAbsolute(tc, state, field_name, exp_val):
    val = getattr(state, field_name)
    logger.info("%s actual: %d expected: %d" \
             %(field_name, val, exp_val))
    cmp = (val == exp_val)
    logger.info('    Match: %s ' %cmp)
    return cmp

def VerifyFieldsEqual(tc, state1, field_name1, state2, field_name2):
    val1 = getattr(state1, field_name1)
    val2 = getattr(state2, field_name2)
    logger.info("%s val: %d %s val: %d" \
             %(field_name1, val1, field_name2, val2))
    cmp = (val1 == val2)
    logger.info('    Match: %s ' %cmp)
    return cmp

def VerifyErrQState(tc):
    #TBD
    return True

def VerifyErrStatistics(tc):
    #TBD
    return True

def PopulatePreQStates(tc):
    tc.pvtdata.gbl = Store.objects.GetAllByClass(NvmeGlobalObject)[0]
    tc.pvtdata.resourcecb_pre_state = tc.pvtdata.gbl.ResourcecbRead()
    tc.pvtdata.hwxtstxcb_pre_state = tc.pvtdata.gbl.HwxtstxcbRead()
    tc.pvtdata.hwdgsttxcb_pre_state = tc.pvtdata.gbl.HwdgsttxcbRead()

    nscb = tc.config.nvmens.NscbRead()
    tc.pvtdata.nscb_pre_state = nscb

    start_session_id = getattr(nscb, "rr_session_id_to_be_served")

    nscb_bytes = bytes(nscb)
    # get the last 32 bytes
    bmap = nscb_bytes[32:]
    # and reverse the bytes
    bmap = bmap[::-1]
    bmap_size = len(bmap)
    loop_count = bmap_size
    byte_num = start_session_id//8
    bit_num = start_session_id%8

    # if we are starting with a partial byte, give us one more chance to 
    # revisit this byte after all bytes are inspected.
    if bit_num: 
        loop_count +=1
    found = False

    logger.debug('bmap_len: %d start_session_id: %d byte_num: %d bit_num: %d' \
                 %(len(bmap), start_session_id, byte_num, bit_num))
    
    for _ in range(loop_count):
        byte = bmap[byte_num]
        for bit_num in range(bit_num, 8):
            bmask = 1 << bit_num
            logger.debug('exploring byte: 0x%x byte_num: %d bit_num: %d mask: 0x%x' \
                         %(byte, byte_num, bit_num, bmask))
            if byte & bmask:
                found = True
                break
        if found == True:
            break
        bit_num = 0
        byte_num = (byte_num + 1)%bmap_size

    assert found == True, "could not find rr_session_id"

    tc.pvtdata.rr_session_id = (byte_num*8)+bit_num
    tc.pvtdata.rr_session_id_nxt = (tc.pvtdata.rr_session_id+1)%(bmap_size*8)

    tc.config.nvmesession = tc.config.nvmens.SessionGet(tc.pvtdata.rr_session_id)
    logger.info('found rr_session_id: %d byte_num: %d bit_num: %d nvme_session: %s ' \
                'rr_session_id_nxt: %d' \
                %(tc.pvtdata.rr_session_id, byte_num, bit_num, tc.config.nvmesession.GID(), \
                  tc.pvtdata.rr_session_id_nxt))

    
    tc.pvtdata.sqcb_pre_state = tc.config.nvmesession.sq.qstate.Read()
    tc.pvtdata.cqcb_pre_state = tc.config.nvmesession.cq.qstate.Read()
    tc.pvtdata.sessprodtxcb_pre_state = tc.config.nvmesession.NvmesessprodtxcbRead()
    tc.pvtdata.sessxtstxcb_pre_state = tc.config.nvmesession.tx_xtsq.qstate.Read()
    tc.pvtdata.sessdgsttxcb_pre_state = tc.config.nvmesession.tx_dgstq.qstate.Read()
    tc.pvtdata.sessprodrxcb_pre_state = tc.config.nvmesession.NvmesessprodrxcbRead()
    tc.pvtdata.sessxtsrxcb_pre_state = tc.config.nvmesession.rx_xtsq.qstate.Read()
    tc.pvtdata.sessdgstrxcb_pre_state = tc.config.nvmesession.rx_dgstq.qstate.Read()

    return

def PopulatePostQStates(tc):
    tc.pvtdata.resourcecb_post_state = tc.pvtdata.gbl.ResourcecbRead()
    tc.pvtdata.hwxtstxcb_post_state = tc.pvtdata.gbl.HwxtstxcbRead()
    tc.pvtdata.hwdgsttxcb_post_state = tc.pvtdata.gbl.HwdgsttxcbRead()
    tc.pvtdata.nscb_post_state = tc.config.nvmens.NscbRead()
    tc.pvtdata.sqcb_post_state = tc.config.nvmesession.sq.qstate.Read()
    tc.pvtdata.cqcb_post_state = tc.config.nvmesession.cq.qstate.Read()
    tc.pvtdata.sessprodtxcb_post_state = tc.config.nvmesession.NvmesessprodtxcbRead()
    tc.pvtdata.sessxtstxcb_post_state = tc.config.nvmesession.tx_xtsq.qstate.Read()
    tc.pvtdata.sessdgsttxcb_post_state = tc.config.nvmesession.tx_dgstq.qstate.Read()
    tc.pvtdata.sessprodrxcb_post_state = tc.config.nvmesession.NvmesessprodrxcbRead()
    tc.pvtdata.sessxtsrxcb_post_state = tc.config.nvmesession.rx_xtsq.qstate.Read()
    tc.pvtdata.sessdgstrxcb_post_state = tc.config.nvmesession.rx_dgstq.qstate.Read()
    return

def PostToPreCopyQStates(tc):
    tc.pvtdata.resourcecb_pre_state = tc.pvtdata.resourcecb_post_state
    tc.pvtdata.hwxtstxcb_pre_state = tc.pvtdata.hwxtstxcb_post_state
    tc.pvtdata.hwdgsttxcb_pre_state = tc.pvtdata.hwdgsttxcb_post_state
    tc.pvtdata.nscb_pre_state = tc.pvtdata.nscb_post_state
    tc.pvtdata.sqcb_pre_state = tc.pvtdata.sqcb_post_state
    tc.pvtdata.cqcb_pre_state = tc.pvtdata.cqcb_post_state
    tc.pvtdata.sessprodtxcb_pre_state = tc.pvtdata.sessprodtxcb_post_state
    tc.pvtdata.sessxtstxcb_pre_state = tc.config.nvmesession.sessxtstxcb_post_state
    tc.pvtdata.sessdgsttxcb_pre_state = tc.config.nvmesession.sessdgsttxcb_post_state
    tc.pvtdata.sessprodrxcb_pre_state = tc.pvtdata.sessprodrxcb_post_state
    tc.pvtdata.sessxtsrxcb_pre_state = tc.config.nvmesession.sessxtsrxcb_post_state
    tc.pvtdata.sessdgstrxcb_pre_state = tc.config.nvmesession.sessdgstrxcb_post_state
    return

def ValidateWriteTxChecks(tc):
    #this function verifies the sanity of various data structures in data path after write
    #command is transmitted.

    #sq pindex/cindex should have been incremented by 1
    if not VerifyFieldMincrModify(tc, tc.pvtdata.sqcb_pre_state, tc.pvtdata.sqcb_post_state,
                                 "p_index0", "log_num_wqes", 1):
        return False
    if not VerifyFieldMincrModify(tc, tc.pvtdata.sqcb_pre_state, tc.pvtdata.sqcb_post_state,
                                 "c_index0", "log_num_wqes", 1):
        return False

    #sq busy should have been cleared
    if not VerifyFieldAbsolute(tc, tc.pvtdata.sqcb_post_state, 'busy', 0):
        return False
    
    #check if rr_session_id_to_be_served is updated to new value
    if not VerifyFieldAbsolute(tc, tc.pvtdata.nscb_post_state,
                               "rr_session_id_to_be_served",
                               tc.pvtdata.rr_session_id_nxt):
        return False

    #check if a CMD context and Tx PDU context is checkedout
    if not VerifyFieldMincrModify(tc, tc.pvtdata.resourcecb_pre_state,
                                  tc.pvtdata.resourcecb_post_state,
                                  "cmdid_ring_proxy_ci", "cmdid_ring_log_sz", 1):
        return False
    if not VerifyFieldMincrModify(tc, tc.pvtdata.resourcecb_pre_state,
                                  tc.pvtdata.resourcecb_post_state,
                                  "cmdid_ring_ci", "cmdid_ring_log_sz", 1):
        return False
    if not VerifyFieldMincrModify(tc, tc.pvtdata.resourcecb_pre_state,
                                  tc.pvtdata.resourcecb_post_state,
                                  "tx_pduid_ring_proxy_ci", "tx_pduid_ring_log_sz", 1):
        return False
    if not VerifyFieldMincrModify(tc, tc.pvtdata.resourcecb_pre_state,
                                  tc.pvtdata.resourcecb_post_state,
                                  "tx_pduid_ring_ci", "tx_pduid_ring_log_sz", 1):
        return False

    #sessprodcbtx xtsq pi/ci should have been incremented by 1
    if not VerifyFieldMincrModify(tc, tc.pvtdata.sessprodtxcb_pre_state,
                                  tc.pvtdata.sessprodtxcb_post_state,
                                  "xts_q_pi", "log_num_xts_q_entries", 1):
        return False
    if not VerifyFieldMincrModify(tc, tc.pvtdata.sessprodtxcb_pre_state,
                                  tc.pvtdata.sessprodtxcb_post_state,
                                  "xts_q_ci", "log_num_xts_q_entries", 1):
        return False

    #sessprodcbtx dgstq pi/ci should have been incremented by 1
    if not VerifyFieldMincrModify(tc, tc.pvtdata.sessprodtxcb_pre_state,
                                  tc.pvtdata.sessprodtxcb_post_state,
                                  "dgst_q_pi", "log_num_dgst_q_entries", 1):
        return False
    if not VerifyFieldMincrModify(tc, tc.pvtdata.sessprodtxcb_pre_state,
                                  tc.pvtdata.sessprodtxcb_post_state,
                                  "dgst_q_ci", "log_num_dgst_q_entries", 1):
        return False

    #sessxtstxcb r0/r1 pi/ci should have been incremented by 1
    if not VerifyFieldMincrModify(tc, tc.pvtdata.sessxtstxcb_pre_state, 
                                  tc.pvtdata.sessxtstxcb_post_state,
                                  "p_index0", "log_num_entries", 1):
        return False
    if not VerifyFieldMincrModify(tc, tc.pvtdata.sessxtstxcb_pre_state, 
                                  tc.pvtdata.sessxtstxcb_post_state,
                                  "c_index0", "log_num_entries", 1):
        return False
    if not VerifyFieldMincrModify(tc, tc.pvtdata.sessxtstxcb_pre_state, 
                                  tc.pvtdata.sessxtstxcb_post_state,
                                  "p_index1", "log_num_entries", 1):
        return False
    if not VerifyFieldMincrModify(tc, tc.pvtdata.sessxtstxcb_pre_state, 
                                  tc.pvtdata.sessxtstxcb_post_state,
                                  "c_index1", "log_num_entries", 1):
        return False

    #sessxtstxcb r0/r1 busy/wb_busy should have become equal
    if not VerifyFieldsEqual(tc, tc.pvtdata.sessxtstxcb_post_state, "r0_busy",
                             tc.pvtdata.sessxtstxcb_post_state, "wb_r0_busy"):
        return False
    if not VerifyFieldsEqual(tc, tc.pvtdata.sessxtstxcb_post_state, "r1_busy",
                             tc.pvtdata.sessxtstxcb_post_state, "wb_r1_busy"):
        return False

    #sessdgsttxcb r0/r1 pi/ci should have been incremented by 1
    if not VerifyFieldMincrModify(tc, tc.pvtdata.sessdgsttxcb_pre_state, 
                                  tc.pvtdata.sessdgsttxcb_post_state,
                                  "p_index0", "log_num_entries", 1):
        return False
    if not VerifyFieldMincrModify(tc, tc.pvtdata.sessdgsttxcb_pre_state, 
                                  tc.pvtdata.sessdgsttxcb_post_state,
                                  "c_index0", "log_num_entries", 1):
        return False
    if not VerifyFieldMincrModify(tc, tc.pvtdata.sessdgsttxcb_pre_state, 
                                  tc.pvtdata.sessdgsttxcb_post_state,
                                  "p_index1", "log_num_entries", 1):
        return False
    if not VerifyFieldMincrModify(tc, tc.pvtdata.sessdgsttxcb_pre_state, 
                                  tc.pvtdata.sessdgsttxcb_post_state,
                                  "c_index1", "log_num_entries", 1):
        return False

    #sessdgsttxcb r0/r1 busy should have become equal
    if not VerifyFieldsEqual(tc, tc.pvtdata.sessdgsttxcb_post_state, "r0_busy",
                             tc.pvtdata.sessdgsttxcb_post_state, "wb_r0_busy"):
        return False
    if not VerifyFieldsEqual(tc, tc.pvtdata.sessdgsttxcb_post_state, "r1_busy",
                             tc.pvtdata.sessdgsttxcb_post_state, "wb_r1_busy"):
        return False

    #make sure xts descriptors are produced and consumed based on 
    #number of lbas in the request
    if not VerifyFieldMincrModify(tc, tc.pvtdata.hwxtstxcb_pre_state,
                                  tc.pvtdata.hwxtstxcb_post_state,
                                  "pi", "log_sz", tc.pvtdata.nlb):
        return False
    #XXX: for some reason opaqueue tag write is not updating the CI value in hwxtstxcb.
    #TO BE FIXED and then re-enable below check
    #if not VerifyFieldsEqual(tc, tc.pvtdata.hwxtstxcb_post_state, "pi",
    #                         tc.pvtdata.hwxtstxcb_post_state, "ci"):
    #    return False

    #make sure two hw dgst descriptors are produced and consumed
    if not VerifyFieldMincrModify(tc, tc.pvtdata.hwdgsttxcb_pre_state,
                                  tc.pvtdata.hwdgsttxcb_post_state,
                                  "pi", "log_sz", 2):
        return False
    if not VerifyFieldsEqual(tc, tc.pvtdata.hwdgsttxcb_post_state, "pi",
                             tc.pvtdata.hwdgsttxcb_post_state, "ci"):
        return False


    return True

def ResetErrQState(tc):
    return

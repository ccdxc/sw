#! /usr/bin/python3

import pdb
import math
from infra.api.objects import PacketHeader
import infra.api.api as infra_api
from infra.common.logging       import logger
import infra.common.defs as defs
from infra.common.glopts import GlobalOptions
from iris.test.callbacks.common.pktslicer import *
import binascii
from random import *
import iris.config.resmgr            as resmgr
from iris.test.callbacks.nvme.nvme import __get_pkt_payload_size_n_offset

def GetSeqNum (tc, pkt, args):
    _, offset = __get_pkt_payload_size_n_offset(tc, args.pkt_id)
    return tc.pvtdata.tcb_pre_state.snd_nxt + offset

def GetScaleSeqNum (tc, pkt):
    logger.info('GetScaleSeqNum called with pkt_id: %d' %tc.pvtdata.curr_pkt_id)
    _, offset = __get_pkt_payload_size_n_offset(tc, tc.pvtdata.curr_pkt_id)

    #NOTE: Currently seqnum callback is the last callback that gets called for every scaled-up packet.
    #If this changes in future, the below line should be moved accordingly.
    tc.pvtdata.curr_pkt_id = tc.pvtdata.curr_pkt_id + 1

    return tc.pvtdata.tcb_pre_state.snd_nxt + offset

def GetAckNum (tc, pkt):
    return tc.pvtdata.tcb_pre_state.rcv_nxt

def GetWindow (tc, pkt):
    return tc.pvtdata.tcb_pre_state.rcv_wnd

def GetRxSeqNum (tc, pkt):
    return tc.pvtdata.tcb_pre_state.rcv_nxt

def GetRxAckNum (tc, pkt):
    return tc.pvtdata.tcb_pre_state.snd_nxt + tc.pvtdata.capsulecmd_len

def GetRxWindow (tc, pkt):
    return tc.pvtdata.tcb_pre_state.snd_wnd

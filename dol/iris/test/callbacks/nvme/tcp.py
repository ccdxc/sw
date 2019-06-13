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

def GetSeqNum (tc, pkt):
    return tc.pvtdata.tcb_pre_state.snd_nxt

def GetAckNum (tc, pkt):
    return tc.pvtdata.tcb_pre_state.rcv_nxt

def GetWindow (tc, pkt):
    return tc.pvtdata.tcb_pre_state.rcv_wnd

#! /usr/bin/python3

import pdb
from infra.api.objects import PacketHeader
import infra.api.api as infra_api
from infra.common.logging       import logger
import infra.common.defs as defs

def GetRqPreEpsn (tc, pkt):
    return tc.pvtdata.pre_qstate.e_psn

def GetPktTxPsn (tc, pkt, args):
    return (tc.pvtdata.pre_qstate.tx_psn + args.pkt_num)

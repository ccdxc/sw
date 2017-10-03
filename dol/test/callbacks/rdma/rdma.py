#! /usr/bin/python3

import pdb
from infra.api.objects import PacketHeader
import infra.api.api as infra_api
from infra.common.logging       import logger
import infra.common.defs as defs

def GetRqPreEpsn (tc, pkt):
    return tc.pvtdata.rq_pre_qstate.e_psn


def GetPktTxPsn (tc, pkt, args):
    return (tc.pvtdata.sq_pre_qstate.tx_psn + args.pkt_num)

def GetPktExpPsn (tc, pkt, args):
    return (tc.pvtdata.rq_pre_qstate.e_psn + args.pkt_num)

def GetLastSuccessMsn (tc, pkt, args):
    return (tc.pvtdata.rq_pre_qstate.msn + args.num_msgs)   # increment by number of msgs received

def GetVAfromSLAB (tc, pkt, args):
    return (tc.config.rdmasession.lqp.pd.ep.slabs.Get(args.slab_id).address + args.offset)

def GetCQExpColor (tc, desc, args = None):
    if tc.pvtdata.rq_cq_pre_qstate.p_index0 == 0:
       return (not tc.pvtdata.rq_cq_pre_qstate.color)
    else:
       return (tc.pvtdata.rq_cq_pre_qstate.color)

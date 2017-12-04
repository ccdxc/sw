#! /usr/bin/python3

import pdb
import math
from infra.api.objects import PacketHeader
import infra.api.api as infra_api
from infra.common.logging       import logger
import infra.common.defs as defs
from infra.common.glopts import GlobalOptions
from test.callbacks.common.pktslicer import *
import binascii

def GetRqPreEpsn (tc, pkt):
    return tc.pvtdata.rq_pre_qstate.e_psn

def GetRqPreEpsnForTx (tc, pkt, args):
    return (tc.pvtdata.rq_pre_qstate.e_psn  + args.pkt_num)


def GetPktTxPsn (tc, pkt, args):
    return (tc.pvtdata.sq_pre_qstate.tx_psn + args.pkt_num)

def GetPktExpPsn (tc, pkt, args):
    return (tc.pvtdata.rq_pre_qstate.e_psn + args.pkt_num)

def GetLastSuccessMsn (tc, pkt, args):
    return (tc.pvtdata.rq_pre_qstate.msn + args.num_msgs)   # increment by number of msgs received

def GetVAfromSLAB (tc, pkt, args):
    if (GlobalOptions.dryrun): return 0
    return (tc.config.rdmasession.lqp.pd.ep.slabs.Get(args.slab_id).address + args.offset)

def GetCQExpColor (tc, desc, args = None):
    if args is None:
        entries = 1
    else:
        entries = args.entries

    pre_val = tc.pvtdata.rq_cq_pre_qstate.p_index0 
    log_num_cq_wqes = getattr(tc.pvtdata.rq_cq_pre_qstate, 'log_num_wqes')
    mask = (2 ** log_num_cq_wqes) - 1

    if pre_val is 0:
        color_change = True
    elif ((pre_val + entries) & mask) is 0:
        color_change = False
    elif pre_val > ((pre_val + entries) & mask):
        color_change = True
    else:
        color_change = False
       
    if color_change:
       return (not tc.pvtdata.rq_cq_pre_qstate.color)
    else:
       return (tc.pvtdata.rq_cq_pre_qstate.color)

def GetEQExpColor (tc, desc, args = None):
    if tc.pvtdata.eq_pre_qstate.p_index0 == 0:
       return (not tc.pvtdata.eq_pre_qstate.color)
    else:
       return (tc.pvtdata.eq_pre_qstate.color)

def GetReqRxCQExpColor (tc, desc, args = None):
    if args is None:
        entries = 1
    else:
        entries = args.entries

    pre_val = tc.pvtdata.sq_cq_pre_qstate.p_index0 
    log_num_cq_wqes = getattr(tc.pvtdata.sq_cq_pre_qstate, 'log_num_wqes')
    mask = (2 ** log_num_cq_wqes) - 1

    if pre_val is 0:
        color_change = True
    elif ((pre_val + entries) & mask) is 0:
        color_change = False
    elif pre_val > ((pre_val + entries) & mask):
        color_change = True
    else:
        color_change = False
       
    if color_change:
       return (not tc.pvtdata.sq_cq_pre_qstate.color)
    else:
       return (tc.pvtdata.sq_cq_pre_qstate.color)
     
def GetPacketPayloadSize(tc, pkt, args):
    pmtu = 1 << tc.pvtdata.rq_pre_qstate.log_pmtu
    if ((args.pkt_num + 1) <= (args.msg_size / pmtu)):
        pkt_payload_size = pmtu 
    else: 
        pkt_payload_size = args.msg_size % pmtu
    return (pkt_payload_size)

def MSB( n ):
  ndx = 0
  while ( 1 < n ):
    n = ( n >> 1 )
    ndx += 1
  return ndx

def GetAckSyndrome(tc, pkt, args):
    # AETH_CODE_ACK << AETH_SYNDROME_CODE_SHIFT | CCCCC (credit count)
    # 8 bits: 3 ack/Nak code + 5 bits
    # 3 bits for ACK: 000
    # 5 bits for lsn(ciredits) = log2(rqwqes) * 2 . If no RQWQEs posted, set credits to zero: 00000
    credits = 0
    if (args.rqwqes == 0):
        credits = 0
    else:
        #credits = int(math.log(args.rqwqes, 2) + 1) * 2
        credits = MSB(args.rqwqes) * 2
    syndrome = ((0 << 5) | credits)
    return syndrome

def GetNakSyndrome(tc, pkt, args):
    # AETH_CODE_NAK << AETH_SYNDROME_CODE_SHIFT | nak_code
    syndrome = ((3 << 5) | args.nak_code) 
    return syndrome

def GetUDPacketPayload(tc, packet, args):
    srcpacket = tc.packets.Get(args.pktid) 
    slicer = PacketSlicer(srcpacket, args)
    slc = slicer.GetSlice()
    ipv4pad = bytes([0x00]*20)
    payload_size = getattr(args, 'pl_size', 64)
    payload = bytes([0xFF]*payload_size)
    print('GetUDPacketPayload: %s ' % (binascii.hexlify(bytes(slc+ipv4pad+payload))))
    return list(slc+ipv4pad+payload)
    
def GetZeroMAC(tc, desc):
    return bytes([0x0]*6)

def GetUDSMAC(tc, desc):
    return bytes(tc.config.rdmasession.session.responder.ep.macaddr.getnum().to_bytes(6, 'big'))

def GetUDInitiatorSMAC(tc, desc):
    return bytes(tc.config.rdmasession.session.initiator.ep.macaddr.getnum().to_bytes(6, 'big'))

# Callbacks for RDMA Multicast
def __get_ring_from_lif(lif, rtype, qid, rid):
    queue = lif.GetQ(rtype, qid)
    ring = queue.rings.Get(rid)
    return ring

def __get_lif_qp(tc, args):
    if args is None:
        return None

    udqpslist = tc.pvtdata.udqps_pruned_list
    if len(udqpslist) < args.idx:
        return None
    return udqpslist[args.idx]

def __get_lif_qp_at_idx(tc, idx):
    udqpslist = tc.pvtdata.udqps_pruned_list
    if len(udqpslist) < idx:
        return None
    return udqpslist[idx]

def GetCQRingForMulticastCopy(tc, args = None):
    (lif, qp) = __get_lif_qp(tc, args)
    ring = __get_ring_from_lif(lif, 'RDMA_CQ', qp.rq_cq.id, 'CQ')
    tc.info("GetCQRingForLif: %s CQid: %d RxCqRing: %s" % (lif.GID(), qp.rq_cq.id, ring.GID()))
    return ring

def GetRdmaRxRingForMulticastCopy(tc, args = None):
    (lif, qp) = __get_lif_qp(tc, args)
    ring = __get_ring_from_lif(lif, 'RDMA_RQ', qp.id, 'RQ')
    tc.info("GetRdmaRxRingForLif: %s Qid: %s RdmaRxRing: %s" % (lif.GID(), qp.GID(), ring.GID()))
    return ring

def GetRdmaRxRingDoorBellForMulticastCopy(tc, args = None):
    (lif, qp) = __get_lif_qp(tc, args)
    ring = __get_ring_from_lif(lif, 'RDMA_RQ', qp.id, 'RQ')
    tc.info("GetRdmaRxRingDoorBellForLif: %s Qid: %s RdmaRxRing: %s" % (lif.GID(), qp.GID(), ring.GID()))
    return ring.doorbell

def GetQpIdForMulticastCopy(tc, desc, args = None):
    (lif, qp) = __get_lif_qp(tc, args)
    tc.info("GetCQRingForLif: %s Qid: %s" % (lif.GID(), qp.GID()))
    return qp.id

def GetEpSlab0ForMulticastCopy(tc, buf, args = None):
    (lif, qp) = __get_lif_qp(tc, args)
    slab = qp.pd.ep.slabs.Get('SLAB0000');
    tc.info("GetEpSlab0ForMulticastCopy: %s Qid: %s Slab: %s" % (lif.GID(), qp.GID(), slab.GID()))
    return slab

def GetMrSlab0LkeyForMulticastCopy(tc, buf, args = None):
    (lif, qp) = __get_lif_qp(tc, args)
    mr = qp.pd.mrs.Get('MR-SLAB0000');
    tc.info("GetEpSlab0LkeyForMulticastCopy: %s Qid: %s Mr: %s Lkey: %s" % (lif.GID(), qp.GID(), mr.GID(), mr.lkey))
    return mr.lkey

def GetEpSlab1ForMulticastCopy(tc, buf, args = None):
    (lif, qp) = __get_lif_qp(tc, args)
    slab = qp.pd.ep.slabs.Get('SLAB0001');
    tc.info("GetEpSlab0ForMulticastCopy: %s Qid: %s Slab: %s" % (lif.GID(), qp.GID(), slab.GID()))
    return slab

def GetMrSlab1LkeyForMulticastCopy(tc, buf, args = None):
    (lif, qp) = __get_lif_qp(tc, args)
    mr = qp.pd.mrs.Get('MR-SLAB0001');
    tc.info("GetEpSlab0LkeyForMulticastCopy: %s Qid: %s Mr: %s Lkey: %s" % (lif.GID(), qp.GID(), mr.GID(), mr.lkey))
    return mr.lkey

def GetAtomicData(tc, pkt, args = None):
    srcpkt = tc.packets.Get(args.pktid)
    rbytes = srcpkt.rawbytes
    val = 0
    for b in list(rbytes):
        val = val << 8
        val = val | b
    return val

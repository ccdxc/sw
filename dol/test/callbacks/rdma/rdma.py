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
from random import *

def GetRqPreEpsn (tc, pkt):
    if GlobalOptions.perf:
        curr_e_psn = tc.pvtdata.e_psn
        tc.pvtdata.e_psn = tc.pvtdata.e_psn + 1
        return curr_e_psn
    else:
        return tc.pvtdata.rq_pre_qstate.e_psn

def GetRqPreEpsnMultiQPs (tc, pkt):
    if GlobalOptions.perf:
        return tc.pvtdata.e_psn
    else:
        return tc.pvtdata.rq_pre_qstate.e_psn

def GetRqPreEpsn1QPiCache (tc, pkt):
    e_psn = tc.config.rdmasession.lqp.rq.qp_e_psn
    tc.config.rdmasession.lqp.rq.qp_e_psn += 1
    return e_psn

def GetRqPreEpsnForTx (tc, pkt, args):
    return (tc.pvtdata.rq_pre_qstate.e_psn  + args.pkt_num)


def GetPktTxPsn (tc, pkt, args):
    return (tc.pvtdata.sq_pre_qstate.tx_psn + args.pkt_num)

def GetRqPktPsn (tc, pkt):
    if GlobalOptions.perf:
        curr_tx_psn = tc.module.pvtdata.sq_pre_qstate.tx_psn
        tc.module.pvtdata.sq_pre_qstate.tx_psn = tc.module.pvtdata.sq_pre_qstate.tx_psn + 1
        return curr_tx_psn
    else:
        return tc.pvtdata.sq_pre_qstate.tx_psn

def GetPktExpPsn (tc, pkt, args):
    return (tc.pvtdata.rq_pre_qstate.e_psn + int(args.pkt_num))

def GetPktMsn (tc, pkt, args):
    return (tc.pvtdata.sq_pre_qstate.ssn + int(args.ack_num))

def GetCQMsn (tc, pkt, args):
    return (tc.pvtdata.sq_pre_qstate.ssn + int(args.sq_wqe_num))

def GetWQEColor (tc, pkt, args):
    return (tc.pvtdata.sq_pre_qstate.color)

def GetLastSuccessMsn (tc, pkt, args):
    return (tc.pvtdata.rq_pre_qstate.msn + args.num_msgs)   # increment by number of msgs received

def GetVAfromSLAB (tc, pkt, args):
    if (GlobalOptions.dryrun): return 0
    return (tc.config.rdmasession.lqp.pd.ep.slabs.Get(args.slab_id).address + args.offset)

def GetVAfromTcSLAB1 (tc, pkt, args):
    if (GlobalOptions.dryrun): return 0
    return (tc.pvtdata.slab_1.address + args.offset)

def GetVAfromTcSLAB2 (tc, pkt, args):
    if (GlobalOptions.dryrun): return 0
    return (tc.pvtdata.slab_2.address + args.offset)

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

def GetReqRxCQExpColor (tc, desc, args):
    log_num_cq_wqes = getattr(tc.pvtdata.sq_cq_pre_qstate, 'log_num_wqes')
    ring0_mask = (2 ** log_num_cq_wqes) - 1

    if (tc.pvtdata.sq_cq_pre_qstate.p_index0 == 0) or \
       ((tc.pvtdata.sq_cq_pre_qstate.p_index0 + args.cq_wqe_num) & ring0_mask) < \
                                              tc.pvtdata.sq_cq_pre_qstate.p_index0:
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

def GetCQColorForMulticastCopy(tc, desc, args = None):
    (lif, qp) = __get_lif_qp(tc, args)
    qp.rq_cq.qstate.Read()
    rq_cq_pre_qstate = qp.rq_cq.qstate.data

    log_num_cq_wqes = getattr(rq_cq_pre_qstate, 'log_num_wqes')
    ring0_mask = (2 ** log_num_cq_wqes) - 1

    if (rq_cq_pre_qstate.p_index0 == 0) or \
       ((rq_cq_pre_qstate.p_index0 + args.cq_wqe_num) & ring0_mask) < \
                                              rq_cq_pre_qstate.p_index0:
       color = not rq_cq_pre_qstate.color
    else:
       color = rq_cq_pre_qstate.color
     
    logger.info("GetCQColorForMulticastCopy: Lif %s CQid: %d Color: %d" % (lif.GID(), qp.rq_cq.id, color))
    return color

def GetCQRingForMulticastCopy(tc, args = None):
    (lif, qp) = __get_lif_qp(tc, args)
    ring = __get_ring_from_lif(lif, 'RDMA_CQ', qp.rq_cq.id, 'CQ')
    logger.info("GetCQRingForLif: %s CQid: %d RxCqRing: %s" % (lif.GID(), qp.rq_cq.id, ring.GID()))
    return ring

def GetRdmaRxRingForMulticastCopy(tc, args = None):
    (lif, qp) = __get_lif_qp(tc, args)
    ring = __get_ring_from_lif(lif, 'RDMA_RQ', qp.id, 'RQ')
    logger.info("GetRdmaRxRingForLif: %s Qid: %s RdmaRxRing: %s" % (lif.GID(), qp.GID(), ring.GID()))
    return ring

def GetRdmaRxRingDoorBellForMulticastCopy(tc, args = None):
    (lif, qp) = __get_lif_qp(tc, args)
    ring = __get_ring_from_lif(lif, 'RDMA_RQ', qp.id, 'RQ')
    logger.info("GetRdmaRxRingDoorBellForLif: %s Qid: %s RdmaRxRing: %s" % (lif.GID(), qp.GID(), ring.GID()))
    return ring.doorbell

def GetQpIdForMulticastCopy(tc, desc, args = None):
    (lif, qp) = __get_lif_qp(tc, args)
    logger.info("GetCQRingForLif: %s Qid: %s" % (lif.GID(), qp.GID()))
    return qp.id

def GetEpSlab0ForMulticastCopy(tc, buf, args = None):
    (lif, qp) = __get_lif_qp(tc, args)
    slab = qp.pd.ep.slabs.Get('SLAB0000');
    logger.info("GetEpSlab0ForMulticastCopy: %s Qid: %s Slab: %s" % (lif.GID(), qp.GID(), slab.GID()))
    return slab

def GetMrSlab0LkeyForMulticastCopy(tc, buf, args = None):
    (lif, qp) = __get_lif_qp(tc, args)
    mr = qp.pd.mrs.Get('MR-SLAB0000');
    logger.info("GetEpSlab0LkeyForMulticastCopy: %s Qid: %s Mr: %s Lkey: %s" % (lif.GID(), qp.GID(), mr.GID(), mr.lkey))
    return mr.lkey

def GetEpSlab1ForMulticastCopy(tc, buf, args = None):
    (lif, qp) = __get_lif_qp(tc, args)
    slab = qp.pd.ep.slabs.Get('SLAB0001');
    logger.info("GetEpSlab0ForMulticastCopy: %s Qid: %s Slab: %s" % (lif.GID(), qp.GID(), slab.GID()))
    return slab

def GetMrSlab1LkeyForMulticastCopy(tc, buf, args = None):
    (lif, qp) = __get_lif_qp(tc, args)
    mr = qp.pd.mrs.Get('MR-SLAB0001');
    logger.info("GetEpSlab0LkeyForMulticastCopy: %s Qid: %s Mr: %s Lkey: %s" % (lif.GID(), qp.GID(), mr.GID(), mr.lkey))
    return mr.lkey

def GetAtomicData(tc, pkt, args = None):
    srcpkt = tc.packets.Get(args.pktid)
    rbytes = srcpkt.rawbytes
    val = 0
    for b in list(rbytes):
        val = val << 8
        val = val | b
    return val

def PickPktTemplate(tc, pkt, args = None):
    if tc.config.rdmasession.session.IsIPV6():
       template = args.v6
    else:
       template = args.v4
    logger.info("ChoosePacketTemplateByFlow: Picking %s" % (template))
    return infra_api.GetPacketTemplate(template)
    
# return a psn which is pkt_num less than e_psn
def GetRqDupPsn(tc, pkt, args = None):
    return tc.pvtdata.rq_pre_qstate.e_psn - args.pkt_num

# return ack_nak_psn stored in rqcb
def GetRqAckNakPsn(tc, pkt, args = None):
    return tc.pvtdata.rq_pre_qstate.ack_nak_psn

# return aeth syndrome stored in rqcb
def GetRqAethSyndrome(tc, pkt, args = None):
    return tc.pvtdata.rq_pre_qstate.aeth_syndrome

# return aeth msn stored in rqcb
def GetRqAethMsn(tc, pkt, args = None):
    return tc.pvtdata.rq_pre_qstate.aeth_msn

def __get_template(tid):
    if tid == None: return None
    return infra_api.GetPacketTemplate(tid)

def GetPacketEncaps(testcase, packet):
    encaps = []
    is_vxlan = testcase.config.rdmasession.rqp.pd.ep.segment.IsFabEncapVxlan()

    # Add VLAN encap always
    encaps.append(__get_template('ENCAP_QTAG'))

    # Also add VXLAN encap for VXLAN end point
    if is_vxlan:
        encaps.append(__get_template('ENCAP_VXLAN'))

    if len(encaps):
        return encaps

    return None

def GetPacketQtag(testcase, packet):
    is_vxlan = testcase.config.rdmasession.rqp.pd.ep.segment.IsFabEncapVxlan()

    if is_vxlan:
        return testcase.config.rdmasession.rqp.pd.ep.intf.vlan_id
    else:
        return testcase.config.rdmasession.rqp.pd.ep.segment.vlan_id

def GetRandomPacketLen(testcase, packet):
    randlen = randint(64, 1000)
    logger.info("GetRandomPacketLen: Packet random len %d" % (randlen))
    return randlen

def GetNewSlab(testcase, buffers):
    return testcase.config.rdmasession.lqp.pd.ep.GetNewSlab()

def GetMrSlabLkey(testcase, descriptor, args):
    if args == None: return None    
    slab_id = testcase.buffers.Get(args.id).slab_id
    if slab_id == None: return None
    return testcase.config.rdmasession.lqp.pd.mrs.Get('MR-' + slab_id.GID()).lkey

def GetSlab(testcase, buffers, args):
    if args == None: return None
    return testcase.buffers.Get(args.id).slab_id

def GetPktPayloadSizeWithPad(testcase, packet, args):
    if args == None:
        return None
    pktid = getattr(args, 'pktid')
    pkt = testcase.packets.Get(args.pktid)
    padsize = 4 - (pkt.payloadsize % 4)
    return (pkt.payloadsize + padsize)

def GetPktPayloadDataWithPad(testcase, packet, args):
    if args == None:
        return None
    pktid = getattr(args, 'pktid')
    pkt = testcase.packets.Get(args.pktid)
    payload = pkt.headers.payload.fields.data
    padsize = 4 - (pkt.payloadsize % 4)
    for x in range(padsize):
        payload.append(0x0)
    return payload


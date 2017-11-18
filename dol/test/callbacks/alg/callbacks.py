#! /usr/bin/python3
from infra.api.objects import PacketHeader
import infra.api.api as infra_api
import infra.penscapy as penscapy
import pdb

class SUNRPCReplyData:
    def __init__(self, pgm=0, vers=0, netid_len=0, netid=0, addr_len=0,
                 addr=0, owner_len=0, owner=0, ValFollows=0):
        self.pgm = pgm
        self.vers = vers
        self.netid_len = netid_len 
        self.netid = netid
        self.addr_len = addr_len
        self.addr = addr
        self.owner_len = owner_len 
        self.owner = owner
        self.ValFollows = ValFollows
        
def GetSUNRPCPgmData(pgm, netid, addr, owner, valfollows=0):
    return SUNRPCReplyData(pgm=pgm, vers=4, netid_len=3,
            netid=netid, addr_len=13, addr=addr, owner=owner, ValFollows=valfollows)

def GetExpectedSUNRPCDumpReplyData(testcase, packet):
    data = []
    data.append(GetSUNRPCPgmData(100000, 'tcp', '0.0.0.0.0.111', '29', 1))
    data.append(GetSUNRPCPgmData(100024, 'tcp', '0.0.0.0.128.8', '29'))

    return data

def GetCpuPacketbyIflow(testcase, args = None):
    root = getattr(testcase.config, 'flow', None)
    if root is None:
        root = getattr(testcase.config.session.iconfig, 'flow', None)

    if root.IsFteEnabled():
        return testcase.packets.Get(args.expktid)

    return None

def GetCpuPacketbyRflow(testcase, args = None):
    root = getattr(testcase.config, 'flow', None)
    if root is None:
        root = getattr(testcase.config.session.rconfig, 'flow', None)

    if root.IsFteEnabled():
        return testcase.packets.Get(args.expktid)

    return None

def __get_expected_packet(testcase, args, config=None):
    root = getattr(testcase.config, 'flow', None)
    if root is None:
       if config is None:
           config = testcase.config.session.iconfig
       root = getattr(config, 'flow', None)

    if root.IsDrop():
        return None

    if args is None:
        return testcase.packets.Get('EXP_PKT')

    return testcase.packets.Get(args.expktid)

def GetL3UcExpectedPacketbyIflow(testcase, args = None):
    return __get_expected_packet(testcase, args, testcase.config.session.iconfig)

def GetL3UcExpectedPacketbyRflow(testcase, args = None):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile

    return __get_expected_packet(testcase, args, testcase.config.session.rconfig)

def __get_packet_template_impl(flow, pkttemplate):
    if pkttemplate is not None:
        template = pkttemplate
    else:
        template = 'ETH'
        if flow.IsIP():
            template += "_%s_%s" % (flow.type, flow.proto)
        else:
            assert(0)

    return infra_api.GetPacketTemplate(template)

def GetPacketTemplateBySessionIflow(testcase, packet, args=None):
    l7proto =  testcase.tracker.PktTemplate()
    return __get_packet_template_impl(testcase.tracker.config.flow, l7proto)

def GetPacketTemplateBySessionRflow(testcase, packet, args=None):
    return __get_packet_template_impl(testcase.config.session.rconfig.flow, args)

def GetForwardingCopy(tc, args = None):
    if tc.tracker.step.IsDrop():
        return None
    return tc.packets.Get(args.pktid)

def GetCpuCopy(tc, args = None):
    if tc.tracker.IsCpuCopyValid():
        return tc.packets.Get(args.pktid)
    return None

def GetExpectDelay(tc, args = None):
    if tc.tracker.IsCpuCopyValid():
        return 1
    return None

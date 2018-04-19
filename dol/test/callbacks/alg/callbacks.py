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

def GetSIP(testcase, args = None):
    sip = testcase.tracker.config.flow.sip
    return (int(sip.value))

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
    l7proto =  testcase.tracker.PktTemplate()
    if (l7proto == 'ETH_IPV4_UDP'):
        l7proto = 'ETH_IPV4_ICMP_IPERROR_UDPERROR'
    return __get_packet_template_impl(testcase.config.session.rconfig.flow, l7proto)

def GetForwardingCopy(tc, args = None):
    if tc.tracker.step.IsDrop():
        return None
    return tc.packets.Get(args.pktid)

def GetRejectCopy(tc, args):
    return  __get_expected_packet(tc, args, tc.config.session.rconfig)

def GetPadding(tc, args):
    data = [0x0] * 20 
    return data

def GetCpuCopy(tc, args = None):
    if tc.tracker.IsCpuCopyValid():
        return tc.packets.Get(args.pktid)
    return None

def GetExpectDelay(tc, args = None):
    if tc.tracker.IsCpuCopyValid():
        return 1
    return None

def GetFTPData(tc, args=None):
    step = tc.tracker.step._FrameworkObject__gid
    if step.find('FTP_PORT_RSP') != -1:
        data = [0x32, 0x30, 0x30, 0x20, 0x50, 0x4f, 0x52, 0x54,
                0x20, 0x63, 0x6f, 0x6d, 0x6d, 0x61, 0x6e, 0x64,
                0x20, 0x73, 0x75, 0x63, 0x63, 0x65, 0x73, 0x73,
                0x66, 0x75, 0x6c, 0x2e, 0x20, 0x43, 0x6f, 0x6e,
                0x73, 0x69, 0x64, 0x65, 0x72, 0x20, 0x75, 0x73, 
                0x69, 0x6e, 0x67, 0x20, 0x50, 0x41, 0x53, 0x56,
                0x2e, 0x0d, 0x0a]
    elif step.find('FTP_PORT') != -1:
        data = [0x50, 0x4f, 0x52, 0x54, 0x20, 0x36, 0x34, 0x2c, 
                0x30, 0x2c, 0x30, 0x2c, 0x32, 0x2c, 0x31, 0x34,
                0x34, 0x2c, 0x32, 0x31, 0x31, 0x0d, 0x0a]
    elif step.find('FTP_EPRT_RSP') != -1:
        data = [0x32, 0x30, 0x30, 0x20, 0x45, 0x50, 0x52, 0x54,
                0x20, 0x63, 0x6f, 0x6d, 0x6d, 0x61, 0x6e, 0x64,
                0x20, 0x73, 0x75, 0x63, 0x63, 0x65, 0x73, 0x73,
                0x66, 0x75, 0x6c, 0x2e, 0x0d, 0x0a]
    elif step.find('FTP_EPRT') != -1:
        data = [0x45, 0x50, 0x52, 0x54, 0x20, 0x7c, 0x32, 0x7c,
                0x32, 0x30, 0x30, 0x30, 0x3a, 0x3a, 0x34, 0x3a, 
                0x30, 0x3a, 0x32, 0x7c, 0x33, 0x37, 0x30, 0x37, 
                0x35, 0x7c, 0x0d, 0x0a]
    elif step.find('FTP_PASV_RSP') != -1:
        data = [0x32, 0x32, 0x37, 0x20, 0x45, 0x6e, 0x74, 0x65,
                0x72, 0x69, 0x6e, 0x67, 0x20, 0x50, 0x61, 0x73,
                0x73, 0x69, 0x76, 0x65, 0x20, 0x4d, 0x6f, 0x64,
                0x65, 0x20, 0x28, 0x36, 0x34, 0x2c, 0x30, 0x2c,
                0x30, 0x2c, 0x33, 0x2c, 0x31, 0x34, 0x34, 0x2c, 
                0x32, 0x31, 0x31, 0x29, 0x2e, 0x0d, 0x0a]
    elif step.find('FTP_PASV') != -1:
        data = [0x50, 0x41, 0x53, 0x56, 0x0d, 0x0a]
    elif step.find('FTP_EPSV_RSP') != -1:
        data = [0x32, 0x32, 0x39, 0x20, 0x45, 0x6e, 0x74, 0x65,
                0x72, 0x69, 0x6e, 0x67, 0x20, 0x45, 0x78, 0x74,
                0x65, 0x6e, 0x64, 0x65, 0x64, 0x50, 0x61, 0x73,
                0x73, 0x69, 0x76, 0x65, 0x20, 0x4d, 0x6f, 0x64,
                0x65, 0x20, 0x28, 0x7c, 0x7c, 0x7c, 0x33, 0x37, 
                0x30, 0x37, 0x35, 0x7c, 0x29, 0x2e, 0x0d, 0x0a]
    elif step.find('FTP_EPSV') != -1:
        data = [0x45, 0x50, 0x53, 0x56, 0x0d, 0x0a]
    else:
        data = []

    return data 

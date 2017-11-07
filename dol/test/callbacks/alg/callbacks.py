#! /usr/bin/python3
from infra.api.objects import PacketHeader
import infra.api.api as infra_api
import infra.penscapy as penscapy

def GetExpectedTFTPIflowOp(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'RRQ' in profile_name:
        return 'RRQ'
    elif 'WRQ' in profile_name:
        return 'WRQ'
    return None

def GetExpectedTFTPRflowOp(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'DATA' in profile_name:
        return 'DATA'
    elif 'OACK' in profile_name:
        return 'OACK'
    elif 'ACK' in profile_name:
        return 'ACK'
    elif 'ERROR' in profile_name:
        return 'ERROR'

    return None

def GetExpectedSUNRPCPortmapvers(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'VERS2' in profile_name:
        return 2
    elif 'VERS4' in profile_name:
        return 4

    return None

def GetExpectedSUNRPCProc(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'GETPORT' in profile_name:
       return 3
    elif 'DUMP' in profile_name:
       return 4
    elif 'CALLIT' in profile_name:
       return 5

    return None

def GetExpectedSUNRPCReplyState(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'SUCCESS' in profile_name:
       return 0
    else: 
       return 1

def GetExpectedSUNRPCAccState(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'ACCEPT' in profile_name:
       return 0
    else:
       return 1

def GetExpectedSUNRPCXid(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'GETPORT' in profile_name:
        return 1122334455
    elif 'DUMP' in profile_name:
        return 1122334466
    else:
        return 1122334477

def GetUDPDataPort(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'GETPORT' in profile_name:
        return 0xd66b
    elif 'DUMP' in profile_name:
        return 0x8008

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

def GetSUNRPCPgmData(pgm, netid, addr, owner):
    return SUNRPCReplyData(pgm, 4, 
            netid=netid, addr=addr, owner=owner)

def GetExpectedSUNRPCDumpReplyData(testcase, packet):
    data = []
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'PORTMAP' in profile_name:
        data.append(GetSUNRPCPgmData(100000, 'tcp', '0.0.0.0.0.111', '29'))
    if 'STAT' in profile_name:
        if (len(data)):
            data[len(data)-1].ValFollows = 1
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

def __get_packet_template_impl(flow, args):
    template = 'ETH'
    if flow.IsIP():
        template += "_%s_%s" % (flow.type, flow.proto)
    else:
        assert(0)

    if args is not None:
        template += "_%s" % (args.l7proto)
 
    return infra_api.GetPacketTemplate(template)

def GetPacketTemplateBySessionIflow(testcase, packet, args=None):
    return __get_packet_template_impl(testcase.config.session.iconfig.flow, args)

def GetPacketTemplateBySessionRflow(testcase, packet, args=None):
    return __get_packet_template_impl(testcase.config.session.rconfig.flow, args)

def __get_packet_template_byprofile_impl(flow, profile_name, args):
    template = 'ETH'
    if flow.IsIP():
        template += "_%s_%s" % (flow.type, flow.proto)
    else:
        assert(0)

    if args is not None:
       if 'SUNRPC_CALL' in args.l7proto:
          if 'VERS2' in profile_name:
              if 'GETPORT' in profile_name:
                  template += "_SUNRPC_CALL_2_GETPORT"
              elif 'DUMP' in profile_name:
                  template += "_SUNRPC_CALL_2_DUMP"

          elif 'VERS4' in profile_name:
              if 'GETPORT' in profile_name:
                  template += "_SUNRPC_CALL_4_GETPORT"
              elif 'DUMP' in profile_name:
                  template += "_SUNRPC_CALL_4_DUMP"

       elif 'SUNRPC_REPLY' in args.l7proto:
          if 'VERS2' in profile_name:
              if 'GETPORT' in profile_name:
                  template += "_SUNRPC_REPLY_2_GETPORT" 
              elif 'DUMP' in profile_name:
                  template += "_SUNRPC_REPLY_2_DUMP"
   
          elif 'VERS4' in profile_name:
              if 'GETPORT' in profile_name:
                  template += "_SUNRPC_REPLY_4_GETPORT"
              elif 'DUMP' in profile_name:
                  template += "_SUNRPC_REPLY_4_DUMP"
    print (template)

    return infra_api.GetPacketTemplate(template)

def GetSRPCTemplatebyIflow(testcase, packet, args=None):
    iterelem = testcase.module.iterator.Get()
    return __get_packet_template_byprofile_impl(testcase.config.session.iconfig.flow, iterelem.profile, args)

def GetSRPCTemplatebyRflow(testcase, packet, args=None):
    iterelem = testcase.module.iterator.Get()
    return __get_packet_template_byprofile_impl(testcase.config.session.rconfig.flow, iterelem.profile, args)

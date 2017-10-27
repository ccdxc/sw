#! /usr/bin/python3

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

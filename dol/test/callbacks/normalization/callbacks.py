#! /usr/bin/python3
import pdb

class TcpOptions:
    def __init__(self, kind, data):
        self.kind = kind
        self.data = data
        return

def GetInputIpv4Flags(testcase, packet):
    profile_name = testcase.module.iterator.Get()
    if 'IP_RSVD_FLAGS_ACTION_ALLOW' in profile_name:
        return 0x4
    elif 'IP_RSVD_FLAGS_ACTION_DROP' in profile_name:
        return 0x4
    elif 'IP_RSVD_FLAGS_ACTION_EDIT' in profile_name:
        return 0x4
    elif 'IP_DF_ACTION_ALLOW' in profile_name:
        return 0x2
    elif 'IP_DF_ACTION_DROP' in profile_name:
        return 0x2
    elif 'IP_DF_ACTION_EDIT' in profile_name:
        return 0x2
    return 0x0

def GetExpectedIpv4Flags(testcase, packet):
    profile_name = testcase.module.iterator.Get()
    if 'IP_RSVD_FLAGS_ACTION_ALLOW' in profile_name:
        return 0x4
    elif 'IP_RSVD_FLAGS_ACTION_EDIT' in profile_name:
        return 0x0
    elif 'IP_DF_ACTION_ALLOW' in profile_name:
        return 0x2
    elif 'IP_DF_ACTION_EDIT' in profile_name:
        return 0x0
    return 0

def GetInputIpv4Options(testcase, packet):
    profile_name = testcase.module.iterator.Get()
    return 0xDEADBEEF

def GetExpectedIpv4Options(testcase, packet):
    profile_name = testcase.module.iterator.Get()
    if 'IP_OPTIONS_ACTION_EDIT' in profile_name:
        return None
    return 0xDEADBEEF

def GetTriggerPacket(testcase):
    profile_name = testcase.module.iterator.Get()
    return testcase.packets.Get('PKT1')

def GetExpectedPacket(testcase):
    profile_name = testcase.module.iterator.Get()
    if 'ACTION_DROP' in profile_name:
        return None
    elif 'DROP_ENABLE' in profile_name:
        return None
    return testcase.packets.Get('PKT2')

def GetInputIcmpCode(testcase, packet):
    profile_name = testcase.module.iterator.Get()
    if 'ICMP_INVALID_CODE_ACTION_ALLOW' in profile_name:
        return 0x1
    elif 'ICMP_INVALID_CODE_ACTION_DROP' in profile_name:
        return 0x1
    elif 'ICMP_INVALID_CODE_ACTION_EDIT' in profile_name:
        return 0x1
    return 0

def GetExpectedIcmpCode(testcase, packet):
    profile_name = testcase.module.iterator.Get()
    if 'ICMP_INVALID_CODE_ACTION_ALLOW' in profile_name:
        return 0x1
    elif 'ICMP_INVALID_CODE_ACTION_DROP' in profile_name:
        return 0x1
    elif 'ICMP_INVALID_CODE_ACTION_EDIT' in profile_name:
        return 0x0
    return 0

def GetInputIcmpType(testcase, packet):
    profile_name = testcase.module.iterator.Get()
    if 'ICMP_INVALID_CODE_ACTION' in profile_name:
        return 0x0
    elif 'ICMP_DEPRECATED_MSGS_DROP_ENABLE' in profile_name:
        return 0x4
    elif 'ICMP_DEPRECATED_MSGS_DROP_DISABLE' in profile_name:
        return 0x4
    elif 'ICMP_REDIRECT_MSG_DROP_ENABLE' in profile_name:
        return 0x5
    elif 'ICMP_REDIRECT_MSG_DROP_DISABLE' in profile_name:
        return 0x5
    return 0

def GetExpectedIcmpType(testcase, packet):
    profile_name = testcase.module.iterator.Get()
    if 'ICMP_INVALID_CODE_ACTION' in profile_name:
        return 0x0
    elif 'ICMP_DEPRECATED_MSGS_DROP_ENABLE' in profile_name:
        return 0x4
    elif 'ICMP_DEPRECATED_MSGS_DROP_DISABLE' in profile_name:
        return 0x4
    elif 'ICMP_REDIRECT_MSG_DROP_ENABLE' in profile_name:
        return 0x5
    elif 'ICMP_REDIRECT_MSG_DROP_DISABLE' in profile_name:
        return 0x5
    return 0

def GetInputTcpReserved(testcase, packet):
    profile_name = testcase.module.iterator.Get()
    if 'TCP_RSVD_FLAGS_ACTION' in profile_name:
        return 0x1
    return 0x0

def GetInputTcpFlags(testcase, packet):
    #TBD: TCP_URG_PAYLOAD_MISSING_ACTION_ALLOW
    profile_name = testcase.module.iterator.Get()
    if 'TCP_UNEXPECTED_MSS_ACTION_ALLOW' in profile_name:
        return None
    elif 'TCP_UNEXPECTED_MSS_ACTION_DROP' in profile_name:
        return None
    elif 'TCP_UNEXPECTED_MSS_ACTION_EDIT' in profile_name:
        return None
    elif 'TCP_UNEXPECTED_WIN_SCALE_ACTION_ALLOW' in profile_name:
        return None
    elif 'TCP_UNEXPECTED_WIN_SCALE_ACTION_DROP' in profile_name:
        return None
    elif 'TCP_UNEXPECTED_WIN_SCALE_ACTION_EDIT' in profile_name:
        return None
    elif 'TCP_URG_FLAG_NOT_SET_ACTION_ALLOW' in profile_name:
        return None
    elif 'TCP_URG_FLAG_NOT_SET_ACTION_DROP' in profile_name:
        return None
    elif 'TCP_URG_FLAG_NOT_SET_ACTION_EDIT' in profile_name:
        return None
    elif 'TCP_URG_PTR_NOT_SET_ACTION_ALLOW' in profile_name:
        return 'urg'
    elif 'TCP_URG_PTR_NOT_SET_ACTION_DROP' in profile_name:
        return 'urg'
    elif 'TCP_URG_PTR_NOT_SET_ACTION_EDIT' in profile_name:
        return 'urg'
    elif 'TCP_RST_WITH_DATA_ACTION_ALLOW' in profile_name:
        return 'rst'
    elif 'TCP_RST_WITH_DATA_ACTION_DROP' in profile_name:
        return 'rst'
    elif 'TCP_RST_WITH_DATA_ACTION_EDIT' in profile_name:
        return 'rst'
    # Need more invalid flags combinations
    elif 'TCP_INVALID_FLAGS_DROP_ENABLE' in profile_name:
        return 'syn,rst'
    elif 'TCP_INVALID_FLAGS_DROP_DISABLE' in profile_name:
        return 'syn,rst'
    elif 'TCP_FLAGS_NONSYN_NOACK_DROP_ENABLE' in profile_name:
        return None
    elif 'TCP_FLAGS_NONSYN_NOACK_DROP_DISABLE' in profile_name:
        return None
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_ALLOW' in profile_name:
        return None
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_DROP' in profile_name:
        return None
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_EDIT' in profile_name:
        return None
    return None

def GetInputTcpOptions(testcase, packet):
    profile_name = testcase.module.iterator.Get()
    echo_ts = []
    echo_ts.append(TcpOptions('Timestamp', '0x3 0x4'))
    echo_ts.append(TcpOptions('NOP', None))
    if 'TCP_UNEXPECTED_MSS_ACTION_ALLOW' in profile_name:
        return [TcpOptions('MSS', '0x1')]
    elif 'TCP_UNEXPECTED_MSS_ACTION_DROP' in profile_name:
        return [TcpOptions('MSS', '0x1')]
    elif 'TCP_UNEXPECTED_MSS_ACTION_EDIT' in profile_name:
        return [TcpOptions('MSS', '0x1')]
    elif 'TCP_UNEXPECTED_WIN_SCALE_ACTION_ALLOW' in profile_name:
        return [TcpOptions('WScale', '0x2')]
    elif 'TCP_UNEXPECTED_WIN_SCALE_ACTION_DROP' in profile_name:
        return [TcpOptions('WScale', '0x2')]
    elif 'TCP_UNEXPECTED_WIN_SCALE_ACTION_EDIT' in profile_name:
        return [TcpOptions('WScale', '0x2')]
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_ALLOW' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_DROP' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_EDIT' in profile_name:
        return echo_ts
    return []

def GetInputTcpUrgPtr(testcase, packet):
    profile_name = testcase.module.iterator.Get()
    if 'TCP_URG_FLAG_NOT_SET_ACTION_ALLOW' in profile_name:
        return 0x1
    elif 'TCP_URG_FLAG_NOT_SET_ACTION_DROP' in profile_name:
        return 0x1
    elif 'TCP_URG_FLAG_NOT_SET_ACTION_EDIT' in profile_name:
        return 0x1
    return 0x0

def GetExpectedTcpReserved(testcase, packet):
    profile_name = testcase.module.iterator.Get()
    if 'TCP_RSVD_FLAGS_ACTION_ALLOW' in profile_name:
        return 0x1
    elif 'TCP_RSVD_FLAGS_ACTION_DROP' in profile_name:
        return 0x1
    elif 'TCP_RSVD_FLAGS_ACTION_EDIT' in profile_name:
        return 0x0
    return 0x0

def GetExpectedTcpFlags(testcase, packet):
    profile_name = testcase.module.iterator.Get()
    if 'TCP_UNEXPECTED_MSS_ACTION_ALLOW' in profile_name:
        return None
    elif 'TCP_UNEXPECTED_MSS_ACTION_DROP' in profile_name:
        return None
    elif 'TCP_UNEXPECTED_MSS_ACTION_EDIT' in profile_name:
        return 'syn'
    elif 'TCP_UNEXPECTED_WIN_SCALE_ACTION_ALLOW' in profile_name:
        return None
    elif 'TCP_UNEXPECTED_WIN_SCALE_ACTION_DROP' in profile_name:
        return None
    elif 'TCP_UNEXPECTED_WIN_SCALE_ACTION_EDIT' in profile_name:
        return 'syn'
    elif 'TCP_URG_FLAG_NOT_SET_ACTION_ALLOW' in profile_name:
        return None
    elif 'TCP_URG_FLAG_NOT_SET_ACTION_DROP' in profile_name:
        return None
    elif 'TCP_URG_FLAG_NOT_SET_ACTION_EDIT' in profile_name:
        return None
    elif 'TCP_URG_PTR_NOT_SET_ACTION_ALLOW' in profile_name:
        return 'urg'
    elif 'TCP_URG_PTR_NOT_SET_ACTION_DROP' in profile_name:
        return 'urg'
    elif 'TCP_URG_PTR_NOT_SET_ACTION_EDIT' in profile_name:
        return None
    elif 'TCP_RST_WITH_DATA_ACTION_ALLOW' in profile_name:
        return 'rst'
    elif 'TCP_RST_WITH_DATA_ACTION_DROP' in profile_name:
        return 'rst'
    elif 'TCP_RST_WITH_DATA_ACTION_EDIT' in profile_name:
        return None
    # Need more invalid flags combinations
    elif 'TCP_INVALID_FLAGS_DROP_ENABLE' in profile_name:
        return None
    elif 'TCP_INVALID_FLAGS_DROP_DISABLE' in profile_name:
        return 'syn,rst'
    elif 'TCP_FLAGS_NONSYN_NOACK_DROP_ENABLE' in profile_name:
        return None
    elif 'TCP_FLAGS_NONSYN_NOACK_DROP_DISABLE' in profile_name:
        return None
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_ALLOW' in profile_name:
        return None
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_DROP' in profile_name:
        return None
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_EDIT' in profile_name:
        return None
    return None

def GetExpectedTcpOptions(testcase, packet):
    profile_name = testcase.module.iterator.Get()
    echo_ts = []
    echo_ts_edit = []
    echo_ts.append(TcpOptions('Timestamp', '0x3 0x4'))
    echo_ts.append(TcpOptions('NOP', None))
    echo_ts_edit.append(TcpOptions('Timestamp', '0x3 0x0'))
    echo_ts_edit.append(TcpOptions('NOP', None))
    if 'TCP_UNEXPECTED_MSS_ACTION_ALLOW' in profile_name:
        return [TcpOptions('MSS', '0x1')]
    elif 'TCP_UNEXPECTED_MSS_ACTION_DROP' in profile_name:
        return [TcpOptions('MSS', '0x1')]
    elif 'TCP_UNEXPECTED_MSS_ACTION_EDIT' in profile_name:
        return [TcpOptions('NOP', None)]
    elif 'TCP_UNEXPECTED_WIN_SCALE_ACTION_ALLOW' in profile_name:
        return [TcpOptions('WScale', '0x2')]
    elif 'TCP_UNEXPECTED_WIN_SCALE_ACTION_DROP' in profile_name:
        return [TcpOptions('WScale', '0x2')]
    elif 'TCP_UNEXPECTED_WIN_SCALE_ACTION_EDIT' in profile_name:
        return [TcpOptions('NOP', None)]
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_ALLOW' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_DROP' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_EDIT' in profile_name:
        return echo_ts_edit
    return []

def GetExpectedTcpUrgPtr(testcase, packet):
    profile_name = testcase.module.iterator.Get()
    if 'TCP_URG_FLAG_NOT_SET_ACTION_ALLOW' in profile_name:
        return 0x1
    elif 'TCP_URG_FLAG_NOT_SET_ACTION_DROP' in profile_name:
        return 0x1
    elif 'TCP_URG_FLAG_NOT_SET_ACTION_EDIT' in profile_name:
        return 0x0
    elif 'TCP_URG_PTR_NOT_SET_ACTION_ALLOW' in profile_name:
        return 0x0
    elif 'TCP_URG_PTR_NOT_SET_ACTION_DROP' in profile_name:
        return 0x0
    elif 'TCP_URG_PTR_NOT_SET_ACTION_EDIT' in profile_name:
        return 0x0
    return 0x0


def GetInputPayloadSize(testcase, packet):
    return 150

def GetExpectedPayloadSize(testcase, packet):
    return 150


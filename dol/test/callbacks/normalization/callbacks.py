#! /usr/bin/python3
import pdb

class TcpOptions:
    def __init__(self, kind, data):
        self.kind = kind
        self.data = data
        return

def GetInputIpv4Flags(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
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
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
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
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'IP_OPTIONS_ACTION' in profile_name:
        return 0xDEADBEEF
    return None

def GetExpectedIpv4Options(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'IP_OPTIONS_ACTION_ALLOW' in profile_name:
        return 0xDEADBEEF
    elif 'IP_OPTIONS_ACTION_DENY' in profile_name:
        return 0xDEADBEEF
    elif 'IP_OPTIONS_ACTION_EDIT' in profile_name:
        return None
    return None

def GetTriggerPacket(testcase):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    return testcase.packets.Get('PKT1')

def GetExpectedPacket(testcase):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'TCP_UNEXPECTED_TS_OPTION_UNSET_ACTION_DROP' in profile_name:
        # Only drop case where packet is actually allowed
        return testcase.packets.Get('PKT2')
    elif 'ACTION_DROP' in profile_name:
        return None
    elif '_DROP_ENABLE' in profile_name:
        return None
    return testcase.packets.Get('PKT2')

def GetInputIcmpCode(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'ICMP_INVALID_CODE_ECHO_REQ_ACTION_ALLOW' in profile_name:
        return 0x1
    if 'ICMP_INVALID_CODE_ECHO_REQ_ACTION_DROP' in profile_name:
        return 0x1
    if 'ICMP_INVALID_CODE_ECHO_REQ_ACTION_EDIT' in profile_name:
        return 0x1
    if 'ICMP_INVALID_CODE_ECHO_REP_ACTION_ALLOW' in profile_name:
        return 0x1
    if 'ICMP_INVALID_CODE_ECHO_REP_ACTION_DROP' in profile_name:
        return 0x1
    if 'ICMP_INVALID_CODE_ECHO_REP_ACTION_EDIT' in profile_name:
        return 0x1
    return 0x0

def GetExpectedIcmpCode(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'ICMP_INVALID_CODE_ECHO_REQ_ACTION_ALLOW' in profile_name:
        return 0x1
    if 'ICMP_INVALID_CODE_ECHO_REQ_ACTION_DROP' in profile_name:
        return 0x1
    if 'ICMP_INVALID_CODE_ECHO_REQ_ACTION_EDIT' in profile_name:
        return 0x0
    if 'ICMP_INVALID_CODE_ECHO_REP_ACTION_ALLOW' in profile_name:
        return 0x1
    if 'ICMP_INVALID_CODE_ECHO_REP_ACTION_DROP' in profile_name:
        return 0x1
    if 'ICMP_INVALID_CODE_ECHO_REP_ACTION_EDIT' in profile_name:
        return 0x0
    return 0x0

def GetInputIcmpType(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'ECHO_REQ_ACTION' in profile_name:
        return 0x8
    elif 'ECHO_REP_ACTION' in profile_name:
        return 0x0
    elif 'ICMP_DEPRECATED_MSGS_TYPE4_DROP_ENABLE' in profile_name:
        return 0x4
    elif 'ICMP_DEPRECATED_MSGS_TYPE4_DROP_DISABLE' in profile_name:
        return 0x4
    elif 'ICMP_DEPRECATED_MSGS_TYPE6_DROP_ENABLE' in profile_name:
        return 0x6
    elif 'ICMP_DEPRECATED_MSGS_TYPE6_DROP_DISABLE' in profile_name:
        return 0x6
    elif 'ICMP_DEPRECATED_MSGS_TYPE15_DROP_ENABLE' in profile_name:
        return 0xf
    elif 'ICMP_DEPRECATED_MSGS_TYPE15_DROP_DISABLE' in profile_name:
        return 0xf
    elif 'ICMP_DEPRECATED_MSGS_TYPE39_DROP_ENABLE' in profile_name:
        return 0x27
    elif 'ICMP_DEPRECATED_MSGS_TYPE39_DROP_DISABLE' in profile_name:
        return 0x27
    elif 'ICMP_DEPRECATED_MSGS_TYPE27_DROP_ENABLE' in profile_name:
        return 0x1b
    elif 'ICMP_DEPRECATED_MSGS_TYPE27_DROP_DISABLE' in profile_name:
        return 0x1b
    elif 'ICMP_REDIRECT_MSG_DROP_ENABLE' in profile_name:
        return 0x5
    elif 'ICMP_REDIRECT_MSG_DROP_DISABLE' in profile_name:
        return 0x5
    return 0

def GetExpectedIcmpType(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'ECHO_REQ_ACTION' in profile_name:
        return 0x8
    elif 'ECHO_REP_ACTION' in profile_name:
        return 0x0
    elif 'ICMP_DEPRECATED_MSGS_TYPE4_DROP_ENABLE' in profile_name:
        return 0x4
    elif 'ICMP_DEPRECATED_MSGS_TYPE4_DROP_DISABLE' in profile_name:
        return 0x4
    elif 'ICMP_DEPRECATED_MSGS_TYPE6_DROP_ENABLE' in profile_name:
        return 0x6
    elif 'ICMP_DEPRECATED_MSGS_TYPE6_DROP_DISABLE' in profile_name:
        return 0x6
    elif 'ICMP_DEPRECATED_MSGS_TYPE15_DROP_ENABLE' in profile_name:
        return 0xf
    elif 'ICMP_DEPRECATED_MSGS_TYPE15_DROP_DISABLE' in profile_name:
        return 0xf
    elif 'ICMP_DEPRECATED_MSGS_TYPE39_DROP_ENABLE' in profile_name:
        return 0x27
    elif 'ICMP_DEPRECATED_MSGS_TYPE39_DROP_DISABLE' in profile_name:
        return 0x27
    elif 'ICMP_DEPRECATED_MSGS_TYPE27_DROP_ENABLE' in profile_name:
        return 0x1b
    elif 'ICMP_DEPRECATED_MSGS_TYPE27_DROP_DISABLE' in profile_name:
        return 0x1b
    elif 'ICMP_REDIRECT_MSG_DROP_ENABLE' in profile_name:
        return 0x5
    elif 'ICMP_REDIRECT_MSG_DROP_DISABLE' in profile_name:
        return 0x5
    return 0

def GetInputTcpReserved(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'TCP_RSVD_FLAGS_ACTION' in profile_name:
        return 0x1
    return 0x0

def GetInputTcpFlags(testcase, packet):
    #TBD: TCP_URG_PAYLOAD_MISSING_ACTION_ALLOW
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
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
    elif 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_ENABLE' in profile_name:
        return 'syn'
    elif 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_DISABLE' in profile_name:
        return 'syn'
    return None

def GetInputTcpOptions(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
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
    elif 'TCP_UNEXPECTED_TS_OPTION_SET_ACTION_ALLOW' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_SET_ACTION_DROP' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_SET_ACTION_EDIT' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_UNSET_ACTION_ALLOW' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_UNSET_ACTION_DROP' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_UNSET_ACTION_EDIT' in profile_name:
        return echo_ts
    elif 'TCP_TS_NOT_PRESENT_DROP_ENABLE' in profile_name:
        return echo_ts
    elif 'TCP_TS_NOT_PRESENT_DROP_DISABLE' in profile_name:
        return echo_ts
    return []

def GetInputTcpUrgPtr(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'TCP_URG_FLAG_NOT_SET_ACTION_ALLOW' in profile_name:
        return 0x1
    elif 'TCP_URG_FLAG_NOT_SET_ACTION_DROP' in profile_name:
        return 0x1
    elif 'TCP_URG_FLAG_NOT_SET_ACTION_EDIT' in profile_name:
        return 0x1
    return 0x0

def GetExpectedTcpReserved(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'TCP_RSVD_FLAGS_ACTION_ALLOW' in profile_name:
        return 0x1
    elif 'TCP_RSVD_FLAGS_ACTION_DROP' in profile_name:
        return 0x1
    elif 'TCP_RSVD_FLAGS_ACTION_EDIT' in profile_name:
        return 0x0
    return 0x0

def GetExpectedTcpFlags(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
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
    elif 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_ENABLE' in profile_name:
        return 'syn'
    elif 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_DISABLE' in profile_name:
        return 'syn'
    return None

def GetExpectedTcpOptions(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    echo_ts = []
    echo_ts_edit = []
    echo_ts_nop_edit = []
    
    echo_ts.append(TcpOptions('Timestamp', '0x3 0x4'))
    echo_ts.append(TcpOptions('NOP', None))
    
    echo_ts_edit.append(TcpOptions('Timestamp', '0x3 0x0'))
    echo_ts_edit.append(TcpOptions('NOP', None))
    
    echo_ts_nop_edit.append(TcpOptions('NOP', None))
    echo_ts_nop_edit.append(TcpOptions('NOP', None))
    echo_ts_nop_edit.append(TcpOptions('NOP', None))
    echo_ts_nop_edit.append(TcpOptions('NOP', None))
    echo_ts_nop_edit.append(TcpOptions('NOP', None))

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
    elif 'TCP_UNEXPECTED_TS_OPTION_SET_ACTION_ALLOW' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_SET_ACTION_DROP' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_SET_ACTION_EDIT' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_UNSET_ACTION_ALLOW' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_UNSET_ACTION_DROP' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_UNSET_ACTION_EDIT' in profile_name:
        return echo_ts
    elif 'TCP_TS_NOT_PRESENT_DROP_ENABLE' in profile_name:
        return echo_ts
    elif 'TCP_TS_NOT_PRESENT_DROP_DISABLE' in profile_name:
        return echo_ts
    return []

def GetExpectedTcpUrgPtr(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
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
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'TCP_DATA_LEN_GT_MSS_ACTION_ALLOW' in profile_name:
        return 1400
    elif 'TCP_DATA_LEN_GT_MSS_ACTION_DROP' in profile_name:
        return 1400
    elif 'TCP_DATA_LEN_GT_MSS_ACTION_EDIT' in profile_name:
        return 1400
    elif 'TCP_DATA_LEN_GT_WIN_SIZE_ACTION_ALLOW' in profile_name:
        return 1400
    elif 'TCP_DATA_LEN_GT_WIN_SIZE_ACTION_DROP' in profile_name:
        return 1400
    elif 'TCP_DATA_LEN_GT_WIN_SIZE_ACTION_EDIT' in profile_name:
        return 1400
    elif 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_ENABLE' in profile_name:
        return 0
    elif 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_DISABLE' in profile_name:
        return 0
    return 150

def GetExpectedPayloadSize(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'TCP_DATA_LEN_GT_MSS_ACTION_ALLOW' in profile_name:
        return 1400
    elif 'TCP_DATA_LEN_GT_MSS_ACTION_DROP' in profile_name:
        return 1400
    elif 'TCP_DATA_LEN_GT_MSS_ACTION_EDIT' in profile_name:
        return 1000
    elif 'TCP_DATA_LEN_GT_WIN_SIZE_ACTION_ALLOW' in profile_name:
        return 1400
    elif 'TCP_DATA_LEN_GT_WIN_SIZE_ACTION_DROP' in profile_name:
        return 1400
    elif 'TCP_DATA_LEN_GT_WIN_SIZE_ACTION_EDIT' in profile_name:
        return 1000
    elif 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_ENABLE' in profile_name:
        return 0
    elif 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_DISABLE' in profile_name:
        return 0
    return 150

def GetInputTcpAck(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_ENABLE' in profile_name:
        return 0
    elif 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_DISABLE' in profile_name:
        return 0
    return 0

def GetExpectedTcpAck(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_ENABLE' in profile_name:
        return 0
    elif 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_DISABLE' in profile_name:
        return 0
    return 0

def GetExpectedCpuPacket(testcase):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'SEC_PROF_TCP_NON_SYN_FIRST_PKT_DROP_ENABLE' in profile_name:
        return None
    elif 'SEC_PROF_TCP_NON_SYN_FIRST_PKT_DROP_DISABLE' in profile_name:
        return testcase.packets.Get('PKT2')
    return None

#! /usr/bin/python3
import pdb

import infra.penscapy.penscapy as penscapy

class TcpOptions:
    def __init__(self, kind, data):
        self.kind = kind
        self.data = data
        return

def GetInputIpv4Len(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    return None

def GetExpectedIpv4Len(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    return None

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

def GetInputIpv4Ttl(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'IP_TTL_NORMALIZE_32' in profile_name:
        return 64
    return 64

def GetExpectedIpv4Ttl(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'IP_TTL_NORMALIZE_32' in profile_name:
        return 32
    return 64

def __get_ipv4_option():
    return penscapy.IPOption(bytes([0x83, 0x03, 0x10]))

def GetInputIpv4Options(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'IP_OPTIONS_ACTION' in profile_name:
        return __get_ipv4_option()
    return None

def GetExpectedIpv4Options(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'IP_OPTIONS_ACTION_ALLOW' in profile_name:
        return __get_ipv4_option()
    elif 'IP_OPTIONS_ACTION_DROP' in profile_name:
        return __get_ipv4_option()
    elif 'IP_OPTIONS_ACTION_EDIT' in profile_name:
        return None
    return None

def __get_ipv6_option():
    return [penscapy.IPv6ExtHdrHopByHop()]

# For multiple IPv6 options
# return [penscapy.IPv6ExtHdrHopByHop(), penscapy.IPv6ExtHdrDestOpt(), penscapy.IPv6ExtHdrRouting(), penscapy.IPv6ExtHdrFragment()]

def GetInputIpv6Options(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'IPV6_OPTIONS_ACTION' in profile_name:
        return __get_ipv6_option()
    return None

def GetExpectedIpv6Options(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'IPV6_OPTIONS_ACTION_ALLOW' in profile_name:
        return __get_ipv6_option()
    elif 'IPV6_OPTIONS_ACTION_DROP' in profile_name:
        return __get_ipv6_option()
    elif 'IPV6_OPTIONS_ACTION_EDIT' in profile_name:
        return None
    return None

def GetInputIpv6Hlim(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'IPV6_HOP_LIMIT_NORMALIZE_32' in profile_name:
        return 64
    return 64

def GetExpectedIpv6Hlim(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'IPV6_HOP_LIMIT_NORMALIZE_32' in profile_name:
        return 32
    return 64

def GetTriggerPacket(testcase):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    return testcase.packets.Get('PKT1')

def GetExpectedPacket(testcase):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'TCP_UNEXPECTED_TS_OPTION_NEG_ACTION_DROP' in profile_name:
        # Only drop case where packet is actually allowed
        return testcase.packets.Get('PKT2')
    elif 'TCP_UNEXPECTED_SACK_OPTION_NEG_ACTION_DROP' in profile_name:
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
    if 'TCP_URG_PTR_NOT_SET_ACTION_ALLOW' in profile_name:
        return 'urg,ack'
    elif 'TCP_URG_PTR_NOT_SET_ACTION_DROP' in profile_name:
        return 'urg,ack'
    elif 'TCP_URG_PTR_NOT_SET_ACTION_EDIT' in profile_name:
        return 'urg,ack'
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_ALLOW' in profile_name:
        return 'urg,ack'
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_DROP' in profile_name:
        return 'urg,ack'
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_EDIT' in profile_name:
        return 'urg,ack'
    elif 'TCP_RST_WITH_DATA_ACTION_ALLOW' in profile_name:
        return 'rst'
    elif 'TCP_RST_WITH_DATA_ACTION_DROP' in profile_name:
        return 'rst'
    elif 'TCP_RST_WITH_DATA_ACTION_EDIT' in profile_name:
        return 'rst'
    # Need more invalid flags combinations
    elif 'TCP_INVALID_FLAGS_1_DROP_ENABLE' in profile_name:
        return 'syn,rst'
    elif 'TCP_INVALID_FLAGS_1_DROP_DISABLE' in profile_name:
        return 'syn,rst'
    elif 'TCP_INVALID_FLAGS_2_DROP_ENABLE' in profile_name:
        return 'syn,fin'
    elif 'TCP_INVALID_FLAGS_2_DROP_DISABLE' in profile_name:
        return 'syn,fin'
    elif 'TCP_INVALID_FLAGS_3_DROP_ENABLE' in profile_name:
        return 'cwr,ece,urg,ack,psh,rst,syn,fin'
    elif 'TCP_INVALID_FLAGS_3_DROP_DISABLE' in profile_name:
        return 'cwr,ece,urg,ack,psh,rst,syn,fin'
    elif 'TCP_INVALID_FLAGS_4_DROP_ENABLE' in profile_name:
        return None # No flags specific case
    elif 'TCP_INVALID_FLAGS_4_DROP_DISABLE' in profile_name:
        return None # No flags specific case
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
    elif 'SEC_PROF_TCP_NORMALIZE_MSS_WITH_MSS_OPTION' in profile_name:
        return 'syn'
    elif 'SEC_PROF_TCP_NORMALIZE_MSS_WITHOUT_MSS_OPTION' in profile_name:
        return 'syn'
    elif 'SEC_PROF_TCP_NORMALIZE_MSS_WITH_WS_ONLY_OPTION' in profile_name:
        return 'syn'
    return 'ack'

def GetInputTcpOptions(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    echo_ts = []
    echo_ts.append(TcpOptions('Timestamp', '0x3 0x4'))
    echo_ts.append(TcpOptions('NOP', None))
    sack = []
    sack.append(TcpOptions('SAck', '12345678 12346000'))
    sack.append(TcpOptions('NOP', None))
    ts_sack_1 = []
    ts_sack_1.append(TcpOptions('Timestamp', '0x3 0x4'))
    ts_sack_1.append(TcpOptions('SAck', '100 200'))
    ts_sack_2 = []
    ts_sack_2.append(TcpOptions('Timestamp', '0x3 0x4'))
    ts_sack_2.append(TcpOptions('SAck', '100 200 300 400'))
    ts_sack_3 = []
    ts_sack_3.append(TcpOptions('Timestamp', '0x3 0x4'))
    ts_sack_3.append(TcpOptions('SAck', '100 200 300 400 500 600'))
    ws_sack_4 = []
    ws_sack_4.append(TcpOptions('WScale', '0x2'))
    ws_sack_4.append(TcpOptions('NOP', None))
    ws_sack_4.append(TcpOptions('SAck', '100 200 300 400 500 600 700 800'))
    ws_sack_4.append(TcpOptions('NOP', None))
    ts_ws_sack_1 = []
    ts_ws_sack_1.append(TcpOptions('WScale', '0x2'))
    ts_ws_sack_1.append(TcpOptions('NOP', None))
    ts_ws_sack_1.append(TcpOptions('Timestamp', '0x3 0x4'))
    ts_ws_sack_1.append(TcpOptions('SAck', '100 200'))
    ws_mss_sack_perm = []
    ws_mss_sack_perm.append(TcpOptions('WScale', '0x2'))
    ws_mss_sack_perm.append(TcpOptions('MSS', '0x2'))
    ws_mss_sack_perm.append(TcpOptions('SAckOK', 'None'))
    ws_mss_sack_perm.append(TcpOptions('NOP', None))
    mss_ws_sack_perm = []
    mss_ws_sack_perm.append(TcpOptions('MSS', '0x2'))
    mss_ws_sack_perm.append(TcpOptions('WScale', '0x2'))
    mss_ws_sack_perm.append(TcpOptions('SAckOK', 'None'))
    mss_ws_sack_perm.append(TcpOptions('NOP', None))
    mss_ws_only = []
    mss_ws_only.append(TcpOptions('MSS', '0x1'))
    mss_ws_only.append(TcpOptions('WScale', '0x2'))
    normalize_mss_ws_only = []
    normalize_mss_ws_only.append(TcpOptions('WScale', '0x2'))

    if 'SEC_PROF_TCP_UNEXPECTED_WIN_SCALE_ACTION_EDIT_SACK_4' in profile_name:
        return ws_sack_4
    elif 'SEC_PROF_TCP_UNEXPECTED_WIN_SCALE_ACTION_EDIT_MSS_SACK_PERM' in profile_name:
        return ws_mss_sack_perm
    elif 'SEC_PROF_TCP_UNEXPECTED_MSS_ACTION_EDIT_WIN_SCALE_ONLY' in profile_name:
        return mss_ws_only
    elif 'SEC_PROF_TCP_UNEXPECTED_MSS_ACTION_EDIT_WIN_SCALE_SACK_PERM' in profile_name:
        return mss_ws_sack_perm
    elif 'TCP_UNEXPECTED_MSS_ACTION_ALLOW' in profile_name:
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
    elif 'TCP_UNEXPECTED_SACK_PERM_ACTION_ALLOW' in profile_name:
        return [TcpOptions('SAckOK', 'None')]
    elif 'TCP_UNEXPECTED_SACK_PERM_ACTION_DROP' in profile_name:
        return [TcpOptions('SAckOK', 'None')]
    elif 'TCP_UNEXPECTED_SACK_PERM_ACTION_EDIT' in profile_name:
        return [TcpOptions('SAckOK', 'None')]
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_ALLOW' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_DROP' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_EDIT' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_NEG_ACTION_ALLOW' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_NEG_ACTION_DROP' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_NEG_ACTION_EDIT' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_NOT_NEG_ACTION_ALLOW' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_NOT_NEG_ACTION_DROP' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_NOT_NEG_ACTION_EDIT' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_SACK_OPTION_NEG_ACTION_ALLOW' in profile_name:
        return sack
    elif 'TCP_UNEXPECTED_SACK_OPTION_NEG_ACTION_DROP' in profile_name:
        return sack
    elif 'TCP_UNEXPECTED_SACK_OPTION_NEG_ACTION_EDIT' in profile_name:
        return sack
    elif 'TCP_UNEXPECTED_SACK_OPTION_NOT_NEG_ACTION_ALLOW' in profile_name:
        return sack
    elif 'TCP_UNEXPECTED_SACK_OPTION_NOT_NEG_ACTION_DROP' in profile_name:
        return sack
    elif 'TCP_UNEXPECTED_SACK_OPTION_NOT_NEG_ACTION_EDIT' in profile_name:
        return sack
    elif 'SEC_PROF_TCP_UNEXPECTED_TS_OPTION_EDIT_SACK_1' in profile_name:
        return ts_sack_1
    elif 'SEC_PROF_TCP_UNEXPECTED_TS_OPTION_EDIT_SACK_2' in profile_name:
        return ts_sack_2
    elif 'SEC_PROF_TCP_UNEXPECTED_TS_OPTION_EDIT_SACK_3' in profile_name:
        return ts_sack_3
    elif 'SEC_PROF_TCP_UNEXPECTED_TS_OPTION_EDIT_WS_SACK_1' in profile_name:
        return ts_ws_sack_1
    elif 'TCP_TS_NOT_PRESENT_DROP_ENABLE' in profile_name:
        return []
    elif 'TCP_TS_NOT_PRESENT_DROP_DISABLE' in profile_name:
        return []
    elif 'TCP_NORMALIZE_MSS_WITH_MSS_OPTION' in profile_name:
        return [TcpOptions('MSS', '1000')]
    elif 'TCP_NORMALIZE_MSS_WITHOUT_MSS_OPTION' in profile_name:
        return []
    elif 'TCP_NORMALIZE_MSS_WITH_WS_ONLY_OPTION' in profile_name:
        return normalize_mss_ws_only
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
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_ALLOW' in profile_name:
        return 0x1
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_DROP' in profile_name:
        return 0x1
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_EDIT' in profile_name:
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
    if 'TCP_URG_PTR_NOT_SET_ACTION_ALLOW' in profile_name:
        return 'urg,ack'
    elif 'TCP_URG_PTR_NOT_SET_ACTION_DROP' in profile_name:
        return 'urg,ack'
    elif 'TCP_URG_PTR_NOT_SET_ACTION_EDIT' in profile_name:
        return 'ack'
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_ALLOW' in profile_name:
        return 'urg,ack'
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_DROP' in profile_name:
        return 'urg,ack'
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_EDIT' in profile_name:
        return 'ack'
    elif 'TCP_RST_WITH_DATA_ACTION_ALLOW' in profile_name:
        return 'rst'
    elif 'TCP_RST_WITH_DATA_ACTION_DROP' in profile_name:
        return 'rst'
    elif 'TCP_RST_WITH_DATA_ACTION_EDIT' in profile_name:
        return 'rst'
    # Need more invalid flags combinations
    elif 'TCP_INVALID_FLAGS_1_DROP_ENABLE' in profile_name:
        return 'syn,rst'
    elif 'TCP_INVALID_FLAGS_1_DROP_DISABLE' in profile_name:
        return 'syn,rst'
    elif 'TCP_INVALID_FLAGS_2_DROP_ENABLE' in profile_name:
        return 'syn,fin'
    elif 'TCP_INVALID_FLAGS_2_DROP_DISABLE' in profile_name:
        return 'syn,fin'
    elif 'TCP_INVALID_FLAGS_3_DROP_ENABLE' in profile_name:
        return 'cwr,ece,urg,ack,psh,rst,syn,fin'
    elif 'TCP_INVALID_FLAGS_3_DROP_DISABLE' in profile_name:
        return 'cwr,ece,urg,ack,psh,rst,syn,fin'
    elif 'TCP_INVALID_FLAGS_4_DROP_ENABLE' in profile_name:
        return None # No flags specific case
    elif 'TCP_INVALID_FLAGS_4_DROP_DISABLE' in profile_name:
        return None # No flags specific case
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
    elif 'SEC_PROF_TCP_NORMALIZE_MSS_WITH_MSS_OPTION' in profile_name:
        return 'syn'
    elif 'SEC_PROF_TCP_NORMALIZE_MSS_WITHOUT_MSS_OPTION' in profile_name:
        return 'syn'
    elif 'SEC_PROF_TCP_NORMALIZE_MSS_WITH_WS_ONLY_OPTION' in profile_name:
        return 'syn'
    return 'ack'

def GetExpectedTcpOptions(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    echo_ts = []
    echo_ts_edit = []
    echo_ts_nop_edit = []
    sack = []
   
    
    echo_ts.append(TcpOptions('Timestamp', '0x3 0x4'))
    echo_ts.append(TcpOptions('NOP', None))
    
    echo_ts_edit.append(TcpOptions('Timestamp', '0x3 0x0'))
    echo_ts_edit.append(TcpOptions('NOP', None))
    
    sack.append(TcpOptions('SAck', '12345678 12346000'))
    sack.append(TcpOptions('NOP', None))

    echo_ts_nop_edit.append(TcpOptions('NOP', None))
    echo_ts_nop_edit.append(TcpOptions('NOP', None))
    echo_ts_nop_edit.append(TcpOptions('NOP', None))
    echo_ts_nop_edit.append(TcpOptions('NOP', None))
    echo_ts_nop_edit.append(TcpOptions('NOP', None))

    ts_sack_1 = []
    ts_sack_1.append(TcpOptions('SAck', '100 200'))
    ts_sack_1.append(TcpOptions('NOP', None))
    ts_sack_2 = []
    ts_sack_2.append(TcpOptions('SAck', '100 200 300 400'))
    ts_sack_2.append(TcpOptions('NOP', None))
    ts_sack_3 = []
    ts_sack_3.append(TcpOptions('SAck', '100 200 300 400 500 600'))
    ts_sack_3.append(TcpOptions('NOP', None))
    ws_sack_4 = []
    ws_sack_4.append(TcpOptions('SAck', '100 200 300 400 500 600 700 800'))
    ws_sack_4.append(TcpOptions('NOP', None))
    ts_ws_sack_1 = []
    ts_ws_sack_1.append(TcpOptions('WScale', '0x2'))
    ts_ws_sack_1.append(TcpOptions('SAck', '100 200'))
    ts_ws_sack_1.append(TcpOptions('NOP', None))
    ts_ws_sack_1.append(TcpOptions('NOP', None))
    ws_mss_sack_perm = []
    ws_mss_sack_perm.append(TcpOptions('MSS', '0x2'))
    ws_mss_sack_perm.append(TcpOptions('SAckOK', 'None'))
    ws_mss_sack_perm.append(TcpOptions('NOP', None))
    mss_ws_sack_perm = []
    mss_ws_sack_perm.append(TcpOptions('WScale', '0x2'))
    mss_ws_sack_perm.append(TcpOptions('SAckOK', 'None'))
    mss_ws_sack_perm.append(TcpOptions('NOP', None))
    mss_ws_sack_perm.append(TcpOptions('NOP', None))
    mss_ws_only = []
    mss_ws_only.append(TcpOptions('WScale', '0x2'))
    normalize_mss_ws_only = []
    normalize_mss_ws_only.append(TcpOptions('MSS', '1460'))
    normalize_mss_ws_only.append(TcpOptions('WScale', '0x2'))

    if 'SEC_PROF_TCP_UNEXPECTED_WIN_SCALE_ACTION_EDIT_MSS_SACK_PERM' in profile_name:
        return ws_mss_sack_perm
    elif 'SEC_PROF_TCP_UNEXPECTED_MSS_ACTION_EDIT_WIN_SCALE_SACK_PERM' in profile_name:
        return mss_ws_sack_perm
    elif 'SEC_PROF_TCP_UNEXPECTED_MSS_ACTION_EDIT_WIN_SCALE_ONLY' in profile_name:
        return mss_ws_only
    elif 'SEC_PROF_TCP_UNEXPECTED_WIN_SCALE_ACTION_EDIT_SACK_4' in profile_name:
        return ws_sack_4
    elif 'TCP_UNEXPECTED_MSS_ACTION_ALLOW' in profile_name:
        return [TcpOptions('MSS', '0x1')]
    elif 'TCP_UNEXPECTED_MSS_ACTION_DROP' in profile_name:
        return [TcpOptions('MSS', '0x1')]
    elif 'TCP_UNEXPECTED_MSS_ACTION_EDIT' in profile_name:
        return []
    elif 'TCP_UNEXPECTED_WIN_SCALE_ACTION_ALLOW' in profile_name:
        return [TcpOptions('WScale', '0x2')]
    elif 'TCP_UNEXPECTED_WIN_SCALE_ACTION_DROP' in profile_name:
        return [TcpOptions('WScale', '0x2')]
    elif 'TCP_UNEXPECTED_WIN_SCALE_ACTION_EDIT' in profile_name:
        return []
    elif 'TCP_UNEXPECTED_SACK_PERM_ACTION_ALLOW' in profile_name:
        return [TcpOptions('SAckOK', 'None')]
    elif 'TCP_UNEXPECTED_SACK_PERM_ACTION_DROP' in profile_name:
        return [TcpOptions('SAckOK', 'None')]
    elif 'TCP_UNEXPECTED_SACK_PERM_ACTION_EDIT' in profile_name:
        return []
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_ALLOW' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_DROP' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_ECHO_TS_ACTION_EDIT' in profile_name:
        return echo_ts_edit
    elif 'TCP_UNEXPECTED_TS_OPTION_NEG_ACTION_ALLOW' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_NEG_ACTION_DROP' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_NEG_ACTION_EDIT' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_NOT_NEG_ACTION_ALLOW' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_NOT_NEG_ACTION_DROP' in profile_name:
        return echo_ts
    elif 'TCP_UNEXPECTED_TS_OPTION_NOT_NEG_ACTION_EDIT' in profile_name:
        return []
    elif 'TCP_UNEXPECTED_SACK_OPTION_NEG_ACTION_ALLOW' in profile_name:
        return sack
    elif 'TCP_UNEXPECTED_SACK_OPTION_NEG_ACTION_DROP' in profile_name:
        return sack
    elif 'TCP_UNEXPECTED_SACK_OPTION_NEG_ACTION_EDIT' in profile_name:
        return sack
    elif 'TCP_UNEXPECTED_SACK_OPTION_NOT_NEG_ACTION_ALLOW' in profile_name:
        return sack
    elif 'TCP_UNEXPECTED_SACK_OPTION_NOT_NEG_ACTION_DROP' in profile_name:
        return sack
    elif 'TCP_UNEXPECTED_SACK_OPTION_NOT_NEG_ACTION_EDIT' in profile_name:
        return []
    elif 'SEC_PROF_TCP_UNEXPECTED_TS_OPTION_EDIT_SACK_1' in profile_name:
        return ts_sack_1
    elif 'SEC_PROF_TCP_UNEXPECTED_TS_OPTION_EDIT_SACK_2' in profile_name:
        return ts_sack_2
    elif 'SEC_PROF_TCP_UNEXPECTED_TS_OPTION_EDIT_SACK_3' in profile_name:
        return ts_sack_3
    elif 'SEC_PROF_TCP_UNEXPECTED_TS_OPTION_EDIT_WS_SACK_1' in profile_name:
        return ts_ws_sack_1
    elif 'TCP_TS_NOT_PRESENT_DROP_ENABLE' in profile_name:
        return []
    elif 'TCP_TS_NOT_PRESENT_DROP_DISABLE' in profile_name:
        return []
    elif 'TCP_NORMALIZE_MSS_WITH_MSS_OPTION' in profile_name:
        return [TcpOptions('MSS', '1460')]
    elif 'TCP_NORMALIZE_MSS_WITHOUT_MSS_OPTION' in profile_name:
        return [TcpOptions('MSS', '1460')]
    elif 'TCP_NORMALIZE_MSS_WITH_WS_ONLY_OPTION' in profile_name:
        return normalize_mss_ws_only
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
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_ALLOW' in profile_name:
        return 0x1
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_DROP' in profile_name:
        return 0x1
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_EDIT' in profile_name:
        return 0x0
    return 0x0

def GetInputPaddingSize(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_DISABLE' in profile_name:
        return 2
    elif 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_ENABLE' in profile_name:
        return 2
    elif 'INVALID_LEN_ACTION_ALLOW' in profile_name:
        return 200
    elif 'INVALID_LEN_ACTION_DROP' in profile_name:
        return 200
    elif 'INVALID_LEN_ACTION_EDIT' in profile_name:
        return 200
    # Adding padding for this case because for native packets the packet length
    # for syn packets with no options and data will be 54/58 bytes and the padding
    # will be done in model automatically but now when that packet comes out with
    # mss option and the packet lenght becomes greater than 60 then the DOL doesn't
    # add pad but the packet comes out with the padding that was sent. So to avoid
    # confusiion in checking always sending the packet greater than 60 in all cases
    # where we insert mss option.
    elif 'TCP_NORMALIZE_MSS_WITHOUT_MSS_OPTION' in profile_name:
        return 10
    elif 'TCP_NORMALIZE_MSS_WITH_WS_ONLY_OPTION' in profile_name:
        return 10
    elif 'SEC_PROF_TCP_URG_PAYLOAD_MISSING_ACTION_ALLOW' in profile_name:
        return 2
    elif 'SEC_PROF_TCP_URG_PAYLOAD_MISSING_ACTION_DROP' in profile_name:
        return 2
    elif 'SEC_PROF_TCP_URG_PAYLOAD_MISSING_ACTION_EDIT' in profile_name:
        return 2
    return 0

def GetExpectedPaddingSize(testcase, packet):
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'INVALID_LEN_ACTION_ALLOW' in profile_name:
        return 200
    elif 'INVALID_LEN_ACTION_DROP' in profile_name:
        return 200
    elif 'INVALID_LEN_ACTION_EDIT' in profile_name:
        return 0
    elif 'TCP_NORMALIZE_MSS_WITHOUT_MSS_OPTION' in profile_name:
        return 10
    elif 'TCP_NORMALIZE_MSS_WITH_WS_ONLY_OPTION' in profile_name:
        return 10
    return 0

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
        return 410
    elif 'TCP_DATA_LEN_GT_WIN_SIZE_ACTION_DROP' in profile_name:
        return 410
    elif 'TCP_DATA_LEN_GT_WIN_SIZE_ACTION_EDIT' in profile_name:
        return 410
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_ALLOW' in profile_name:
        return 0
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_DROP' in profile_name:
        return 0
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_EDIT' in profile_name:
        return 0
    elif 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_ENABLE' in profile_name:
        return 0
    elif 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_DISABLE' in profile_name:
        return 0
    elif 'TCP_NORMALIZE_MSS_WITH_MSS_OPTION' in profile_name:
        return 0
    elif 'TCP_NORMALIZE_MSS_WITHOUT_MSS_OPTION' in profile_name:
        return 0
    elif 'TCP_NORMALIZE_MSS_WITH_WS_ONLY_OPTION' in profile_name:
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
        return 410
    elif 'TCP_DATA_LEN_GT_WIN_SIZE_ACTION_DROP' in profile_name:
        return 410
    elif 'TCP_DATA_LEN_GT_WIN_SIZE_ACTION_EDIT' in profile_name:
        return 400
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_ALLOW' in profile_name:
        return 0
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_DROP' in profile_name:
        return 0
    elif 'TCP_URG_PAYLOAD_MISSING_ACTION_EDIT' in profile_name:
        return 0
    elif 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_ENABLE' in profile_name:
        return 0
    elif 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_DISABLE' in profile_name:
        return 0
    elif 'TCP_RST_WITH_DATA_ACTION_EDIT' in profile_name:
        return 0
    elif 'TCP_NORMALIZE_MSS_WITH_MSS_OPTION' in profile_name:
        return 0
    elif 'TCP_NORMALIZE_MSS_WITHOUT_MSS_OPTION' in profile_name:
        return 0
    elif 'TCP_NORMALIZE_MSS_WITH_WS_ONLY_OPTION' in profile_name:
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

def GetExpectedCpuPacket(testcase, args):
    pktid = getattr(args, 'pktid', None)
    if pktid is None:
        pktid = 'PKT2'
    iterelem = testcase.module.iterator.Get()
    profile_name = iterelem.profile
    if 'SEC_PROF_TCP_NON_SYN_FIRST_PKT_DROP_ENABLE' in profile_name:
        return None
    elif 'SEC_PROF_TCP_NON_SYN_FIRST_PKT_DROP_DISABLE' in profile_name:
        return testcase.packets.Get(pktid)
    return None

def GetPaddingSize(tc, pkt):
    iterelem = tc.module.iterator.Get()
    profile_name = iterelem.profile
    if 'SEC_PROF_TCP_SPLIT_HANDSHAKE_DROP_DISABLE' in profile_name:
        return 2
    return 0

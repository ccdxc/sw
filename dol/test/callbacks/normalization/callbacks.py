#! /usr/bin/python3
import pdb

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

def GetInputPayloadSize(testcase, packet):
    return 150

def GetExpectedPayloadSize(testcase, packet):
    return 150



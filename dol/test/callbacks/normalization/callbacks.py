#! /usr/bin/python3
import pdb

def GetInputIpv4Flags(testcase, packet):
    profile_name = testcase.module.iterator.Get()
    if 'IP_RSVD_FLAG' in profile_name:
        return 0x4
    return 0x0

def GetExpectedIpv4Flags(testcase, packet):
    return 0

def GetExpectedPacket(testcase):
    profile_name = testcase.module.iterator.Get()
    if 'DROP' in profile_name:
        return None
    return testcase.packets.Get('PKT2')

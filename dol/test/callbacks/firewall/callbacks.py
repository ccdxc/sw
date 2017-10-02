#! /usr/bin/python3

def GetForwardingCopy(tc, args = None):
    return tc.packets.Get(args.pktid)

def GetCpuCopy(tc, args = None):
    if tc.tracker.IsCpuCopyValid():
        return tc.packets.Get(args.pktid)
    return None

def GetExpectDelay(tc, args = None):
    if tc.tracker.IsCpuCopyValid():
        return 1
    return None

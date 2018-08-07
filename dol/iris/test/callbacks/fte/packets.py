# /usr/bin/python3
import pdb
from infra.api.objects import PacketHeader
import infra.api.api as infra_api

def GetCpuPacketbyIflow(testcase, args = None):
    root = getattr(testcase.config, 'flow', None)
    src = getattr(testcase.config, 'src', None)
    if root is None:
        root = getattr(testcase.config.session.iconfig, 'flow', None)
        src = getattr(testcase.config.session.rconfig, 'src', None)

    if not src.tenant.IsHostPinned():
       return None

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

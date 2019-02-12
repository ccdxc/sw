#! /usr/bin/python3
import pdb
import iota.harness.api as api
import iota.test.iris.config.mplsudp.tunnel as tunnel

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    result = tunnel.GetTunnelManager().DeleteTunnels()
    if result != api.types.status.SUCCESS:
        api.Logger.error("Failed to delete tunnels")
        return result

    result = tunnel.GetTunnelManager().CreateTunnels()
    if result != api.types.status.SUCCESS:
        api.Logger.error("Failed to create tunnels")
        return result
    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

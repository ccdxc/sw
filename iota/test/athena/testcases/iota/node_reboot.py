#! /usr/bin/python3
import time
import pdb

import iota.harness.api as api

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()

    return api.types.status.SUCCESS

def Trigger(tc):
    #api.Logger.info("Saved IotaAgent state...")
    #resp = api.SaveIotaAgentState(tc.nodes)
    #if resp != api.types.status.SUCCESS:
    #    api.Logger.error("Failed to save node")
    #    return resp
    
    api.Logger.info("Restart Nodes...")
    tc.resp = api.RestartNodes(tc.nodes)

    #api.Logger.info("Restore IotaAgent...")
    #tc.resp = api.RestoreIotaAgentState(tc.nodes)
    #if tc.resp != api.types.status.SUCCESS:
    #    api.Logger.error("Failed to restore IotaAgent after host reboot/reimage")
    #else:
    #    api.Logger.info("Restore IotaAgent successful post reboot/reimage")

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp != api.types.status.SUCCESS:
        api.Logger.error("Failed to restore naples-node after reload")
        return api.types.status.FAILURE

    api.Logger.info("Node restored after reboot")
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS


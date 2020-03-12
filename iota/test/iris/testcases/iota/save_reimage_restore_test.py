#! /usr/bin/python3
import time
import pdb

import iota.harness.api as api
import iota.test.iris.config.netagent.hw_push_config as hw_config

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()

    return api.types.status.SUCCESS

def Trigger(tc):
    resp = api.SaveIotaAgentState(tc.nodes)
    if resp != api.types.status.SUCCESS:
        api.Logger.error("Failed to save node")
        return resp

    api.Logger.info("Saved IotaAgent state...")
    pdb.set_trace()

    resp = api.ReInstallImage(fw_version=None, dr_version="latest")
    if resp != api.types.status.SUCCESS:
        api.Logger.error("Failed to reimage naples-node")
        return resp

    api.Logger.info("Restore IotaAgent...")
    tc.resp = api.RestoreIotaAgentState(tc.nodes)
    if tc.resp != api.types.status.SUCCESS:
        api.Logger.error("Failed to restore IotaAgent after host reboot/reimage")
    else:
        api.Logger.info("Restore IotaAgent successful post reboot/reimage")

    api.Logger.info("Attempting ReAddWorkloads on all naples nodes...")
    for node in tc.nodes:
        hw_config.ReAddWorkloads(node)

    return resp

def Verify(tc):
    if tc.resp != api.types.status.SUCCESS:
        api.Logger.error("Failed to restore naples-node after reload")
        return api.types.status.FAILURE

    api.Logger.info("Node restored after reboot")
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

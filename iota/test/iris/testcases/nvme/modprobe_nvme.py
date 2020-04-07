#! /usr/bin/python3
import iota.harness.api as api

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()

    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Logger.info("Loading nvme driver on nodes: {0}".format(tc.nodes))

    # Change it to only run on host node...
    for n in tc.nodes:
        api.Trigger_AddHostCommand(req, n, "modprobe nvme")
        # allow device to register before proceeding
        api.Trigger_AddHostCommand(req, n, "sleep 2")

    for n in tc.nodes:
        api.Trigger_AddHostCommand(req, n, "lsmod | grep nvme")

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    api.Logger.info("modprobe_nvme results for the following nodes: {0}".format(tc.nodes))

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

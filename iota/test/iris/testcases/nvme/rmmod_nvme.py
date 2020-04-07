#! /usr/bin/python3
import iota.harness.api as api

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])
    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Logger.info("Uninstalling nvme driver on the following nodes: {0}".format(tc.nodes))

    # Change it to only run on host node
    for n in tc.nodes:
        api.Trigger_AddHostCommand(req, n, "rmmod nvme")
        # allow device to register before proceeding
        api.Trigger_AddHostCommand(req, n, "sleep 2")

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    api.Logger.info("rmmod_nvme results")

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

#! /usr/bin/python3
import iota.harness.api as api
# To Shows the registry keys that were read by the driver

def Setup(tc):
    api.Logger.info("Ionic Config Register keys")
    tc.nodes = api.GetNaplesHostnames()
    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    for n in tc.nodes:
        api.Logger.info("Printing Register keys for node %s" % n)
        api.Trigger_AddHostCommand(req, n, "/mnt/c/Windows/Temp/drivers-windows/ionicConfig.exe RegKeys")

    tc.resp = api.Trigger(req)

    if tc.resp is None:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    status = []
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)

        if cmd.exit_code != 0:
            status.append(cmd.exit_code)
            api.Logger.error("Ionic Config is failed to get register keys")
            api.Logger.error(cmd.stderr)

    if status and all(status):
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

#! /usr/bin/python3
import iota.harness.api as api
# To Shows driver info

def Setup(tc):
    api.Logger.info("Ionic Config gets driver info")
    tc.nodes = api.GetNaplesHostnames()
    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    for n in tc.nodes:
        api.Logger.info("Get Info %s" % n)
        # Execute command(IonicConfig.exe info) to To Shows driver info
        api.Trigger_AddHostCommand(req, n, "/mnt/c/Windows/temp/drivers-windows/IonicConfig.exe info")

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
            api.Logger.error("Ionic Config is failed to get info")
            api.Logger.error(cmd.stderr)

    if status and all(status):
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

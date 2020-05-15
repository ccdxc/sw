#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.naples_host as naples_host
# To read interface counters from device

def Setup(tc):
    api.Logger.info("Ionic Config lif stats")
    tc.nodes = api.GetNaplesHostnames()
    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    for n in tc.nodes:
        for wl in api.GetWorkloads(n):
            interface_name = naples_host.GetWindowsIntName(n, wl.interface)
            api.Logger.info("Get lif stats for interface %s on node %s" % (interface_name, n))
            api.Trigger_AddHostCommand(req, n, "/mnt/c/Windows/temp/drivers-windows/IonicConfig.exe "
                                               "LifStats -n '%s'" % interface_name)

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
            api.Logger.error("Ionic Config is failed to get lif stats")
            api.Logger.error(cmd.stderr)

    if status and all(status):
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def Teardown(tc):
    return api.types.status.SUCCESS
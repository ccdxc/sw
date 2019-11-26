#! /usr/bin/python3
import iota.harness.api as api

def Main(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    cmd = "grep -m 1 fatal /obfl/asicerrord_err.log*"
    cmd1 = "grep -m 1 ECCERR /obfl/asicerrord_err.log*"
#    cmd2 = "grep -c -i error /obfl/asicerrord_err.log*"
#    cmd3 = "grep -c -i unknown /obfl/asicerrord_err.log*"

    resp = None
    for naples_host in api.GetNaplesHostnames():
        api.Trigger_AddNaplesCommand(req, naples_host, cmd)
        api.Trigger_AddNaplesCommand(req, naples_host, cmd1)
        resp = api.Trigger(req)

    if resp is None:
        return api.types.status.FAILURE

    for commands in resp.commands:
        if commands.exit_code == 0:
            api.PrintCommandResults(commands)
            api.Logger.error("ECCERR found Waiting to be debbuged - contact Rahul")
            time.sleep(72000)
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

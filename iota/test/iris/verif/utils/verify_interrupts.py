#! /usr/bin/python3
import time
import iota.harness.api as api
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

def Main(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "grep -m 1 fatal /obfl/asicerrord_err.log*"
    cmd1 = "grep -m 1 ECCERR /obfl/asicerrord_err.log*"
#    cmd2 = "grep -c -i error /obfl/asicerrord_err.log*"
#    cmd3 = "grep -c -i unknown /obfl/asicerrord_err.log*"

    resp = None
    for naples_host in api.GetNaplesHostnames():
        api.Trigger_AddNaplesCommand(req, naples_host, cmd)
        api.Trigger_AddNaplesCommand(req, naples_host, cmd1)
#        api.Trigger_AddNaplesCommand(req, naples_host, cmd2)
        resp = api.Trigger(req)

    if resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    for commands in resp.commands:
        api.PrintCommandResults(commands)
        if commands.exit_code == 0 and not api.Trigger_IsBackgroundCommand(commands):
            result =  api.types.status.FAILURE

    return result

#! /usr/bin/python3
import iota.harness.api as api
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

# The cleanup step cleans up the previous asicerrord files
# so that we can start with a clean setup.
def Main(tc):
    if GlobalOptions.skip_setup:
        return api.types.status.SUCCESS

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "rm /obfl/asicerrord_err.log.*"
    cmd1 = "truncate -s 0 /obfl/asicerrord_err.log"

    resp = None
    for naples_host in api.GetNaplesHostnames():
        api.Trigger_AddNaplesCommand(req, naples_host, cmd)
        api.Trigger_AddNaplesCommand(req, naples_host, cmd1)

    resp = api.Trigger(req)
    if resp is None:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

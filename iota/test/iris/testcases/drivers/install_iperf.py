import os
import iota.harness.api as api

source_file = api.GetTopDir() + '/iota/images/arm-iperf'

def Main(step):

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.ChangeDirectory("iperf")

    for naples_host in api.GetNaplesHostnames():
        api.CopyToNaples(naples_host, [source_file], "")
        api.Trigger_AddNaplesCommand(req, naples_host, "mv /arm-iperf /usr/bin/iperf")

    resp = api.Trigger(req)

    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE

    return api.types.status.SUCCESS


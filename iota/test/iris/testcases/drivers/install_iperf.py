import os
import iota.harness.api as api
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions
import iota.test.utils.naples_host as host

IPERF_BINARY = api.GetTopDir() + '/iota/images/iperf3_aarch64'
IONIC_STATS_SCRIPT = api.GetTopDir() + '/platform/drivers/freebsd/usr/src/ionic_stats.sh'

def Main(step):
    if GlobalOptions.skip_setup:
        return api.types.status.SUCCESS

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.ChangeDirectory("iperf")

    for naples_host in api.GetNaplesHostnames():
        if api.GetNodeOs(naples_host) == host.OS_TYPE_BSD: 
            api.CopyToHost(naples_host, [IONIC_STATS_SCRIPT], "")
            api.Trigger_AddHostCommand(req, naples_host, "cp  ionic_stats.sh " + api.HOST_NAPLES_DIR)
        api.CopyToNaples(naples_host, [IPERF_BINARY], "")
        api.Trigger_AddNaplesCommand(req, naples_host, "mv /iperf3_aarch64 /usr/bin/iperf3")

    resp = api.Trigger(req)

    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE

    return api.types.status.SUCCESS


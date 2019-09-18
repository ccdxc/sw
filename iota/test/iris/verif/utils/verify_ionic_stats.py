#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.utils.naples_host as host


def Main(tCase):
    for node in api.GetNaplesHostnames():
        for i in api.GetNaplesHostInterfaces(node):
            req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
            if api.GetNodeOs(node) == host.OS_TYPE_BSD:
                api.Trigger_AddHostCommand(
                    req, node, 'sysctl dev.%s | grep pkts | grep -v ": 0"' % host.GetNaplesSysctl(i))
                # Clear the stats.
                api.Trigger_AddHostCommand(
                    req, node, 'sysctl dev.%s.reset_stats=1' % host.GetNaplesSysctl(i))
            else:
                api.Trigger_AddHostCommand(
                    req, node, 'ethtool -S %s | grep packets | grep -v ": 0' % i)

            resp = api.Trigger(req)
            if resp is None:
                api.Logger.error(
                    "Failed to get stats for %s, is driver loaded?" % i)
                return api.types.status.FAILURE

            for cmd in resp.commands:
                if cmd.exit_code != 0:
                    if cmd.stdout is None:
			# Some tests can have driver loaded but no traffic
			# is generated
                        api.Logger.info("Stats output is empty")
                        api.PrintCommandResults(cmd)
                    else:
                        api.Logger.info("Stats output for %s: %s" %
                                        (i, cmd.stdout))
                        api.PrintCommandResults(cmd)

    return api.types.status.SUCCESS

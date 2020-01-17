#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.naples_host as host
import iota.test.iris.config.netagent.hw_push_config as hw_config

def Setup(tc):
    api.Logger.info("Validate stats reset feature")

    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    if tc.os != host.OS_TYPE_BSD:
        api.Logger.info("Not implemented")
        return api.types.status.IGNORED

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.os != host.OS_TYPE_BSD:
        api.Logger.info("Not implemented")
        return api.types.status.IGNORED

    for node in tc.nodes:
        req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
        for i in api.GetNaplesHostInterfaces(node):
            api.Trigger_AddHostCommand(req, node, 'sysctl dev.%s.reset_stats=1' % host.GetNaplesSysctl(i))
            api.Trigger_AddHostCommand(req, node, 'sysctl dev.%s | grep -v ": 0"' % host.GetNaplesSysctl(i))

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):

    for node in tc.nodes:
        # this is required to bring the testbed into operation state
        # after driver unload interfaces need to be initialized
        hw_config.ReAddWorkloads(node)

    if tc.resp is None:
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        if cmd.exit_code != 0:
            if tc.os == host.OS_TYPE_BSD:
                if cmd.stdout is not None:
                    api.Logger.error("Stats is not cleared")
                    api.PrintCommandResults(cmd)
                    return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS


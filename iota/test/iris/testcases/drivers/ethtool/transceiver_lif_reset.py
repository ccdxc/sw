#! /usr/bin/python3
import re
import time
import iota.harness.api as api
import iota.test.iris.utils.debug as debug_utils
import iota.test.iris.utils.host as host_utils
import iota.test.utils.naples_host as host
import iota.test.iris.config.netagent.hw_push_config as hw_config
# tc.desc = 'compare transceiver data before and after lif reset'

def Setup(tc):
    api.Logger.info("Driver/Device Info: Tranciever Data verify after LIF reset")
    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.os != host.OS_TYPE_BSD:
        api.Logger.info("test not supported yet for os %s" %tc.os)
        return api.types.status.SUCCESS

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_cookies = []

    for n in tc.nodes:
        intfs = api.GetNaplesHostInterfaces(n)
        for i in intfs:
            api.Logger.info("getting Media status info from %s" % i)
            api.Trigger_AddHostCommand(req, n, "sysctl dev.%s.media_status" % host.GetNaplesSysctl(i))
            tc.cmd_cookies.append('before')
            api.Trigger_AddHostCommand(req, n, "sysctl dev.%s.reset=1" % host.GetNaplesSysctl(i))
            api.Trigger_AddHostCommand(req, n, "sysctl dev.%s.media_status" % host.GetNaplesSysctl(i))
            tc.cmd_cookies.append('after')

    tc.resp = api.Trigger(req)

    if tc.resp == None:
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        if cmd.exit_code != 0:
            api.Logger.error("Failed to get media status info (check if cable is plugged in)")
            api.Logger.info(cmd.stderr)
            return api.types.status.FAILURE

    return api.types.status.SUCCESS


def Verify(tc):

    if tc.os != host.OS_TYPE_BSD:
        return api.types.status.SUCCESS

    for node in tc.nodes:
        # this is required to bring the testbed into operation state
        hw_config.ReAddWorkloads(node)

    if tc.resp is None:
        return api.types.status.FAILURE

    before = ""
    cookie_idx = 0
    testStatus = api.types.status.SUCCESS
    for cmd in tc.resp.commands:
        # cmds are per node in sequence; resp's are per node & intf in seq.
        # so, we can compare before and after data, per node & intf right in the loop
        api.PrintCommandResults(cmd)
        if (cmd.command.find("reset") != -1): # skip the reset cmd
            continue
        if (cmd.command.find("sleep") != -1): # skip the sleep cmd
            continue

        if (tc.cmd_cookies[cookie_idx] == 'before'):
            before = cmd.stdout
        if (tc.cmd_cookies[cookie_idx] == 'after'):
            cmdArr = cmd.stdout.split('.media_status:')
            if (before == cmd.stdout):
                api.Logger.info("Node %s transceiver data matches after LIF reset" % cmdArr[0])
            else:
                api.Logger.error("Node %s transceiver data does not match after LIF reset" % cmdArr[0])
                testStatus = api.types.status.FAILURE
        cookie_idx += 1

        # is EXIT code !0?
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    return testStatus

def Teardown(tc):
    return api.types.status.SUCCESS


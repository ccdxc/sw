#! /usr/bin/python3
import iota.harness.api as api
import iota.test.athena.utils.misc as misc_utils

# Check if athena_app running in no dpdk mode

def Setup(tc):
    api.Logger.info("Test if Athena app is in NO_DPDK mode")
    naples_nodes = api.GetNaplesHostnames()
    return api.types.status.SUCCESS

def Trigger(tc):
    naples_nodes = api.GetNaplesHostnames()
    for node in naples_nodes:
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddNaplesCommand(req, node, "ps -aef | grep athena_app | grep no-dpdk | grep -v 'grep'")
        resp = api.Trigger(req)
        cmd = resp.commands[0]
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("pgrep for athena_app failed on Node {}".format(node))
            return api.types.status.FAILURE
            
        if "athena_app" in cmd.stdout:
            athena_app_pid = cmd.stdout.strip().split()[1]
            api.Logger.info("athena_app up and running on Node {} with PID {}".format(node, athena_app_pid))
        else:
            api.Logger.error("athena_app is not running on Node {}")
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
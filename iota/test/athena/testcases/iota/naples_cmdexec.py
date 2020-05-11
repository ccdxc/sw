#!/usr/bin/python3

import iota.harness.api as api
import ipaddress
import json
import time

# Testing default/Custom policy.json
# Need to start athena_app first

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    naples_nodes = api.GetNaplesHostnames()
    for node in naples_nodes:
        api.Logger.info("Start second athena_app to pick up policy.json")
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddNaplesCommand(req, node, "/nic/tools/start-sec-agent.sh")
        api.Trigger_AddNaplesCommand(req, node, "\r\n")
        resp = api.Trigger(req)
        cmd = resp.commands[0]
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("Start second athena_app failed on node {}".format(node))
            return api.types.status.FAILURE
    
    time.sleep(10)

    for node in naples_nodes:
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddNaplesCommand(req, node, "ps -aef | grep athena_app")

        resp = api.Trigger(req)
        cmd = resp.commands[0]
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("ps failed on Node {}".format(node))
            return api.types.status.FAILURE
        if "athena_app" not in cmd.stdout:
            # TODO: If athena_app is not running, run start_agent.sh manually
            api.Logger.error("no athena_app running on Node {}, need to start athena_app first".format(node))
            return api.types.status.FAILURE
    
        athena_sec_app_pid = cmd.stdout.strip().split()[1]
        api.Logger.info("athena_app up and running on Node {} with PID {}".format(node, athena_sec_app_pid))

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS


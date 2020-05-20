#!/usr/bin/python3

import iota.harness.api as api
import iota.harness.infra.store as store
import iota.test.athena.utils.misc as misc_utils
from iota.harness.infra.glopts import GlobalOptions
import ipaddress
import json
import re
import time

# Testing default/Custom policy.json
# Need to start athena_app first

def Setup(tc):
    tc.nics =  store.GetTopology().GetNicsByPipeline("athena")
    tc.nodes= []
    for nic in tc.nics:
        tc.nodes.append(nic.GetNodeName())

    # if file is already there, it will overwrite the old file
    cmd = ""
    for node in tc.nodes:
        # because new logic in athena_app is to read policy.json in /data
        # if we want to test default policy.json, we have to clean /data first

        tc.policy_type = getattr(tc.args, 'type', "default")
        if tc.policy_type == "default":
            api.Logger.info("Test default policy.json")
            api.Logger.info("Clean old policy.json file in /data")
            cmd = "rm -f /data/policy.json"
        else:
            api.Logger.info("Test Custom policy.json")
            api.Logger.info("Copy policy.json file from /nic/conf/athena/ to /data/")
            cmd = "cp /nic/conf/athena/" + tc.policy_type + ".json  /data/policy.json"

        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddNaplesCommand(req, node, cmd)
        api.Trigger_AddNaplesCommand(req, node, "sync")
        resp = api.Trigger(req)
        cmd = resp.commands[0]
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("copy policy.json file to /data/ failed on node {}".format(node))
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Trigger(tc):
    for node in tc.nodes:
        api.Logger.info("Start second athena_app to pick up policy.json")
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        api.Trigger_AddNaplesCommand(req, node, "/nic/tools/start-sec-agent-iota.sh", background = True)
        resp = api.Trigger(req)
        cmd = resp.commands[0]
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("Start second athena_app failed on node {}".format(node))
            return api.types.status.FAILURE

    # default policy.json needs 80s and 1.5M custom policy.json needs 140s
    misc_utils.Sleep(140)

    for node in tc.nodes:
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddNaplesCommand(req, node, "ps -aef | grep athena_app | grep soft-init | grep -v 'grep'")

        resp = api.Trigger(req)
        cmd = resp.commands[0]
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("ps failed on Node {}".format(node))
            return api.types.status.FAILURE
        if "athena_app" not in cmd.stdout:
            api.Logger.error("no athena_app running on Node {}, need to start athena_app first".format(node))
            return api.types.status.FAILURE

        athena_sec_app_pid = cmd.stdout.strip().split()[1]
        api.Logger.info("athena_app up and running on Node {} with PID {}".format(node, athena_sec_app_pid))

        req = api.Trigger_CreateExecuteCommandsRequest()
        sig_cmd = "kill -SIGUSR1 " + athena_sec_app_pid
        api.Trigger_AddNaplesCommand(req, node, sig_cmd)
        resp = api.Trigger(req)
        cmd = resp.commands[0]
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("kill -SIGUSR1 failed on Node {}".format(node))
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def VerifyCustomPolicy(tc, flow_count, cfg_path):
    api.Logger.info("Verify Custom policy.json")
    with open(cfg_path, 'r') as policy_json_file:
        f = json.load(policy_json_file)
        count, NumSips, NumDips = 0, 0, 0
        for flows in f["v4_flows"]:
            sip_lo = flows["sip_lo"]
            sip_hi = flows["sip_hi"]
            start = int(ipaddress.IPv4Address(sip_lo))
            end = int(ipaddress.IPv4Address(sip_hi))
            if start > end: 
                api.Logger.error(f"ERROR: sip_lo is bigger than sip_hi, please check {CFG_PATH}")
                return api.types.status.FAILURE
            NumSips = end - start + 1

            dip_lo = flows["dip_lo"]
            dip_hi = flows["dip_hi"]
            start = int(ipaddress.IPv4Address(dip_lo))
            end = int(ipaddress.IPv4Address(dip_hi))
            if start > end: 
                api.Logger.error(f"ERROR: dip_lo is bigger than dip_hi, please check {CFG_PATH}")
                return api.types.status.FAILURE
            NumDips = end - start + 1

            count += NumSips * NumDips
    if int(flow_count) == count:
        api.Logger.info(f"Verify policy.json Successed, flow_count is {flow_count} ")
        return api.types.status.SUCCESS
    else:
        api.Logger.error(f"ERROR: flow count for policy.json is wrong, Calculated flow count: {count}, Actual flow count: {flow_count}")
        return api.types.status.FAILURE

def Verify(tc):
    misc_utils.Sleep(15) # time gap before further cmd, flow count need time to settle down

    CUSTOM_CFG_PATH = api.GetTopDir() + "/nic/conf" + "/athena" + "/" + tc.policy_type + ".json"
    DEFAULT_CFG_PATH = api.GetTopDir() + "/nic/conf" + "/athena" + "/" + "policy.json" + ".json"

    for node in tc.nodes:
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddNaplesCommand(req, node, "wc -l /data/flows_sec.log")
        resp = api.Trigger(req)
        cmd = resp.commands[0]
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("wc failed on Node {}".format(node))
            return api.types.status.FAILURE

        flow_count = cmd.stdout.strip().split()[0]
        if tc.policy_type == "default":
            ret = VerifyCustomPolicy(tc, flow_count, DEFAULT_CFG_PATH)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("flow count for default policy.json is wrong, flow_count is %s"% (flow_count))
                return api.types.status.FAILURE
        else:
            ret = VerifyCustomPolicy(tc, flow_count, CUSTOM_CFG_PATH)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("flow count for custom policy.json is wrong, please check detailed log")
                return api.types.status.FAILURE
    return api.types.status.SUCCESS

def StopAthenaApp(tc):
    for node in tc.nodes:
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddNaplesCommand(req, node, "ps -aef | grep athena_app | grep soft-init | grep -v 'grep'")

        resp = api.Trigger(req)
        cmd = resp.commands[0]
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("ps failed on Node {}".format(node))
            return api.types.status.FAILURE
        if "athena_app" not in cmd.stdout:
            # TODO: If athena_app is not running, run start_agent.sh manually
            api.Logger.error("no athena_app running on Node {}, no termination process needed".format(node))
            continue

        athena_sec_app_pid = cmd.stdout.strip().split()[1]
        api.Logger.info("athena_app up and running on Node {} with PID {}".format(node, athena_sec_app_pid))

        req = api.Trigger_CreateExecuteCommandsRequest()
        sig_cmd = "kill -SIGTERM " + athena_sec_app_pid
        api.Trigger_AddNaplesCommand(req, node, sig_cmd)
        resp = api.Trigger(req)
        cmd = resp.commands[0]
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("kill -SIGTERM failed on Node {}".format(node))
            return api.types.status.FAILURE
    misc_utils.Sleep(5)
    return api.types.status.SUCCESS

def Teardown(tc):
    StopAthenaApp(tc)
    for node in tc.nodes:
        req = api.Trigger_CreateExecuteCommandsRequest()
        if tc.policy_type == "default":
            api.Trigger_AddNaplesCommand(req, node, "> /data/flows_sec.log")
        else:
            api.Trigger_AddNaplesCommand(req, node, "rm -f /data/policy.json && > /data/flows_sec.log")
        resp = api.Trigger(req)
        cmd = resp.commands[0]
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("clean flow log failed on Node {}".format(node))
            return api.types.status.FAILURE
    return api.types.status.SUCCESS
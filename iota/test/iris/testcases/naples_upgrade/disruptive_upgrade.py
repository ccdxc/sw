#! /usr/bin/python3
import time
import json
import random
import pdb

import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.naples_upgrade.testupgapp_utils as testupgapp_utils
import iota.test.iris.testcases.naples_upgrade.upgrade_utils as upgrade_utils
import iota.test.utils.ping as ping
import iota.test.utils.arping as arping
import iota.test.iris.config.netagent.api as netagent_cfg_api

def verify_connectivity(tc):
    if arping.ArPing(tc) != api.types.status.SUCCESS:
        api.Logger.info("arping failed on setup")
    if ping.TestPing(tc, 'all', 'ipv4', 64) != api.types.status.SUCCESS:
        api.Logger.info("ping test failed on setup")
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Setup(tc):
    tc.pkt_size = 128
    tc.interval = 0.001 #1msec
    tc.count = int(tc.args.sleep / tc.interval)
    tc.Nodes = [random.choice(api.GetNaplesHostnames())]
    api.Logger.info(f"Running Naples upgrade test on {tc.Nodes}")

    if upgrade_utils.ResetUpgLog(tc.Nodes) != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    if verify_connectivity(tc) != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest()
    for node in tc.Nodes:
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/upgrade_halt_state_machine")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/pcieport_upgdata")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/pciemgr_upgdata")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/pciemgr_upgrollback")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/nicmgr_upgstate")
        api.Trigger_AddNaplesCommand(req, node, "touch /data/upgrade_to_same_firmware_allowed")
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Setup failed %s", cmd_resp.command)
            return api.types.status.FAILURE

    # Start upgrade test app
    if not tc.iterators.option:
        return api.types.status.SUCCESS
    for n in tc.Nodes:
        testupgapp_utils.startTestUpgApp(n, tc.iterators.option)

    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("starting ping in background ..")
    ping.TestPing(tc, "all", 'ipv4', tc.pkt_size, interval=tc.interval, count=tc.count, background=True, hping3=True)

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        cmd = 'curl -k -d \'{"kind": "SmartNICRollout","meta": {"name": "test disruptive upgrade","tenant": "tenant-foo"},"spec": {"ops": [{"op": 4,"version": "0.1"}]}}\' -X POST -H "Content-Type:application/json" ' + 'https://' + api.GetNicIntMgmtIP(n) + ':8888/api/v1/naples/rollout/'
        api.Trigger_AddHostCommand(req, n, cmd)
    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    api.Logger.info(f"Sleeping for {tc.args.sleep} ")
    time.sleep(tc.args.sleep)

    if tc.resp is None:
        api.Logger.error("No response for Naples Upgrade POST request")
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        cmd = 'curl -k https://' + api.GetNicIntMgmtIP(n) + ':8888/api/v1/naples/rollout/'
        api.Trigger_AddHostCommand(req, n, cmd)
    tc.resp = api.Trigger(req)
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.info("cmd returned failure")
            return api.types.status.FAILURE
        resp = json.loads(cmd.stdout)
        try:
            for item in resp['Status']['status']:
                if not item['Op'] == 4:
                    api.Logger.info("opcode is bad")
                    return api.types.status.FAILURE
                if "fail" in tc.iterators.option:
                    if not item['opstatus'] == 'failure':
                        api.Logger.info("opstatus is bad")
                        return api.types.status.FAILURE
                    if tc.iterators.option not in item['Message']:
                        api.Logger.info("message is bad")
                        return api.types.status.FAILURE
                else:
                    if not item['opstatus'] == 'success':
                        api.Logger.info("opstatus is bad")
                        return api.types.status.FAILURE
        except:
            api.Logger.info("resp: ", json.dumps(resp, indent=1))

    ping.TestTerminateBackgroundPing(tc, tc.pkt_size)
    ping_loss_duration = ping.GetMaxPktLossDuration(tc, tc.interval)
    api.Logger.info(f"Traffic dropped for {ping_loss_duration} sec")

    if upgrade_utils.VerifyUpgLog(tc.Nodes, tc.GetLogsDir()):
        api.Logger.error("Failed to verify the upgrade logs")
        return api.types.status.FAILURE
    if verify_connectivity(tc) != api.types.status.SUCCESS:
        api.Logger.error("Post trigger connectivity test failed.")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    for n in tc.Nodes:
        if not tc.iterators.option:
            testupgapp_utils.stopTestUpgApp(n, False)
        else:
            testupgapp_utils.stopTestUpgApp(n, True)

    req = api.Trigger_CreateExecuteCommandsRequest()
    for node in tc.Nodes:
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/upgrade_halt_state_machine")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/pcieport_upgdata")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/pciemgr_upgdata")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/pciemgr_upgrollback")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/nicmgr_upgstate")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /data/upgrade_to_same_firmware_allowed")
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Setup failed %s", cmd_resp.command)

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        cmd = 'curl -k -X DELETE https://' + api.GetNicIntMgmtIP(n) + ':8888/api/v1/naples/rollout/'
        api.Trigger_AddHostCommand(req, n, cmd)
    tc.resp = api.Trigger(req)
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

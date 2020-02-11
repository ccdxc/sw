#! /usr/bin/python3
import time
import json
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.naples_upgrade.upgradedefs as upgradedefs
import iota.test.iris.testcases.naples_upgrade.common as common
import iota.test.iris.testcases.naples_upgrade.ping as ping
import iota.test.iris.testcases.naples_upgrade.arping as arping
import iota.test.iris.config.netagent.api as netagent_cfg_api

def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    if arping.ArPing(tc) != api.types.status.SUCCESS:
        api.Logger.info("arping failed on setup")
    if ping.TestPing(tc, 'local_only', 'ipv4', 64) != api.types.status.SUCCESS or ping.TestPing(tc, 'remote_only', 'ipv4', 64) != api.types.status.SUCCESS:
        api.Logger.info("ping test failed on setup")
        return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest()
    for node in tc.Nodes:
        api.Trigger_AddNaplesCommand(req, node, "cp /update/{} /update/{}.orig".format(common.UPGRADE_NAPLES_PKG_COMPAT_CHECK, common.UPGRADE_NAPLES_PKG_COMPAT_CHECK))
        api.Trigger_AddNaplesCommand(req, node, "cp /update/{} /update/{}.orig".format(common.UPGRADE_NAPLES_PKG, common.UPGRADE_NAPLES_PKG))
        api.Trigger_AddNaplesCommand(req, node, "cp /update/{} /update/{}".format(common.UPGRADE_NAPLES_PKG_COMPAT_CHECK, common.UPGRADE_NAPLES_PKG))
        api.Trigger_AddNaplesCommand(req, node, "touch /data/upgrade_to_same_firmware_allowed")
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Setup failed %s", cmd_resp.command)

    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        cmd = 'curl -d \'{"kind": "SmartNICRollout","meta": {"name": "test disruptive upgrade","tenant": "tenant-foo"},"spec": {"ops": [{"op": 4,"version": "0.1"}]}}\' -X POST -H "Content-Type:application/json" ' + api.GetNicIntMgmtIP(n) + ':8888/api/v1/naples/rollout/'
        api.Trigger_AddHostCommand(req, n, cmd)
    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    time.sleep(tc.args.sleep)

    if tc.resp is None:
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        cmd = 'curl ' + api.GetNicIntMgmtIP(n) + ':8888/api/v1/naples/rollout/'
        api.Trigger_AddHostCommand(req, n, cmd)
    tc.resp = api.Trigger(req)
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)

    if netagent_cfg_api.switch_profile(push_base_profile=True) != \
       api.types.status.SUCCESS:
        api.Logger.info("Failed to push the base profile")
        return api.types.status.FAILURE

    if netagent_cfg_api.PushBaseConfig(ignore_error = False) != \
       api.types.status.SUCCESS:
        api.Logger.info("policy push failed")
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        if cmd.exit_code != 0:
            return api.types.status.FAILURE
        if arping.ArPing(tc) != api.types.status.SUCCESS:
            api.Logger.info("arping failed on verify")
        if ping.TestPing(tc, 'local_only', 'ipv4', 64) != api.types.status.SUCCESS or ping.TestPing(tc, 'remote_only', 'ipv4', 64) != api.types.status.SUCCESS:
            api.Logger.info("ping test failed")
            return api.types.status.FAILURE
        resp = json.loads(cmd.stdout)
        try:
            for item in resp['Status']['status']:
                if not item['Op'] == 4:
                    api.Logger.info("opcode is bad")
                    return api.types.status.FAILURE
                if not item['opstatus'] == 'failure':
                    api.Logger.info("opstatus is bad")
                    return api.types.status.FAILURE
                if "Compat Check Failed. Metadata version mismatch: Component versions" not in item['Message']:
                    api.Logger.info("message is bad")
                    return api.types.status.FAILURE
            return api.types.status.SUCCESS
        except:
            api.Logger.info("No Status field returned")
            return api.types.status.SUCCESS

def Teardown(tc):
    req = api.Trigger_CreateExecuteCommandsRequest()
    for node in tc.Nodes:
        api.Trigger_AddNaplesCommand(req, node, "cp /update/{}.orig /update/{}".format(common.UPGRADE_NAPLES_PKG_COMPAT_CHECK, common.UPGRADE_NAPLES_PKG_COMPAT_CHECK))
        api.Trigger_AddNaplesCommand(req, node, "cp /update/{}.orig /update/{}".format(common.UPGRADE_NAPLES_PKG, common.UPGRADE_NAPLES_PKG))
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /data/delphi.dat-lock")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /data/delphi.dat")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/{}.orig".format(common.UPGRADE_NAPLES_PKG_COMPAT_CHECK))
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/{}.orig".format(common.UPGRADE_NAPLES_PKG))
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /data/upgrade_to_same_firmware_allowed")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /data/NaplesTechSupport-*")
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Setup failed %s", cmd_resp.command)
            return api.types.status.FAILURE
    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        cmd = 'curl -X DELETE ' + api.GetNicIntMgmtIP(n) + ':8888/api/v1/naples/rollout/'
        api.Trigger_AddHostCommand(req, n, cmd)
    tc.resp = api.Trigger(req)
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

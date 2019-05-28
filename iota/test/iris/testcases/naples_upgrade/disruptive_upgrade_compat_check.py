#! /usr/bin/python3
import time
import json
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.naples_upgrade.upgradedefs as upgradedefs
import iota.test.iris.testcases.naples_upgrade.common as common

def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()

    req = api.Trigger_CreateExecuteCommandsRequest()
    for node in tc.Nodes:
        api.Trigger_AddNaplesCommand(req, node, "cp /update/{} /update/{}.orig".format(common.UPGRADE_NAPLES_PKG_COMPAT_CHECK, common.UPGRADE_NAPLES_PKG_COMPAT_CHECK))
        api.Trigger_AddNaplesCommand(req, node, "cp /update/{} /update/{}.orig".format(common.UPGRADE_NAPLES_PKG, common.UPGRADE_NAPLES_PKG))
        api.Trigger_AddNaplesCommand(req, node, "cp /update/{} /update/{}".format(common.UPGRADE_NAPLES_PKG_COMPAT_CHECK, common.UPGRADE_NAPLES_PKG))
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Setup failed %s", cmd_resp.command)
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Trigger(tc):

    cmd = 'curl -d \'{"kind": "SmartNICRollout","meta": {"name": "test disruptive upgrade","tenant": "tenant-foo"},"spec": {"ops": [{"op": 4,"version": "0.1"}]}}\' -X POST -H "Content-Type:application/json" 169.254.0.1:8888/api/v1/naples/rollout/'

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        api.Trigger_AddHostCommand(req, n, cmd)
    tc.resp = api.Trigger(req)
    time.sleep(tc.args.sleep)

    return api.types.status.SUCCESS

def Verify(tc):

    if tc.resp is None:
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    cmd = 'curl 169.254.0.1:8888/api/v1/naples/rollout/'
    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        api.Trigger_AddHostCommand(req, n, cmd)
    tc.resp = api.Trigger(req)
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE
        resp = json.loads(cmd.stdout)
        try:
            for item in resp['Status']['status']:
                if not item['Op'] == 4:
                    print("opcode is bad")
                    return api.types.status.FAILURE
                if not item['opstatus'] == 'failure':
                    print("opstatus is bad")
                    return api.types.status.FAILURE
                if not item['Message'] == "Compat Check Failed":
                    print("message is bad")
                    return api.types.status.FAILURE
            return api.types.status.SUCCESS
        except:
            print("No Status field returned")
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
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Setup failed %s", cmd_resp.command)
            return api.types.status.FAILURE
    cmd = 'curl -X DELETE 169.254.0.1:8888/api/v1/naples/rollout/'
    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        api.Trigger_AddHostCommand(req, n, cmd)
    tc.resp = api.Trigger(req)
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

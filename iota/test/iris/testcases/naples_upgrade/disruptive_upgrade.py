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
        api.Logger.info("ping test failed")
        return api.types.status.FAILURE
    if not tc.iterators.option:
        return api.types.status.SUCCESS
    for n in tc.Nodes:
        common.startTestUpgApp(n, tc.iterators.option)
    return api.types.status.SUCCESS

def Trigger(tc):

    cmd = 'curl -d \'{"kind": "SmartNICRollout","meta": {"name": "test disruptive upgrade","tenant": "tenant-foo"},"spec": {"ops": [{"op": 4,"version": "0.1"}]}}\' -X POST -H "Content-Type:application/json" 169.254.0.1:8888/api/v1/naples/rollout/'

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
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

    cmd = 'curl 169.254.0.1:8888/api/v1/naples/rollout/'
    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        api.Trigger_AddHostCommand(req, n, cmd)
    tc.resp = api.Trigger(req)
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE
        netagent_cfg_api.PushBaseConfig(ignore_error = False)
        if arping.ArPing(tc) != api.types.status.SUCCESS:
            api.Logger.info("arping failed on verify")
        if ping.TestPing(tc, 'local_only', 'ipv4', 64) != api.types.status.SUCCESS or ping.TestPing(tc, 'remote_only', 'ipv4', 64) != api.types.status.SUCCESS:
            api.Logger.info("ping test failed")
            return api.types.status.FAILURE
        resp = json.loads(cmd.stdout)
        for item in resp['Status']['status']:
            if not item['Op'] == 4:
                print("opcode is bad")
                return api.types.status.FAILURE
            if "fail" in tc.iterators.option:
                if not item['opstatus'] == 'failure':
                    print("opstatus is bad")
                    return api.types.status.FAILURE
                if tc.iterators.option not in item['Message']:
                    print("message is bad")
                    return api.types.status.FAILURE
            else:
                if not item['opstatus'] == 'success':
                    print("opstatus is bad")
                    return api.types.status.FAILURE
        return api.types.status.SUCCESS

def Teardown(tc):
    for n in tc.Nodes:
        if not tc.iterators.option:
            common.stopTestUpgApp(n, False)
        else:
            common.stopTestUpgApp(n, True)
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

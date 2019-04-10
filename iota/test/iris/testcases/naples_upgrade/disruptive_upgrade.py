#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2
import iota.test.iris.testcases.naples_upgrade.upgradedefs as upgradedefs
import iota.test.iris.testcases.naples_upgrade.common as common


def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    for n in tc.Nodes:
        common.startTestUpgApp(n, tc.iterators.option)
    return api.types.status.SUCCESS

def Trigger(tc):

    cmd = 'curl -d \'{"kind": "SmartNICRollout","meta": {"name": "test disruptive upgrade","tenant": "tenant-foo"},"spec": {"ops": [{"op": 1,"version": "0.1"},{"op": 3,"version": "0.1"}]}}\' -X POST -H "Content-Type:application/json" 169.254.0.1:8888/api/v1/naples/rollout/'

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

    return api.types.status.SUCCESS

def Teardown(tc):
    for n in tc.Nodes:
        common.stopTestUpgApp(n)
    return api.types.status.SUCCESS

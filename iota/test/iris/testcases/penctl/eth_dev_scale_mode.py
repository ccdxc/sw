#! /usr/bin/python3
import pdb
import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common

def Main(step):
    nodes = api.GetNaplesHostnames()

    assert(len(nodes) != 0)

    api.Logger.info("Update mode to eth-dev-scale")
    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in nodes:
        common.AddPenctlCommand(req, n, "update mode -f eth-dev-scale")
    api.Trigger(req)

    return api.types.status.SUCCESS

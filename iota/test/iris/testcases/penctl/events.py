#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2
import iota.test.iris.testcases.penctl.penctldefs as penctldefs
import iota.test.iris.testcases.penctl.common as common


def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    return api.types.status.SUCCESS

def Trigger(tc):
    tc.cmd_cookies = []
    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        common.AddPenctlCommand(req, n, "show events | tail -n 20")
        tc.cmd_cookies.append("No events found")
        #TODO: Trigger event
        #common.AddPenctlCommand(req, n, "show events | tail -n 20")

    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
	#TODO: Enable this code
        #if cmd.stdout != tc.cmd_cookies[cookie_idx]:
        #    return api.types.status.FAILURE
        if cmd.exit_code != 0:
            return api.types.status.FAILURE
        cookie_idx += 1
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

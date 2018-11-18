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
        #common.AddPenctlCommand(req, n, "execute rm -rf /data/core/core*")
        common.AddPenctlCommand(req, n, "list cores")
        #TODO verify "No core files found"
        #common.AddPenctlCommand(req, n, "execute kill -6 nmd")
        #TODO we can't bring up NMD from here
        #common.AddPenctlCommand(req, n, "execute /nic/bin/nmd -hostif lo")
        common.AddPenctlCommand(req, n, "list cores")
        #TODO verify nmd core file is present and delete it
        #common.AddPenctlCommand(req, n, "delete core -f <name>")


    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE
        cookie_idx += 1
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

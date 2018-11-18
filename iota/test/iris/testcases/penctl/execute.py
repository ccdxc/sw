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
	#TODO delphictl is crashing for some reason
        #common.AddPenctlCommand(req, n, "execute delphictl -shm -metrics AccelSeqQueueMetrics")
        common.AddPenctlCommand(req, n, "execute halctl -h")
        common.AddPenctlCommand(req, n, "execute ifconfig -a")
	#TODO enable this cli after adding code to ignore return 1
        #common.AddPenctlCommand(req, n, "execute")
        common.AddPenctlCommand(req, n, "execute top -b -n 1")
        common.AddPenctlCommand(req, n, "execute ls -al /nic/bin/ /nic/tools/")
        #TODO verify/etc

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

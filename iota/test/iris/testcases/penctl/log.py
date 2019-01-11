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

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        common.AddPenctlCommand(req, n, "show logs -m %s  2>&1 | tail -n 2" % (tc.iterators.option))

    tc.resp = api.Trigger(req)

    for n in tc.Nodes:
        resp = api.CopyFromHost(n, [penctldefs.PENCTL_DEST_DIR + "/%s.log" %(tc.iterators.option)], "%s/%s_%s.log" % (tc.GetLogsDir(), n, tc.iterators.option))


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
    return api.types.status.SUCCESS

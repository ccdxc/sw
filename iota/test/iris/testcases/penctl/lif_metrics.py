#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.penctl.penctldefs as penctldefs
import iota.test.iris.testcases.penctl.common as common
import iota.test.iris.testcases.drivers.interface as interface


def Setup(tc):
    tc.nodes = api.GetWorkloadNodeHostnames()

    tc.node_intfs = {}
    for node in tc.nodes:
        #return api.types.status.SUCCESS if api.GetNodeOs(node) in ["esx"] else api.types.status.FAILURE
        tc.node_intfs[node] = interface.GetNodeInterface(node)

    ret = interface.ConfigureInterfaces(tc)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Error in setting up interfaces")
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Trigger(tc):

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.nodes:
        if api.IsNaplesNode(n):
            common.AddPenctlCommand(req, n, "show metrics lif > metrics_lif.out.before")

    tc.lif_metrics_old = api.Trigger(req)

    common.SendTraffic(tc)

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.nodes:
        if api.IsNaplesNode(n):
            common.AddPenctlCommand(req, n, "show metrics lif > metrics.lif.out.after")

    tc.lif_metrics_new = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.lif_metrics_new is None:
        return api.types.status.FAILURE

    for cmd in tc.lif_metrics_new.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

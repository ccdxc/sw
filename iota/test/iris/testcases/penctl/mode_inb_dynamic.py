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
    if len(tc.Nodes) > 0:
        n = tc.Nodes[0]
        common.PrepareDhcpConfig("10.20.30.40")
        common.SetupRemoteDhcp(n)
        common.SetNaplesModeInband_Dynamic(n)

    return api.types.status.SUCCESS

def Verify(tc):
    if len(tc.Nodes) > 0:
        n = tc.Nodes[0]
        api.Logger.info("Got controller : {}".format(common.AgentGetControllers(n)[0]))
        # For now use the hardcoded value of 10.20.30.40:9009, later have this one automated as well
        if common.AgentGetControllers(n)[0] == "10.20.30.40":
            return api.types.status.SUCCESS

    return api.types.status.FAILURE

def Teardown(tc):
    return api.types.status.SUCCESS

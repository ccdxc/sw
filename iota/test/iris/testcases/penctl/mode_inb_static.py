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
        common.SetupRemoteDhcp(n)
        common.SetNaplesModeInband_Static(n, "1.1.1.1,2.2.2.2", "3.3.3.3/24")

    return api.types.status.SUCCESS

def Verify(tc):
    if len(tc.Nodes) > 0:
        n = tc.Nodes[0]
        mgmt_ip = common.PenctlGetNaplesMgtmIp(n)
        controllers = common.PenctlGetControllers(n)

        api.Logger.info("{} MODE : {} NETWORKMODE : {} CONTROLLER : {}".format(n, common.PenctlGetMode(n), common.PenctlGetNetworkMode(n), common.PenctlGetControllers(n)))
        api.Logger.info("Got controller : {}".format(common.AgentGetControllers(n)[0]))
        if mgmt_ip == "3.3.3.3/24" and common.AgentGetControllers(n)[0] == "1.1.1.1:9009" and controllers[0] == "1.1.1.1":
            return api.types.status.SUCCESS

    return api.types.status.FAILURE

def Teardown(tc):
    return api.types.status.SUCCESS

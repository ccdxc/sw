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
    for n in tc.Nodes:
        common.SetupRemoteDhcp(n)
        common.SetNaplesModeOOB_Static(n, "4.4.4.4,5.5.5.5.", "6.6.6.6/24")

    return api.types.status.SUCCESS

def Verify(tc):
    for n in tc.Nodes:
        mgmt_ip = common.PenctlGetNaplesMgtmIp(n)
        controllers = common.PenctlGetControllers(n)

        api.Logger.info("{} MODE : {} NETWORKMODE : {} CONTROLLER : {}".format(n, common.PenctlGetMode(n), common.PenctlGetNetworkMode(n), common.PenctlGetControllers(n)))
        api.Logger.info("Got controller : {}".format(common.AgentGetControllers(n)[0]))
        if mgmt_ip == "6.6.6.6/24" and common.AgentGetControllers(n)[0] == "4.4.4.4" and controllers[0] == "4.4.4.4":
            return api.types.status.SUCCESS

    return api.types.status.FAILURE

def Teardown(tc):
    return api.types.status.SUCCESS

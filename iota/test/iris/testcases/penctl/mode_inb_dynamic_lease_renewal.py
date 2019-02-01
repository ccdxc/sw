#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2
import iota.test.iris.testcases.penctl.penctldefs as penctldefs
import iota.test.iris.testcases.penctl.common as common

def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    tc.venice_ips = ["1.1.1.1", "2.2.2.2"]
    tc.controller_ip_penctl = []
    return api.types.status.SUCCESS

def Trigger(tc):
    if len(tc.Nodes) > 0:
        n = tc.Nodes[0]
        common.SetNaplesModeInband_Dynamic(n)
        for i in range(0, len(tc.venice_ips)):
            common.PrepareDhcpConfig(tc.venice_ips[i])
            common.SetupRemoteDhcp(n)
            api.Logger.info("Waiting for lease renewal duration.")
            time.sleep(45)
            tc.controller_ip_penctl.append(common.PenctlGetControllersStatus(n)[0])

    return api.types.status.SUCCESS

def Verify(tc):
    for i in range(0, len(tc.venice_ips)):
        api.Logger.info("PENCTL_CTRL : {} EXPECTED : {}".format(tc.controller_ip_penctl[i], tc.venice_ips[i]))
        if tc.controller_ip_penctl[i] != tc.venice_ips[i]:
             api.Logger.info("FAILED!")
             return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

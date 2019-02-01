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

        for i in range(0, len(tc.venice_ips)):
            common.PrepareDhcpConfig(tc.venice_ips[i])
            common.SetupRemoteDhcp(n)
            time.sleep(5)
            common.SetNaplesModeInband_Dynamic(n)
            api.Logger.info("DHCP Server updated. Waiting 5 seconds for DHCP handshake between Naples and DHCP Server.")
            time.sleep(10)
            tc.controller_ip_penctl.append(common.PenctlGetControllersStatus(n)[0])

    return api.types.status.SUCCESS

def Verify(tc):
    for i in range(0, len(tc.venice_ips)):
        if tc.controller_ip_penctl[i] != tc.venice_ips[i]:
             api.Logger.info("FAILED! GOT : {} EXPECTED : {}".format(tc.controller_ip_penctl[i], tc.venice_ips[i]))
             return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    if len(tc.Nodes) > 0:
        n = tc.Nodes[0]
        common.ResetNMDState(n)
        common.RebootHost(n)

    return api.types.status.SUCCESS

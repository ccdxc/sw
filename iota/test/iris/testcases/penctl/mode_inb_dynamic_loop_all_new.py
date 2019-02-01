#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2
import iota.test.iris.testcases.penctl.penctldefs as penctldefs
import iota.test.iris.testcases.penctl.common as common

def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    tc.venice_ips = [["1.1.1.1", "2.2.2.2", "3.3.3.3"], ["4.4.4.4", "5.5.5.5", "6.6.6.6"]]
    #tc.venice_ips = ["1.1.1.1", "2.2.2.2"]
    tc.controller_ip_penctl = []
    return api.types.status.SUCCESS

def Trigger(tc):
    if len(tc.Nodes) > 0:
        n = tc.Nodes[0]
        common.PrepareDhcpConfig(tc.venice_ips[0])
        common.SetupRemoteDhcp(n)
        common.SetNaplesModeInband_Dynamic(n)
        api.Logger.info("DHCP Server updated. Waiting 5 seconds for DHCP handshake between Naples and DHCP Server.")
        time.sleep(5)
        common.PrepareDhcpConfig(tc.venice_ips[1])
        common.SetupRemoteDhcp(n)
        common.RebootHost(n)
        tc.controller_ip_penctl.append(common.PenctlGetControllersStatus(n)[0])

    return api.types.status.SUCCESS

def Verify(tc):
    api.Logger.info("Verification pending.")
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

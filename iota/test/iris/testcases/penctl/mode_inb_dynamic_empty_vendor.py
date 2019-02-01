#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2
import iota.test.iris.testcases.penctl.penctldefs as penctldefs
import iota.test.iris.testcases.penctl.common as common


def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    tc.naples_status_after = []
    tc.controller_ip_penctl_after = []

    return api.types.status.SUCCESS

def Trigger(tc):
    if len(tc.Nodes) > 0:
        n = tc.Nodes[0]
        common.PrepareDhcpConfigNoControllers()
        common.SetupRemoteDhcp(n)
        time.sleep(10)
        common.SetNaplesModeInband_Dynamic(n)
        tc.naples_status_after.append(common.PenctlGetTransitionPhaseStatus(n))
        tc.controller_ip_penctl_after.append(common.PenctlGetControllersStatus(n))

    return api.types.status.SUCCESS

def Verify(tc):
    if len(tc.Nodes) > 0:
       n = tc.Nodes[0]
       if tc.naples_status_after[0] != "MISSING_VENDOR_SPECIFIED_ATTRIBUTES":
           api.Logger.info("Got Naples Status as {} instead of MISSING_VENDOR_SPECIFIED_ATTRIBUTES  and got controllers as {}".format(tc.naples_status_after[0], tc.controller_ip_penctl_after[0]))
           return api.types.status.FAILURE 

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

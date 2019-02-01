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
    for n in tc.Nodes:
        common.PrepareDhcpConfigNoControllers()
        common.SetupRemoteDhcp(n)
        time.sleep(10)
        common.SetNaplesModeOOB_Dynamic(n)
        tc.naples_status_after.append(common.PenctlGetTransitionPhaseStatus(n))
        tc.controller_ip_penctl_after.append(common.PenctlGetControllersStatus(n))

    return api.types.status.SUCCESS

def Verify(tc):
    i = 0
    for n in tc.Nodes:
       if tc.naples_status_after[i] != "MISSING_VENDOR_SPECIFIED_ATTRIBUTES":
           api.Logger.info("Got Naples Status as {} instead of MISSING_VENDOR_SPECIFIED_ATTRIBUTES  and got controllers as {}".format(tc.naples_status_after[i], tc.controller_ip_penctl_after[i]))
           return api.types.status.FAILURE 
       i = i + 1

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

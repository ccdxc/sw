#! /usr/bin/python3
import time
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

	common.PenctlGetTransitionPhaseStatus(n)
        common.SetNaplesModeInband_Static(n, "1.1.1.1","2.2.2.2/24")
        tc.before_reboot_status = common.PenctlGetTransitionPhaseStatus(n)
        #common.PenctlGetControllersStatus(n)[0]
           
        common.RebootHost(n)

        tc.after_reboot_status = common.PenctlGetTransitionPhaseStatus(n)
        #tc.controller_ip_penctl_after.append(common.PenctlGetControllersStatus(n)[0])

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.before_reboot_status != "REBOOT_PENDING" \
       and tc.after_reboot_status != "VENICE_REGISTRATION_DONE":
           api.Logger.info("Test Failed. STATUS - BEFORE {} AFTER {}".format(tc.before_reboot_status, tc.after_reboot_status))
           return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    if len(tc.Nodes) > 0:
        n = tc.Nodes[0]
        common.ResetNMDState(n)
        common.RebootHost(n)
    return api.types.status.SUCCESS

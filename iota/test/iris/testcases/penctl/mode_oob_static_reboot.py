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
        common.SetNaplesModeOOB_Static(n, "4.4.4.4","5.5.4.4/24")
        tc.before_reboot_status = common.PenctlGetTransitionPhaseStatus(n)
        #common.PenctlGetControllersStatus(n)[0]
           
        api.Logger.info("Going for first reboot")
        common.RebootHost(n)
        tc.after_first_reboot_status = common.PenctlGetTransitionPhaseStatus(n)
        api.Logger.info("The Status after first reboot is {}".format(tc.after_first_reboot_status))
        #tc.controller_ip_penctl_after.append(common.PenctlGetControllersStatus(n)[0])

        api.Logger.info("Going for second reboot")
        common.RebootHost(n)
        tc.after_second_reboot_status = common.PenctlGetTransitionPhaseStatus(n)
        api.Logger.info("The Status after second reboot is {}".format(tc.after_second_reboot_status))

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.before_reboot_status != "REBOOT_PENDING" \
       or tc.after_first_reboot_status != "VENICE_UNREACHABLE" \
       or tc.after_second_reboot_status != "VENICE_UNREACHABLE":
          api.Logger.info("Test Failed. STATUS - BEFORE {} AFTER FIRST REBOOT {} AFTER SECOND REBOOT {}".format(tc.before_reboot_status, \
                         tc.after_first_reboot_status,\
                         tc.after_second_reboot_status))
          return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    if len(tc.Nodes) > 0:
        n = tc.Nodes[0]
        common.ResetNMDState(n)
        common.RebootHost(n)
    return api.types.status.SUCCESS

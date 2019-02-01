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
    for n in tc.Nodes:
#        common.PrepareDhcpConfig("5.5.5.5")
#        common.SetupRemoteDhcp(n)
#        common.SetNaplesModeInband_Dynamic(n)
#        time.sleep(5)
         api.Logger.info("Rebooting Host {}".format(n))
         common.ResetNMDState(n)
#         common.RebootHost(n)
#        print("RESTART NMD : {}".format(common.RestartNMD(n)))
#        print("CONTROLLERS : {}".format(common.PenctlGetControllersStatus(n)))
#        print("TRANSITION PHASE : {}".format(common.PenctlGetTransitionPhaseStatus(n)))
#         print("{}".format(common.NodeInit(n)))
         print("ADMISSION PHASE : {}".format(common.PenctlGetAdmissionPhaseStatus(n)))

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

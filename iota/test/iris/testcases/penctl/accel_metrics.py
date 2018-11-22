#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2
import iota.test.iris.testcases.penctl.penctldefs as penctldefs
import iota.test.iris.testcases.penctl.common as common
import iota.test.iris.testcases.drivers.interface as interface
import iota.test.iris.testcases.storage.ymltest2 as ymltest2

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.nodes:
        common.AddPenctlCommand(req, n, "show metrics accelhwring")
        common.AddPenctlCommand(req, n, "show metrics accelseqqueue")

    tc.storage_metrics_old = api.Trigger(req)


    return api.types.status.SUCCESS

def Trigger(tc):

    #Run Traffic test
    ret = ymltest2.RunTest(tc)

    if ret != api.types.status.SUCCESS:
        api.Logger.error("Running storage tests failed")
        return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.nodes:
        common.AddPenctlCommand(req, n, "show metrics accelhwring")
        common.AddPenctlCommand(req, n, "show metrics accelseqqueue")

    tc.storage_metrics_new = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    #Verify metrics here
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

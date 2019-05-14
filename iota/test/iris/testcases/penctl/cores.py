#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.penctl.penctldefs as penctldefs
import iota.test.iris.testcases.penctl.common as common

def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    for n in tc.Nodes:
        ret = common.CleanupCores(n)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Setup failed")
            return ret
    return api.types.status.SUCCESS

def Trigger(tc):
    tc.cmd_cookies = []
    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        ret = common.CreateNaplesCores(n)
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE

    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    for n in tc.Nodes:
        cores = common.GetCores(n)
        if len(cores) != len(common.core_file_names) or set(cores) != set(common.core_file_names):
            api.Logger.error("Cores don't match : expected %s, actual %s" %(common.core_file_names, cores))
            return api.types.status.FAILURE
        api.Logger.info("Cores matched")

    return api.types.status.SUCCESS

def Teardown(tc):
    for n in tc.Nodes:
        common.CleanupCores(n)
    return api.types.status.SUCCESS

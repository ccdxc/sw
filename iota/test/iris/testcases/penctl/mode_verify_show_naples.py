#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.penctl.common as common

def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    return api.types.status.SUCCESS

def Trigger(tc):
    for n in tc.Nodes:
        api.Logger.info("/tmp/fru.json contents : {}".format(common.GetNaplesFruJson(n)))
        api.Logger.info("penctl show naples fru contens : {}".format(common.GetPenctlFruJson(n)))


    return api.types.status.SUCCESS

def Verify(tc):
    if len(tc.Nodes) > 0:
        for n in tc.Nodes:
            if common.CheckFruInfo(n) != api.types.status.SUCCESS and common.IsNaplesHostManagedDefault(n) != api.types.status.SUCCESS:
                return api.types.status.FAILURE
    else:
        api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

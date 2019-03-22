#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as netagent_cfg_api


def Setup(tc):
    return api.types.status.SUCCESS


def Trigger(tc):
    ret = netagent_cfg_api.DeleteBaseConfig(ignore_error = False)
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    ret = netagent_cfg_api.PushBaseConfig(ignore_error = False)
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    #Readd Naples workloads
    for naples in api.GetNaplesHostnames():
        ret = api.ReAddWorkloads(api.GetWorkloads(naples))
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    result = api.types.status.SUCCESS
    return result


def Teardown(tc):
    return api.types.status.SUCCESS

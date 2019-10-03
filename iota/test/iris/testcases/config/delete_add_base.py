#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as netagent_cfg_api


def Setup(tc):
    tc.hal_log = getattr(tc.args, "hal_log", True)
    if tc.hal_log:
        agent_nodes = api.GetNaplesHostnames()
        #For now this is run for scale only, disable hal debug logs
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        for node in agent_nodes:
            api.Trigger_AddNaplesCommand(req, node, "/nic/bin/halctl debug trace --level debug")

        api.Trigger(req)

    return api.types.status.SUCCESS


def Trigger(tc):
    count = getattr(tc.args, "count", 1)
    kinds = getattr(tc.args, "kinds", None)

    wait = getattr(tc.args, "wait", 30)
    time.sleep(int(wait))

    for i in range(0, int(count)):
        ret = netagent_cfg_api.DeleteBaseConfig(ignore_error = False, kinds=kinds)
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE

        ret = netagent_cfg_api.PushBaseConfig(ignore_error = False, kinds=kinds)
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE

        if not kinds or "Endpoint" in kinds:
            #Readd Naples workloads
            for naples in api.GetNaplesHostnames():
                ret = api.ReSetupWorkoads(api.GetWorkloads(naples))
                if ret != api.types.status.SUCCESS:
                    return api.types.status.FAILURE

        time.sleep(60)

    return api.types.status.SUCCESS

def Verify(tc):
    result = api.types.status.SUCCESS
    return result


def Teardown(tc):
    if tc.hal_log:
        agent_nodes = api.GetNaplesHostnames()
        #For now this is run for scale only, disable hal debug logs
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        for node in agent_nodes:
            api.Trigger_AddNaplesCommand(req, node, "/nic/bin/halctl debug trace --level error")

        api.Trigger(req)
    return api.types.status.SUCCESS

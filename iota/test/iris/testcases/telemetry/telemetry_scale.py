#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.telemetry.utils as utils
import copy
from ipaddress import ip_address

def Setup(tc):
    return api.types.status.SUCCESS

def UpdateFlowMonitorObjects(fpObjs):
    ip_start = ip_address("3.1.1.1")
    export = copy.deepcopy(fpObjs[0].spec.exports[0])
    exports = []
    updateObjects = []
    # update the exports in mirror objects
    for obj in reversed(fpObjs):
        ip_start += 1
        export.destination = str(ip_start)
        exports.append(copy.deepcopy(export))
        obj.spec.exports = copy.deepcopy(exports)
        updateObjects.append(obj)
        if len(exports) == 4:
            exports = []
    return updateObjects

def UpdateMirrorSessionObjects(msObjs):
    ip_start = ip_address("2.1.1.1")
    collector = copy.deepcopy(msObjs[0].spec.collectors[0])
    collectors = []
    updateObjects = []
    for obj in reversed(msObjs):
        ip_start += 1
        collector.export_config.destination = str(ip_start)
        collectors.append(copy.deepcopy(collector))
        obj.spec.collectors = copy.deepcopy(collectors)
        updateObjects.append(obj)
    return updateObjects


def Trigger(tc):
    result = api.types.status.SUCCESS
    mirrorPolicies = utils.GetTargetJsons('mirror', "scale")
    flowmonPolicies = utils.GetTargetJsons('flowmon', "scale")
    iters = getattr(tc.args, "iters", 10)

    for mp_json, fp_json in zip(mirrorPolicies, flowmonPolicies):
        for i in range(iters):
            #
            # Push Mirror Session and Flow Export objects
            #
            mpObjs = agent_api.AddOneConfig(mp_json)
            fpObjs = agent_api.AddOneConfig(fp_json)
            ret = agent_api.PushConfigObjects(mpObjs+fpObjs)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Failed to push the telemetry objects")
                return api.types.status.FAILURE
            #
            # Update Mirror Session and Flow Export objects
            #
            mpObjs = UpdateMirrorSessionObjects(mpObjs)
            fpObjs = UpdateFlowMonitorObjects(fpObjs)
            ret = agent_api.UpdateConfigObjects(mpObjs+fpObjs)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Failed to update the telemetry objects")
                return api.types.status.FAILURE
            #
            # Delete Mirror Session and Flow Export objects
            #
            ret = agent_api.DeleteConfigObjects(fpObjs+mpObjs)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Failed to delete the telemetry objects")
                return api.types.status.FAILURE
            ret = agent_api.RemoveConfigObjects(mpObjs+fpObjs)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Failed to remove the telemetry objects")
                return api.types.status.FAILURE

    return result

def Verify(tc):
    showMirrorCmd = "/nic/bin/halctl show mirror"
    showFlowMonitorCmd = "/nic/bin/halctl show flow-monitor"
    showCollectorCmd = "/nic/bin/halctl show collector"
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    for node_name in api.GetNaplesHostnames():
        api.Trigger_AddNaplesCommand(req, node_name, showMirrorCmd)
        api.Trigger_AddNaplesCommand(req, node_name, showFlowMonitorCmd)
        api.Trigger_AddNaplesCommand(req, node_name, showCollectorCmd)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)

    if not api.Trigger_IsSuccess(resp):
        api.Logger.error("Failed to  execute HAL command to dump mirror and flow monitor.")
        return api.types.status.FAILURE

    for cmd in resp.commands:
        if len(cmd.stdout):
            api.Logger.error("Commad: %s validation failed. Expecting empty"%cmd.command)
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

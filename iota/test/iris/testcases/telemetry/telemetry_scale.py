#! /usr/bin/python3
import copy
import random
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.telemetry.utils as utils
import iota.test.iris.testcases.expanded_telemetry.utils as eutils
from ipaddress import ip_address

def Setup(tc):
    tc.node_intf_obj_map = {}
    for node_name in api.GetNaplesHostnames():
        uplink_objs = eutils.generateUplinkIntfCfgObj(node_name)
        tc.node_intf_obj_map[node_name] = uplink_objs
        api.Logger.info("%s Interface objects:"%(node_name))
        agent_api.PrintConfigObjects(uplink_objs)
    return api.types.status.SUCCESS

def UpdateFlowMonitorObjects(fpObjs):
    ip_start = ip_address("3.1.1.1")
    export = copy.deepcopy(fpObjs[0].spec.exports[0])
    exports = []
    updateObjects = []
    # update the exports in mirror objects
    for obj in reversed(fpObjs):
        export.destination = str(ip_start)
        exports.append(copy.deepcopy(export))
        obj.spec.exports = copy.deepcopy(exports)
        updateObjects.append(obj)
        if len(exports) == 4:
            exports = []
        ip_start += 1
    return updateObjects

def UpdateMirrorSessionObjects(msObjs):
    ip_start = ip_address("2.1.1.1")
    collector = copy.deepcopy(msObjs[0].spec.collectors[0])
    collectors = []
    updateObjects = []
    for obj in reversed(msObjs):
        collector.export_config.destination = str(ip_start)
        collectors.append(copy.deepcopy(collector))
        obj.spec.collectors = copy.deepcopy(collectors)
        updateObjects.append(obj)
        ip_start += 1
    return updateObjects

def UpdateCollectorObjects(coObjs):
    ip_start = ip_address("2.1.1.1")
    for obj in coObjs:
        obj.spec.destination = str(ip_start)
        ip_start += 1
    return coObjs

def AddRemoveCollectorsOnInterface(tc, coObjs):
    node_name = random.choice(api.GetNaplesHostnames())
    api.Logger.info("Pushing lif telemetry on %s"%node_name)
    for obj in tc.node_intf_obj_map[node_name]:
        del obj.spec.TxCollectors[:]
        del obj.spec.RxCollectors[:]
        for coObj in coObjs:
            obj.spec.RxCollectors.append(coObj.meta.name)
            obj.spec.TxCollectors.append(coObj.meta.name)
        ret = agent_api.UpdateConfigObjects(tc.node_intf_obj_map[node_name], [node_name])
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Failed to push the interface objects")
            return api.types.status.FAILURE

    for obj in tc.node_intf_obj_map[node_name]:
        del obj.spec.TxCollectors[:]
        del obj.spec.RxCollectors[:]
        ret = agent_api.UpdateConfigObjects(tc.node_intf_obj_map[node_name], [node_name])
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Failed to push the interface objects")
            return api.types.status.FAILURE
    return api.types.status.SUCCESS


def Trigger(tc):
    result = api.types.status.SUCCESS
    mirrorPolicies = utils.GetTargetJsons('mirror', "scale")
    flowmonPolicies = utils.GetTargetJsons('flowmon', "scale")
    colPolicies = utils.GetTargetJsons('mirror', "collector")
    iters = getattr(tc.args, "iters", 10)
    mpObjs = fpObjs = []
    for mp_json, fp_json, col_json in zip(mirrorPolicies, flowmonPolicies, colPolicies):
        for i in range(iters):
            #
            # Push Mirror Session and Flow Export objects
            #
            mpObjs = agent_api.AddOneConfig(mp_json)
            fpObjs = agent_api.AddOneConfig(fp_json)
            coObjs =  agent_api.AddOneConfig(col_json)
            ret = agent_api.PushConfigObjects(mpObjs+fpObjs+coObjs)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Failed to push the telemetry objects")
                return api.types.status.FAILURE

            #
            # Add and Remove the collecors to interface config
            #
            ret = AddRemoveCollectorsOnInterface(tc, coObjs)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Failed to update the interface objects")
                return api.types.status.FAILURE
            ret = agent_api.DeleteConfigObjects(coObjs)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Failed to delete the telemetry objects")
                return api.types.status.FAILURE

            #
            # Update Mirror Session and Flow Export objects
            #
            mpObjs = UpdateMirrorSessionObjects(mpObjs)
            fpObjs = UpdateFlowMonitorObjects(fpObjs)
            coObjs = UpdateCollectorObjects(coObjs)
            ret = agent_api.UpdateConfigObjects(mpObjs+fpObjs)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Failed to update the telemetry objects")
                return api.types.status.FAILURE
            ret = agent_api.PushConfigObjects(coObjs)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Failed to delete the telemetry objects")
                return api.types.status.FAILURE

            #
            # Add and Remove the collecors to interface config
            #
            ret = AddRemoveCollectorsOnInterface(tc, coObjs)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Failed to update the interface objects")
                return api.types.status.FAILURE

            #
            # Delete Mirror Session and Flow Export objects
            #
            ret = agent_api.DeleteConfigObjects(fpObjs+mpObjs+coObjs)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Failed to delete the telemetry objects")
                return api.types.status.FAILURE
            ret = agent_api.RemoveConfigObjects(mpObjs+fpObjs+coObjs)
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

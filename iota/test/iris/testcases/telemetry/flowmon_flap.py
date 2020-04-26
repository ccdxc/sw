#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.telemetry.utils as utils
import iota.harness.infra.utils.periodic_timer as timer
from collections import defaultdict

def GetCollectorWorkloadFromObjects(tc):
    for obj in tc.newObjects:
        for obj_export_cfg in obj.spec.exports:
            tc.collector_cfg.append(obj_export_cfg)
            api.Logger.info("export-dest: {} proto: {} port: {}".format(obj_export_cfg.destination,
                                                                        obj_export_cfg.proto_port.protocol,
                                                                        obj_export_cfg.proto_port.port))
    if len(tc.collector_cfg) == 0:
        return api.types.status.FAILURE

    for col in tc.collector_cfg:
        for wl in api.GetWorkloads():
            if (wl.ip_address == col.destination) or \
               (col.destination in tc.wl_sec_ip_info[wl.workload_name]):
                tc.collector_wl.append(wl)

    return api.types.status.SUCCESS

def Setup(tc):
    tc.skip_flap = None
    tc.newObjects = None
    tc.collector_cfg = []
    tc.collector_wl = []
    tc.wl_sec_ip_info = defaultdict(lambda: dict())
    tc.IsBareMetal = utils.IsBareMetal()
    tc.port_down_time = getattr(tc.args, "port_down_time", 60)
    policies = utils.GetTargetJsons('flowmon', tc.iterators.proto)
    policy_json = policies[0]
    tc.verif_json = utils.GetVerifJsonFromPolicyJson(policy_json)
    tc.newObjects = agent_api.AddOneConfig(policy_json)

    # Push flow export onjects
    ret = agent_api.PushConfigObjects(tc.newObjects)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Unable to push flowmon objects")
        tc.newObjects = None
        return api.types.status.FAILURE

    # Populate secondary IP
    utils.PopulateSecondaryAddress(tc)

    # Get collector
    ret =  GetCollectorWorkloadFromObjects(tc)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Failed to get collector workload")
        return ret

    ret = utils.DetectUpLinkState(api.GetNaplesHostnames(), utils.PORT_OPER_STATUS_UP, all)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("All uplink on Nodes are not in UP state.")
        tc.skip_flap = True
        return api.types.status.SUCCESS

    if api.GetNicMode() in ["classic", "unified"]:
        api.Logger.info(f"NIC mode: {api.GetNicMode()}, Skipping uplink flap")
        tc.skip_flap = True

    api.Logger.info("All uplink on Nodes are UP!")
    # Bring up inband and reset the active link on bond.
    ret = utils.SetupInbandInterface()
    if ret != api.types.status.SUCCESS:
        return ret

    return api.types.status.SUCCESS

def Trigger(tc):
    collector_info = utils.GetFlowmonCollectorsInfo(tc.collector_wl, tc.collector_cfg)
    ret = utils.RunAll(tc, tc.verif_json, 'flowmon', collector_info, tc.IsBareMetal)
    if ret['res'] != api.types.status.SUCCESS:
        return ret['res']

    if tc.skip_flap:
        api.Logger.info("Skipping switch port flap")
        return api.types.status.SUCCESS

    # Flap the uplink
    flapTask = utils.GetSwitchPortFlapTask(api.GetNaplesHostnames(), 1, tc.port_down_time)
    flapTask.start()

    # Make sure up link is down
    ret = utils.DetectUpLinkState(api.GetNaplesHostnames(), utils.PORT_OPER_STATUS_DOWN, any)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Failed to detect any uplink in DOWN state.")
        flapTask.join(tc.port_down_time)
        return ret

    # Rerun the tests
    ret = utils.RunAll(tc, tc.verif_json, 'flowmon', collector_info, tc.IsBareMetal)
    api.Logger.info("Waiting for switch flap thread to join..")
    flapTask.join(tc.port_down_time)
    return ret["res"]

def Verify(tc):
    result = api.types.status.SUCCESS
    return result

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    if tc.newObjects:
        agent_api.DeleteConfigObjects(tc.newObjects)
        agent_api.RemoveConfigObjects(tc.newObjects)

    return api.types.status.SUCCESS


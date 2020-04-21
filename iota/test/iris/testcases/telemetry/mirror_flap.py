#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.telemetry.utils as utils
import time

from collections import defaultdict

def GetCollectorWorkloadFromObjects(tc):
    collector_ip = None
    for obj in tc.newObjects:
        for col in obj.spec.collectors:
            collector_ip = col.export_config.destination
            api.Logger.info("export-dest: %s "%collector_ip)
            tc.collector_ip.append(collector_ip)
            tc.collector_type.append(col.type)

    if len(tc.collector_ip) == 0:
        return api.types.status.FAILURE

    for col in tc.collector_ip:
        for wl in api.GetWorkloads():
            if col == wl.ip_address or (col in tc.wl_sec_ip_info[wl.workload_name]):
                tc.collector_wl.append(wl)
    return api.types.status.SUCCESS

def Setup(tc):
    tc.skip_flap = False
    tc.newObjects = None
    tc.collector_ip = []
    tc.collector_wl = []
    tc.collector_type = []
    tc.wl_sec_ip_info = defaultdict(lambda: dict())
    tc.IsBareMetal = utils.IsBareMetal()
    tc.port_down_time = getattr(tc.args, "port_down_time", 60)
    policies = utils.GetTargetJsons('mirror', tc.iterators.proto)
    policy_json =  policies[0]
    tc.verif_json = utils.GetVerifJsonFromPolicyJson(policy_json)

    # Push Mirror objects
    tc.newObjects = agent_api.AddOneConfig(policy_json)
    ret = agent_api.PushConfigObjects(tc.newObjects)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Unable to push mirror objects")
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

    api.Logger.info("All uplink on Nodes are UP!")
    # Bring up inband and reset the active link on bond.
    ret = utils.SetupInbandInterface()
    if ret != api.types.status.SUCCESS:
        return ret

    return api.types.status.SUCCESS

def Trigger(tc):
    collector_info = utils.GetMirrorCollectorsInfo(tc.collector_wl, tc.collector_ip, tc.collector_type)
    ret = utils.RunAll(tc, tc.verif_json, 'mirror', collector_info, tc.IsBareMetal)
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

    # Rerun the test
    ret = utils.RunAll(tc, tc.verif_json, 'mirror', collector_info, tc.IsBareMetal)
    api.Logger.info("Waiting for switch flap thread to join..")
    flapTask.join(tc.port_down_time)
    return ret['res']

def Verify(tc):
    result = api.types.status.SUCCESS
    return result

def Teardown(tc):
    api.Logger.info("Tearing down ...")

    # Remove secondary IP address
    utils.RemoveSecondaryAddress(tc)

    # Delete the objects
    if tc.newObjects:
        agent_api.DeleteConfigObjects(tc.newObjects)
        agent_api.RemoveConfigObjects(tc.newObjects)

    return api.types.status.SUCCESS

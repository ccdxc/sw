#! /usr/bin/python3

import copy
import pdb
import threading
import time

import iota.harness.api as api
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.utils.vmotion_utils as vmotion_utils

def __create_endpoint_info(tc):
    time.sleep(5) # trying to run vmotion and config update concurrently (hack)

    for dest_host, workloads in tc.vmotion_cntxt.MoveRequest.items():
        api.Logger.debug("Creating endpoint info at %s for workloads being moved" % dest_host)
        if not api.IsNaplesNode(dest_host):
            continue 
        for wl in workloads:
            api.Logger.debug("Updating ep-info for %s" % wl.workload_name)
            ep_filter = "meta.name=" + wl.workload_name + ";" 
            objects = agent_api.QueryConfigs("Endpoint", filter=ep_filter) 
            assert(len(objects) == 1) 
            obj = copy.deepcopy(objects[0]) 
            # delete endpoint being moved on new host, TEMP 
            resp = agent_api.DeleteConfigObjects([obj], [dest_host], True)
            if resp != api.types.status.SUCCESS:
                api.Logger.error("DeleteConfigObjects failed for %s for %s" % (wl.workload_name, dest_host))

            obj.spec.node_uuid           = tc.vmotion_cntxt.UUIDMap[dest_host]
            obj.spec.migration           = "START" 
            current_host = tc.vmotion_cntxt.CurrentHome[wl]
            if (api.IsNaplesNode(current_host)):
                obj.status.node_uuid         = tc.vmotion_cntxt.UUIDMap[current_host]
                obj.spec.homing_host_address = api.GetNicMgmtIP(current_host)
            else:
                obj.status.node_uuid         = "0011.2233.4455"  # TEMP
                obj.spec.homing_host_address = "169.169.169.169" # TEMP

            # this triggers endpoint on new host(naples) to setup flows
            agent_api.PushConfigObjects([obj], [dest_host], True)
    api.Logger.debug("Completed endpoint info creation at NewHome")
    return

def __delete_endpoint_info(tc):
    api.Logger.debug("Deleting endpoint info from CurrentHome of moved workloads")
    for wload, host in tc.vmotion_cntxt.CurrentHome.items():
        if not api.IsNaplesNode(host):
            continue

        api.Logger.debug("Deleting ep-info at %s for wload: %s" % (host, wload.workload_name))
        ep_filter = "meta.name=" + wload.workload_name + ";"
        objects = agent_api.QueryConfigs("Endpoint", filter=ep_filter)
        assert(len(objects) == 1)
        agent_api.DeleteConfigObjects(objects[:1], [host], True)
    return

def Main(tc):
    # Check if tc.skip is set
    tc_skip = getattr(tc, 'skip', False)
    if tc_skip: 
        return api.types.status.SUCCESS

    api.Logger.info("Vmotion Callback Invoked. TC: %s" % tc.Name())
    vmotion_enabled = getattr(tc.args, 'vmotion_enable', False)
    if not vmotion_enabled:
        api.Logger.debug("Vmotion is not enabled for TC: %s" % tc.Name())
        return api.types.status.SUCCESS

    tc.vmotion_resp = api.types.status.FAILURE
    if getattr(tc, 'vmotion_cntxt', None):

        # Update the Naples endpoint information - mimicking Venice (run a thread)
        cfg_thread = threading.Thread(target=__create_endpoint_info, args=(tc, ))

        req = api.Trigger_WorkloadMoveRequest()
        for dest_host, workloads in tc.vmotion_cntxt.MoveRequest.items():
            api.Trigger_WorkloadMoveAddRequest(req, workloads, dest_host)
        cfg_thread.start()
        tc.vmotion_resp = api.TriggerMove(req)

        if tc.vmotion_resp != api.types.status.SUCCESS:
            api.Logger.error("Vmotion failed for TC: %s" % tc.Name()) 
            
        cfg_thread.join()

        if tc.vmotion_resp == api.types.status.SUCCESS: 
            __delete_endpoint_info(tc)
            vmotion_utils.UpdateCurrentHome(tc)

    return tc.vmotion_resp


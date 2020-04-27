#! /usr/bin/python3
import time
import json
import copy
import threading
import re
import iota.harness.api as api
import iota.test.iris.config.netagent.api as agent_api

def triggerVmotion(tc, wl, node):
    api.Logger.info("triggering vmotion for workload %s to node %s" %(wl.workload_name, node))
    req = api.Trigger_WorkloadMoveRequest()
    api.Trigger_WorkloadMoveAddRequest(req, [wl], node)
    tc.resp = api.TriggerMove(req)

def getWorkloadsToRemove(tc, node):
    #import pdb; pdb.set_trace()
    wl_to_rem = []
    new_node  = None
    workloads = api.GetWorkloads(node)
    for wl in workloads:
        m = re.search(node,wl.workload_name)
        if m:
            continue
        else:
            wl_to_rem.append(wl)
        if not new_node:
            n = re.search('(.*)-ep(.*)',wl.workload_name)
            new_node = n.group(1)
    api.Logger.info("Removing workload count {} to move to {}".format(wl_to_rem, new_node))
    return (wl_to_rem, new_node)

def create_ep_info(tc, wl, new_node, migr_state, old_node):
    # get a naples handle to move to
    ep_filter = "meta.name=" + wl.workload_name + ";"
    objects = agent_api.QueryConfigs("Endpoint", filter=ep_filter)
    assert(len(objects) == 1)
    object                          = copy.deepcopy(objects[0])
    # delete endpoint being moved on new host, TEMP
    agent_api.DeleteConfigObjects([object], [new_node], True)

    object.spec.node_uuid           = tc.uuidMap[new_node]
    object.spec.migration           = migr_state 
    if (api.IsNaplesNode(old_node)):
        object.status.node_uuid         = tc.uuidMap[old_node]
        object.spec.homing_host_address = api.GetNicMgmtIP(old_node)
    else:
        object.status.node_uuid         = "0011.2233.4455"  # TEMP
        object.spec.homing_host_address = "169.169.169.169" # TEMP
    # this triggers endpoint on new host(naples) to setup flows
    agent_api.PushConfigObjects([object], [new_node], True)

def delete_ep_info(tc, wl, node):
    ep_filter = "meta.name=" + wl.workload_name + ";"
    objects = agent_api.QueryConfigs("Endpoint", filter=ep_filter)
    assert(len(objects) == 1)
    object = objects[0]
    agent_api.DeleteConfigObjects([object], [node], True)


def Setup(tc):
    vm_threads = []
    node_list  = []
    node = getattr(tc.args, "node", None)
    if node:
        node_list.append(node)
    else:
        '''
        add all nodes in the topo
        '''
        nodes = api.GetNodes()
        for node in nodes:
            node_list.append(node.Name())
    tc.uuidMap  = api.GetNaplesNodeUuidMap()
    for node in node_list:
        (wls,new_node) = getWorkloadsToRemove(tc, node)
        for wl in  wls:
            api.Logger.info("Moving wl {} from node {} to node {}".format(wl.workload_name, wl.node_name,new_node))
            vm_thread = threading.Thread(target=triggerVmotion, args=(tc, wl, new_node,))
            vm_threads.append(vm_thread)
            vm_thread.start()
            if (api.IsNaplesNode(new_node)):
                create_ep_info(tc, wl, new_node, "START", node)
        for vm_thread in vm_threads:
            vm_thread.join()
        for wl in wls:
            if (api.IsNaplesNode(node)):
                delete_ep_info(tc, wl, node)
    return api.types.status.SUCCESS

def Trigger(tc):
    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

#! /usr/bin/python3
import time
import json
import copy
import threading
import re
import iota.harness.api as api
import iota.test.iris.config.netagent.api as agent_api
import iota.test.utils.ping as ping
import iota.test.iris.testcases.vmotion.arping as arping
import iota.test.iris.testcases.vmotion.vm_utils as vm_utils 

def triggerVmotions(tc, wls, node):
    for wl in wls:
        api.Logger.info("triggering vmotion for workload %s to node %s" %(wl.workload_name, node))
    req = api.Trigger_WorkloadMoveRequest()
    api.Trigger_WorkloadMoveAddRequest(req, wls, node)
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

def getNonNaplesNodes(tc):
    tc.NonNaplesNodes = list()
    for node in tc.AllNodes:
        if node not in tc.Nodes:
            tc.NonNaplesNodes.append(node)

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
    tc.Nodes    = api.GetNaplesHostnames()
    tc.AllNodes = api.GetWorkloadNodeHostnames()
    tc.uuidMap  = api.GetNaplesNodeUuidMap()
    tc.move_info         = []
    tc.vm_dsc_to_dsc     = True
    tc.num_moves         = 0

    if hasattr(tc.args, "conntrack"):
        tc.detailed = True
    else:
        tc.detailed = False


    getNonNaplesNodes(tc)
    if arping.ArPing(tc) != api.types.status.SUCCESS:
        api.Logger.info("arping failed on setup")
    if ping.TestPing(tc, 'local_only', 'ipv4', 64) != api.types.status.SUCCESS or ping.TestPing(tc, 'remote_only', 'ipv4', 64) != api.types.status.SUCCESS:
        api.Logger.info("ping test failed on setup")
        return api.types.status.FAILURE

    for node in node_list:
        (wls,new_node) = getWorkloadsToRemove(tc, node)
        tc.num_moves = len(wls)
        vm_utils.update_move_info(tc, wls, False, new_node)

    #Start Fuz
    ret = vm_utils.start_fuz(tc)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Fuz start failed")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Trigger(tc):
    tc.resp = vm_utils.do_vmotion(tc, tc.vm_dsc_to_dsc)
    return tc.resp 

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    new_node = ''
    ret1 =  vm_utils.wait_and_verify_fuz(tc)
    ret2 = api.types.status.SUCCESS
    for wl_info in tc.move_info:
        wl_info.sess_info_after = vm_utils.get_session_info(tc, wl_info.wl)
        if (wl_info.sess_info_before): 
            ret2 = vm_utils.verify_session_info(tc, wl_info)
        new_node = wl_info.new_node 
    if ret1 != api.types.status.SUCCESS or ret2 != api.types.status.SUCCESS:
        return api.types.status.FAILURE
    if vm_utils.verify_dbg_vmotion(tc, new_node) != api.types.status.SUCCESS:
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

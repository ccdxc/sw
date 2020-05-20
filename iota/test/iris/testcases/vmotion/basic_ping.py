#! /usr/bin/python3
import time
import json
import copy
import threading
import ipaddress
import iota.harness.api as api
import iota.test.iris.config.netagent.api as agent_api
import iota.test.utils.ping as ping
import iota.test.iris.testcases.vmotion.arping as arping
import iota.test.iris.testcases.vmotion.vm_utils as vm_utils 
from iota.test.iris.testcases.penctl.common import GetNaplesUUID

def getWorkloads(tc, type):
    if type == 'naples':
        if not len(tc.Nodes):
            return None
        node = tc.Nodes[0]
    else:
        if not len(tc.NonNaplesNodes):
            return None
        node = tc.NonNaplesNodes[0]
    return api.GetWorkloads(node)

def getNonNaplesNodes(tc):
    tc.NonNaplesNodes = list()
    for node in tc.AllNodes:
        if node not in tc.Nodes:
            tc.NonNaplesNodes.append(node)

def Setup(tc):
    '''
    for n in api.GetNaplesHostnames():
        uuid = GetNaplesUUID(n)
        api.SetNaplesNodeUuid(n, uuid)
    '''

    tc.Nodes    = api.GetNaplesHostnames()
    tc.AllNodes = api.GetWorkloadNodeHostnames()
    tc.uuidMap  = api.GetNaplesNodeUuidMap()
    tc.new_node = None
    tc.old_node = None
    tc.vm_non_dsc_to_dsc = False
    tc.vm_dsc_to_dsc     = False
    tc.skip_teardown     = False
    tc.move_info         = []
    tc.dbg_stats_before  = dict()
    tc.dbg_stats_after   = dict()
    if tc.args.vm_type == 'non_dsc_to_dsc':
        tc.vm_non_dsc_to_dsc = True
    else:
        tc.vm_dsc_to_dsc     = True 
    tc.num_moves = int(getattr(tc.args, "num_moves", 1))
    tc.dsc_conn_type  = getattr(tc.args, "dsc_con_type", "oob")

    if hasattr(tc.args, "conntrack"):
        tc.detailed = True
    else:
        tc.detailed = False

    if hasattr(tc.args,"skip_teardown"):
        api.Logger.info("Setting tc to skip teardown")
        tc.skip_teardown = True
        #if tc.args.skip_teardown == "True":
        #    tc.skip_teardown = True

    vm_utils.increase_timeout()
        
    getNonNaplesNodes(tc)
    if arping.ArPing(tc) != api.types.status.SUCCESS:
        api.Logger.info("arping failed on setup")
    if ping.TestPing(tc, 'local_only', 'ipv4', 64) != api.types.status.SUCCESS or ping.TestPing(tc, 'remote_only', 'ipv4', 64) != api.types.status.SUCCESS:
        api.Logger.info("ping test failed on setup")
        return api.types.status.FAILURE

    '''
    identify workloads to be moved, as we want to run fuz if wl being moved is involved
    '''
    vm_utils.create_move_info(tc, tc.vm_dsc_to_dsc)

    vm_utils.get_vm_dbg_stats(tc)

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
    new_node = ''
    for wl_info in tc.move_info:
        vm_utils.get_sessions_info(tc, wl_info.new_node)
        wl_info.sess_info_after = vm_utils.get_session_info(tc, wl_info.wl)
        import pprint
        pprint.pprint(wl_info.sess_info_after)
    ret1 =  vm_utils.wait_and_verify_fuz(tc)
    if tc.resp is None:
        api.Logger.info("trigger failed, skipping verify")
        return api.types.status.FAILURE
    ret2 = api.types.status.SUCCESS
    for wl_info in tc.move_info:
        if (wl_info.sess_info_before): 
            ret2 = vm_utils.verify_session_info(tc, wl_info)
        new_node = wl_info.new_node 
    if ret1 != api.types.status.SUCCESS or ret2 != api.types.status.SUCCESS:
        return api.types.status.FAILURE
    if vm_utils.verify_dbg_vmotion(tc, new_node) != api.types.status.SUCCESS:
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Teardown(tc):
    # adding sleep to make sure last occurance of vmotion is updated in old(src) node
    time.sleep(5)
    if tc.skip_teardown:
        api.Logger.info("skipping teardown")
        return api.types.status.SUCCESS
    if tc.GetStatus() != api.types.status.SUCCESS:
        api.Logger.info("verify failed, returning without teardown")
        return tc.GetStatus()

    vm_utils.move_back_vms(tc)

    return vm_utils.verify_vm_dbg_stats(tc) 

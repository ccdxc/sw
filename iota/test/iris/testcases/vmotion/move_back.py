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

__fuz_run_time = "120s"

def copy_fuz(tc):
    tc.fuz_exec = {}
    def copy_to_entity(entity):
        fullpath = api.GetTopDir() + '/iota/bin/fuz'
        resp = api.CopyToWorkload(entity.node_name, entity.workload_name, [fullpath], '')
        #Create a symlink at top level
        realPath = "realpath fuz"
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddCommand(req, entity.node_name, entity.workload_name, realPath, background = False)
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                return api.types.status.FAILURE
            tc.fuz_exec[cmd.entity_name] = cmd.stdout.split("\n")[0]
        return api.types.status.SUCCESS

    for idx, pairs in enumerate(tc.workload_pairs):
        ret = copy_to_entity(pairs[0])
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
        ret = copy_to_entity(pairs[1])
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def start_fuz(tc):
    ret = copy_fuz(tc)
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    tc.serverCmds = []
    tc.clientCmds = []
    tc.cmd_descr = []

    serverReq = None
    clientReq = None

    serverReq = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    clientReq = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    # ping test above sets the workload pairs to remote only
    # setting wl_pairs as per arg selected in testbundle 
    workload_pairs = []
 
    if tc.args.type == 'local_only':
        api.Logger.info("local_only test")
        workload_pairs = api.GetLocalWorkloadPairs()
    elif tc.args.type == 'both':
        api.Logger.info(" both local and remote test")
        workload_pairs = api.GetLocalWorkloadPairs()
        workload_pairs.extend(api.GetRemoteWorkloadPairs())
    else:
        api.Logger.info("remote_only test")
        workload_pairs = api.GetRemoteWorkloadPairs()

    wl_under_move = []
    for wl_info in tc.move_info:
        wl_under_move.append(wl_info.wl)
    '''
    tc.workload_pairs updated in ping test above
    resetting that to limit fuz tests to vm under move
    ''' 
    tc.workload_pairs = [] 
    for pairs in workload_pairs:
        if pairs[0] in wl_under_move or pairs[1] in wl_under_move:
            api.Logger.info("Adding %s and %s for fuz test" %(pairs[0].workload_name, pairs[1].workload_name))
            tc.workload_pairs.append(pairs)

    for idx, pairs in enumerate(tc.workload_pairs):
        client = pairs[0]
        server = pairs[1]
        cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                       (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
        tc.cmd_descr.append(cmd_descr)
        num_sessions = int(getattr(tc.args, "num_sessions", 1))

        serverCmd = None
        clientCmd = None
        port = api.AllocateTcpPort()

        api.Logger.info("Starting Fuz test from %s num-sessions %d Port %d" % (cmd_descr, num_sessions, port))

        serverCmd = tc.fuz_exec[server.workload_name]  + " -port " + str(port)
        clientCmd = tc.fuz_exec[client.workload_name]  + " -conns " + str(num_sessions) + " -duration " + str(__fuz_run_time) + " -attempts 1 -read-timeout 100 -talk " + server.ip_address + ":" + str(port)

        tc.serverCmds.append(serverCmd)
        tc.clientCmds.append(clientCmd)

        api.Trigger_AddCommand(serverReq, server.node_name, server.workload_name,
                               serverCmd, background = True)

        api.Trigger_AddCommand(clientReq, client.node_name, client.workload_name,
                               clientCmd, background = True)


    tc.server_resp = api.Trigger(serverReq)
    #Sleep for some time as bg may not have been started.
    time.sleep(5)
    tc.fuz_client_resp = api.Trigger(clientReq)
    return api.types.status.SUCCESS

def update_node_info(tc, cmd_resp):
    wl_moving = {} 
    for move_info in tc.move_info:
        wl_moving[move_info.wl.workload_name] = move_info
    for cmd in cmd_resp.commands:
        if cmd.entity_name in wl_moving.keys():
            move_info = wl_moving[cmd.entity_name]
            api.Logger.info("updating node in for entity {} to node {}".format(cmd.entity_name, move_info.new_node)) 
            api.Trigger_UpdateNodeForCommands(cmd_resp, cmd.entity_name, 
                                              move_info.old_node, 
                                              move_info.new_node)


def wait_and_verify_fuz(tc):
    update_node_info(tc, tc.server_resp)
    update_node_info(tc, tc.fuz_client_resp)

    tc.fuz_client_resp = api.Trigger_WaitForAllCommands(tc.fuz_client_resp)
    api.Trigger_TerminateAllCommands(tc.server_resp)
    for idx, cmd in enumerate(tc.fuz_client_resp.commands):
        if cmd.exit_code != 0:
            api.Logger.error("Fuz commmand failed Workload : {}, command : {},  stdout : {} stderr : {}".format(cmd.entity_name, cmd.command, cmd.stdout, cmd.stderr))
            return api.types.status.FAILURE

    api.Logger.info("Fuz test successfull")
    return api.types.status.SUCCESS


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

class MoveInfo:
    def __init__(self):
        self.new_node         = ''
        self.wl               = ''
        self.old_node         = ''
        self.sess_info_before = None
        self.sess_info_after  = None
        return

def update_move_info(tc, workloads, factor_l2seg, new_node):
    wire_encap   = []
    for wl in workloads:
        #import pdb; pdb.set_trace()
        if tc.num_moves <= 2:
            if (factor_l2seg and wl.uplink_vlan in wire_encap):
                continue
        move_info = MoveInfo()
        move_info.new_node = new_node
        move_info.wl       = wl
        move_info.old_node = wl.node_name 
        wire_encap.append(wl.uplink_vlan)
        tc.move_info.append(move_info)
        if (len(tc.move_info) == tc.num_moves):
            break
    api.Logger.info("Num of move_info elements are {}".format(len(tc.move_info)))

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
        update_move_info(tc, wls, False, new_node)

    #Start Fuz
    ret = start_fuz(tc)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Fuz start failed")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def do_vmotion(tc, dsc_to_dsc):
    vm_threads = []
    wls        = []

    for wl_info in tc.move_info:
        api.Logger.info("moving wl {} from node {} to node {}".format(wl_info.wl.workload_name, wl_info.old_node, wl_info.new_node))
        wls.append(wl_info.wl)

    vm_thread = threading.Thread(target=triggerVmotions,args=(tc, wls, wl_info.new_node, ))
    vm_threads.append(vm_thread)
    vm_thread.start()

    for wl_info in tc.move_info: 
        vm_utils.create_ep_info(tc, wl_info.wl, wl_info.new_node, "START", wl_info.old_node)
    
    # wait for vmotion thread to complete, meaning vmotion is done on vcenter
    for vm_thread in vm_threads:
        vm_thread.join()
    return api.types.status.SUCCESS


def Trigger(tc):
    tc.resp = do_vmotion(tc, tc.vm_dsc_to_dsc)
    return tc.resp 

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    new_node = ''
    for wl_info in tc.move_info:
        wl_info.sess_info_after = vm_utils.get_session_info(tc, wl_info.wl)

    ret1 =  wait_and_verify_fuz(tc)
    ret2 = api.types.status.SUCCESS
    for wl_info in tc.move_info:
        vm_utils.get_sessions_info(tc, wl_info.new_node)
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

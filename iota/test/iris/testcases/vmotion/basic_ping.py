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
            api.Logger.info("updating node in for entity {} to node {}", 
                             cmd.entity_name, move_info.new_node) 
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
            api.Logger.error("Fuz commmand failed Workload : {}, command : {},  stdout : {} stderr : {}", cmd.entity_name, cmd.command, cmd.stdout, cmd.stderr)
            return api.types.status.FAILURE

    api.Logger.info("Fuz test successfull")
    return api.types.status.SUCCESS

def triggerVmotion(tc, wl, node):
    api.Logger.info("triggering vmotion for workload %s to node %s" %(wl.workload_name, node))
    req = api.Trigger_WorkloadMoveRequest()
    api.Trigger_WorkloadMoveAddRequest(req, [wl], node)
    tc.resp = api.TriggerMove(req)

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

def create_move_info(tc, dsc_to_dsc):
    factor_l2seg = True
    new_node = ''
    old_node = ''
    api.Logger.info("In do_vmotion for dsc_to_dsc {}".format(dsc_to_dsc))
    # get first naples node to move VM to
    if (dsc_to_dsc):
        assert(len(tc.Nodes) >=2 )
        # set old name as Node[0] and new node as Node[1]
        old_node = tc.Nodes[0]
        new_node = tc.Nodes[1]
    else:
        assert(len(tc.Nodes) >= 1)
        assert(len(tc.NonNaplesNodes) >= 1)
        old_node = tc.NonNaplesNodes[0]
        new_node = tc.Nodes[0]
        
    workloads = api.GetWorkloads(old_node)
    assert(len(workloads) != 0)
    api.Logger.info("Identify workloads to move from {} to {}".format(old_node, new_node))
    update_move_info(tc, workloads, factor_l2seg, new_node)


def do_vmotion(tc, dsc_to_dsc):
    vm_threads = []

    for wl_info in tc.move_info:
        if (api.IsNaplesNode(wl_info.wl.node_name)):
            wl_info.sess_info_before = vm_utils.get_session_info(tc, wl_info.wl)
            vm_utils.get_sessions_info(tc, wl_info.old_node)
        api.Logger.info("moving wl {} from node {} to node {}".format(wl_info.wl.workload_name, wl_info.old_node, wl_info.new_node))
        vm_thread = threading.Thread(target=triggerVmotion,args=(tc, wl_info.wl, wl_info.new_node, ))
        vm_threads.append(vm_thread)
        vm_thread.start()
        vm_utils.create_ep_info(tc, wl_info.wl, wl_info.new_node, "START", wl_info.old_node)
    # wait for vmotion thread to complete, meaning vmotion is done on vcenter
    for vm_thread in vm_threads:
        vm_thread.join()
    return api.types.status.SUCCESS

def getNonNaplesNodes(tc):
    tc.NonNaplesNodes = list()
    for node in tc.AllNodes:
        if node not in tc.Nodes:
            tc.NonNaplesNodes.append(node)

def Setup(tc):
    tc.Nodes    = api.GetNaplesHostnames()
    tc.AllNodes = api.GetWorkloadNodeHostnames()
    tc.uuidMap  = api.GetNaplesNodeUuidMap()
    tc.new_node = None
    tc.old_node = None
    tc.vm_non_dsc_to_dsc = False
    tc.vm_dsc_to_dsc     = False
    tc.move_info         = []
    if tc.args.vm_type == 'non_dsc_to_dsc':
        tc.vm_non_dsc_to_dsc = True
    else:
        tc.vm_dsc_to_dsc     = True 
    tc.num_moves = int(getattr(tc.args, "num_moves", 1))
    tc.dsc_conn_type  = getattr(tc.args, "dsc_con_type", "oob")
        
    getNonNaplesNodes(tc)
    if arping.ArPing(tc) != api.types.status.SUCCESS:
        api.Logger.info("arping failed on setup")
    if ping.TestPing(tc, 'local_only', 'ipv4', 64) != api.types.status.SUCCESS or ping.TestPing(tc, 'remote_only', 'ipv4', 64) != api.types.status.SUCCESS:
        api.Logger.info("ping test failed on setup")
        return api.types.status.FAILURE

    '''
    identify workloads to be moved, as we want to run fuz if wl being moved is involved
    '''
    create_move_info(tc, tc.vm_dsc_to_dsc)

    #Start Fuz
    ret = start_fuz(tc)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Fuz start failed")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Trigger(tc):
    tc.resp = do_vmotion(tc, tc.vm_dsc_to_dsc)
    return tc.resp 

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    for wl_info in tc.move_info:
        wl_info.sess_info_after = vm_utils.get_session_info(tc, wl_info.wl)

    ret1 =  wait_and_verify_fuz(tc)
    ret2 = api.types.status.SUCCESS
    for wl_info in tc.move_info:
        vm_utils.get_sessions_info(tc, wl_info.new_node)
        if (wl_info.sess_info_before): 
            ret2 = vm_utils.verify_session_info(tc, wl_info)
    
    if ret1 != api.types.status.SUCCESS or ret2 != api.types.status.SUCCESS:
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Teardown(tc):
    # adding sleep to make sure last occurance of vmotion is updated in old(src) node
    time.sleep(5)
    if tc.GetStatus() != api.types.status.SUCCESS:
        api.Logger.info("verify failed, returning without teardown")
        return tc.GetStatus()
    vm_threads = [] 
    for wl_info in tc.move_info:    
        # if new_node and old_node handles exists, vmotion trigger happend
        # cleanup and restore to as per DB in iota
        if (wl_info.new_node and wl_info.old_node):
            vm_thread = threading.Thread(target=triggerVmotion, args=(tc, wl_info.wl, wl_info.old_node,))
            vm_threads.append(vm_thread)
            vm_thread.start()
            if (api.IsNaplesNode(wl_info.old_node)):
                vm_utils.create_ep_info(tc, wl_info.wl, wl_info.old_node, "START", wl_info.new_node)
    for vm_thread in vm_threads:
        vm_thread.join()
    for wl_info in tc.move_info:
       if (api.IsNaplesNode(wl_info.new_node)):
            vm_utils.delete_ep_info(tc, wl_info.wl, wl_info.new_node)
    return api.types.status.SUCCESS

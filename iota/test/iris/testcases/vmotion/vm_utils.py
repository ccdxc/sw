#! /usr/bin/python3

import time
import copy
import ipaddress
import threading
import re
import iota.test.iris.config.netagent.api as agent_api
import iota.harness.api as api
import yaml


__fuz_run_time = "240s"

class MoveInfo:
    def __init__(self):
        self.new_node         = ''
        self.wl               = ''
        self.old_node         = ''
        self.sess_info_before = None
        self.sess_info_after  = None
        return


def get_sessions_info(tc, node):
    tc.cmd_cookies = []
    sessions       = []
    api.Logger.info("showing session info on node %s" % (node))
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd_cookie = "hal show session"
    api.Trigger_AddNaplesCommand(req, node, "/nic/bin/halctl show session ")
    tc.cmd_cookies.append(cmd_cookie)
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
    return



def get_session_info(tc, wl):
    tc.cmd_cookies = []
    sessions       = []
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd_cookie = "hal show session"
    api.Logger.info("Getting session info from node %s" %(wl.node_name))
    api.Trigger_AddNaplesCommand(req, wl.node_name, "/nic/bin/halctl show session --dstip %s --yaml" % (wl.ip_address))
    tc.cmd_cookies.append(cmd_cookie)
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if tc.cmd_cookies[cookie_idx].find("hal show session") != -1 and cmd.stdout == '':
           api.Logger.info("hal show session returned no sessions")
           return None
        else:
           yaml_out = yaml.load_all(cmd.stdout, Loader=yaml.FullLoader)
           for session_info in yaml_out:
                sessions.append(session_info)
    api.Logger.info('session_info for {} sessions'.format(len(sessions)))
    return sessions

def get_session_per_node_info(tc, wl, node):
    tc.cmd_cookies = []
    sessions       = []
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd_cookie = "hal show session"
    api.Trigger_AddNaplesCommand(req, node_name, "/nic/bin/halctl show session --dstip %s --yaml" % (wl.ip_address))
    tc.cmd_cookies.append(cmd_cookie)
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    cookie_idx = 0
    for cmd in tc.resp.commands:
        if tc.cmd_cookies[cookie_idx].find("hal show session") != -1 and cmd.stdout == '':
           api.Logger.info("hal show session returned no sessions")
           return None
        else:
           yaml_out = yaml.load_all(cmd.stdout, Loader=yaml.FullLoader)
           for session_info in yaml_out:
                sessions.append(session_info)
    api.Logger.info('session_info for {} sessions'.format(len(sessions)))
    return sessions


def build_dict(session, sess_dict):
    # v4 key assumed
    i_flow_key  = session['spec']['initiatorflow']['flowkey']['flowkey']['v4key']
    if 'l4fields' not in i_flow_key:
        return
    if 'tcpudp' not in i_flow_key['l4fields']:
        return
    # key -> sip, dip, proto, sport, dport
    sess_i_key  = (i_flow_key['sip'], 
                       i_flow_key['dip'], 
                       i_flow_key['ipproto'], 
                       i_flow_key['l4fields']['tcpudp']['sport'],
                       i_flow_key['l4fields']['tcpudp']['dport'])
    sess_i_data = {'action': session['spec']['initiatorflow']['flowdata']['flowinfo']['flowaction']}
    if 'conntrackinfo' in session['spec']['initiatorflow']['flowdata']:
        sess_i_data['tcpwinscale'] = session['spec']['initiatorflow']['flowdata']['conntrackinfo']['tcpwinscale']
        sess_i_data['tcpmss'] = session['spec']['initiatorflow']['flowdata']['conntrackinfo']['tcpmss']
    if 'initiatorflowstats' in session['stats']:
        sess_i_data['packets'] = session['stats']['initiatorflowstats']['flowpermittedpackets']
    sess_dict[sess_i_key] = sess_i_data

    if 'flowkey' not in session['spec']['responderflow'] or 'v4key' not in session['spec']['responderflow']['flowkey']['flowkey']:
        return
    r_flow_key  = session['spec']['responderflow']['flowkey']['flowkey']['v4key']
    sess_r_key  = (r_flow_key['sip'], 
                       r_flow_key['dip'], 
                       r_flow_key['ipproto'], 
                       r_flow_key['l4fields']['tcpudp']['sport'],
                       r_flow_key['l4fields']['tcpudp']['dport'])
    sess_r_data = {'action': session['spec']['responderflow']['flowdata']['flowinfo']['flowaction']}
    if 'conntrackinfo' in session['spec']['responderflow']['flowdata']:
        sess_r_data['tcpwinscale'] = session['spec']['responderflow']['flowdata']['conntrackinfo']['tcpwinscale']
        sess_r_data['tcpmss'] = session['spec']['responderflow']['flowdata']['conntrackinfo']['tcpmss']
    if 'responderflowstats' in session['stats']:
        sess_r_data['packets'] = session['stats']['responderflowstats']['flowpermittedpackets']
    sess_dict[sess_r_key] = sess_r_data

def compare_session_info(sess_before_dict, sess_after_dict, detailed=False):
    ret = api.types.status.SUCCESS
    for flow_key,flow_data in sess_before_dict.items():
        (sip, dip, proto, sport, dport) = flow_key
        sip = str(ipaddress.ip_address(sip))
        dip = str(ipaddress.ip_address(dip))
        if flow_key in sess_after_dict:
            api.Logger.info('flow key sip:{} dip:{} proto:{} sport:{} dport:{} '.format(sip, dip, proto, sport, dport))
            sess_before_data = sess_before_dict[flow_key]
            sess_after_data  = sess_after_dict[flow_key]
            if (sess_before_data['action'] != sess_after_data['action']): 
                action_before = sess_before_dict[flow_key]['action']
                action_after  = sess_after_dict[flow_key]['action']
                api.Logger.info('flow action mismatch {} {} {} {} {} -> before {}, after {}  '.format(sip, dip, proto, sport, dport, action_before, action_after))
                ret = api.types.status.FAILURE
                continue
            if not detailed:
                api.Logger.info('flow compare for action successful')
                continue 
            if 'tcpwinscale' not in sess_before_data:
                continue
            if ((sess_before_data['tcpwinscale'] != sess_after_data['tcpwinscale']) or 
                (sess_before_data['tcpmss']      != sess_after_data['tcpmss'])): 
                api.Logger.info('flow conntrack info mismatch key sip:{} dip:{} proto:{} sport:{} dport:{} '.format(sip, dip, proto, sport, dport))
                api.Logger.info('before data tcpwinscale : {} tcpmss : {}'.format(sess_before_data['tcpwinscale'], sess_before_data['tcpmss']))
                api.Logger.info('after data tcpwinscale : {} tcpmss : {}'.format(sess_after_data['tcpwinscale'], sess_after_data['tcpmss']))
                ret = api.types.status.FAILURE
                continue 
            
            if 'packets' not in sess_before_data:
                continue
            if (sess_after_data['packets'] < sess_before_data['packets']): 
                api.Logger.info('flow conntrack info mismatch key sip:{} dip:{} proto:{} sport:{} dport:{} '.format(sip, dip, proto, sport, dport))
                api.Logger.info('permitted pkts before : {}, after : {}'.format(sess_before_data['packets'], sess_after_data['packets']))
                ret = api.types.status.FAILURE
                continue 
            api.Logger.info('flow compare successful')
        else:
            api.Logger.info('flow not found {} {} {} {} {}'.format(sip, dip, proto, sport, dport))
            ret = api.types.status.FAILURE
    return ret 

def verify_session_info(tc, wl_info):
    sess_before_dict = {} 
    sess_after_dict  = {} 
    if not (hasattr(wl_info, 'sess_info_before')):
        api.Logger.info("no sessions before move")
        return api.types.status.SUCCESS
    if not (hasattr(wl_info, 'sess_info_after')):
        api.Logger.info("no sessions after move")
        return api.types.status.SUCCESS
    for session in wl_info.sess_info_before:
        if session != None:
            build_dict(session, sess_before_dict)
    for session in wl_info.sess_info_after:
        if session != None:
            build_dict(session, sess_after_dict)
    ret = compare_session_info(sess_before_dict, sess_after_dict, tc.detailed)
    if ret != api.types.status.SUCCESS:
        api.Logger.info('session compare failed for wl {}'.format(wl_info.wl.workload_name))
    api.Logger.info('session compare successful for wl {}'.format(wl_info.wl.workload_name))
    return ret 

def pick_new_node(tc):
    # remove the node being used to pick wl for traffic
    # pick the next node in the naples nodes
    naples_nodes = tc.Nodes[:]
    naples_nodes.remove(tc.server.node_name)
    assert(len(naples_nodes) >= 1)
    new_node     = naples_nodes[0]
    tc.new_node  = new_node
    return api.types.status.SUCCESS

def triggerVmotion(tc, wl, node):
    api.Logger.info("triggering vmotion for workload %s to node %s" %(wl.workload_name, node))
    req = api.Trigger_WorkloadMoveRequest()
    api.Trigger_WorkloadMoveAddRequest(req, [wl], node)
    tc.resp = api.TriggerMove(req)

def triggerVmotions(tc, wls, node):
    for wl in wls:
        api.Logger.info("triggering vmotion for workload %s to node %s" %(wl.workload_name, node))
    req = api.Trigger_WorkloadMoveRequest()
    api.Trigger_WorkloadMoveAddRequest(req, wls, node)
    tc.resp = api.TriggerMove(req)

def do_vmotion(tc, dsc_to_dsc):
    vm_threads = []
    wls        = []

    for wl_info in tc.move_info:
        api.Logger.info("moving wl {} from node {} to node {}".format(wl_info.wl.workload_name, wl_info.old_node, wl_info.new_node))
        wls.append(wl_info.wl)
        if (api.IsNaplesNode(wl_info.wl.node_name)):
            wl_info.sess_info_before = get_session_info(tc, wl_info.wl)

    vm_thread = threading.Thread(target=triggerVmotions,args=(tc, wls, wl_info.new_node, ))
    vm_threads.append(vm_thread)
    vm_thread.start()

    for wl_info in tc.move_info: 
        create_ep_info(tc, wl_info.wl, wl_info.new_node, "START", wl_info.old_node)
    
    # wait for vmotion thread to complete, meaning vmotion is done on vcenter
    for vm_thread in vm_threads:
        vm_thread.join()
    if tc.resp != api.types.status.SUCCESS:
        api.Logger.info("vmotion failed")
        return api.types.status.FAILURE
    else:
        api.Logger.info("vmotion successful")
        return api.types.status.SUCCESS


def create_ep_info(tc, wl, dest_node, migr_state, src_node):
    # get a naples handle to move to
    ep_filter = "meta.name=" + wl.workload_name + ";"
    if not hasattr(tc, 'dsc_conn_type'):
       api.Logger.info(" seeing dsc_conn_type to oob")
       tc.dsc_conn_type = 'oob'  
    objects = agent_api.QueryConfigs("Endpoint", filter=ep_filter)
    assert(len(objects) == 1)
    object                          = copy.deepcopy(objects[0])
    # delete endpoint being moved on new host, TEMP
    agent_api.DeleteConfigObjects([object], [dest_node], True)

    object.spec.node_uuid           = tc.uuidMap[dest_node]
    object.spec.migration           = migr_state 
    if (api.IsNaplesNode(src_node)):
        object.status.node_uuid         = tc.uuidMap[src_node]
        if (tc.dsc_conn_type == "oob"):
            object.spec.homing_host_address = api.GetNicMgmtIP(src_node)
        else:
            object.spec.homing_host_address = api.GetBondIp(src_node)
    else:
        object.status.node_uuid         = "0011.2233.4455"  # TEMP
        object.spec.homing_host_address = "169.169.169.169" # TEMP
    # this triggers endpoint on new host(naples) to setup flows
    agent_api.PushConfigObjects([object], [dest_node], True)

def delete_ep_info(tc, wl, node):
    ep_filter = "meta.name=" + wl.workload_name + ";"
    objects = agent_api.QueryConfigs("Endpoint", filter=ep_filter)
    assert(len(objects) == 1)
    object = objects[0]
    agent_api.DeleteConfigObjects([object], [node], True)

def vm_move_back(tc):
    vm_threads = [] 
    # if new_node and old_node handles exists, vmotion trigger happend
    # cleanup and restore to as per DB in iota
    if (tc.new_node and tc.old_node):
       vm_thread = threading.Thread(target=triggerVmotion, args=(tc, tc.wl, tc.old_node,))
       vm_threads.append(vm_thread)
       vm_thread.start()
       if (api.IsNaplesNode(tc.old_node)):
           create_ep_info(tc, tc.wl, tc.old_node, "START", tc.new_node)
    for vm_thread in vm_threads:
       vm_thread.join()
    if (api.IsNaplesNode(tc.new_node)):
       delete_ep_info(tc, tc.wl, tc.new_node)
    return api.types.status.SUCCESS

def increase_timeout():
    #Query will get the reference of objects on store
    store_profile_objects = agent_api.QueryConfigs(kind='SecurityProfile')
    if len(store_profile_objects) == 0:
        api.Logger.error("No security profile objects in store")
        return api.types.status.FAILURE

    for object in store_profile_objects:
        object.spec.timeouts.session_idle = "360s"
        object.spec.timeouts.tcp = "360s"
        object.spec.timeouts.udp = "360s"
        object.spec.timeouts.icmp = "120s"
        object.spec.timeouts.tcp_half_close = "360s"
        object.spec.timeouts.tcp_close = "360s"
        object.spec.timeouts.tcp_connection_setup = "60s"
        object.spec.timeouts.tcp_drop = "360s"
        object.spec.timeouts.udp_drop = "60s"
        object.spec.timeouts.icmp_drop = "300s"

    #Now push the update as we modified.
    agent_api.UpdateConfigObjects(store_profile_objects)
    return api.types.status.SUCCESS


def vm_process_dbg_out(f):
    ret_dict = dict()
    found_pattern = False
    header_skipped = False
    lines = f.splitlines()
    for line in lines:
        m = re.search('EndPoint.*useg-vlan.*MigrationState',line)
        if m:
            found_pattern = True
            continue
        if found_pattern:
            if not header_skipped:
                header_skipped = True
                continue
            else:
                m2=re.search('(.*)\s+[ \t](\d+)[ \t].*(\d)',line)
                if not m2:
                    break
                mac = m2.group(1).rstrip()
                useg_vlan = m2.group(2)
                mig_state = m2.group(3)
                ret_dict[mac] = {"useg_vlan":useg_vlan,"mig_state":mig_state}
    return ret_dict

def process_dbg_vmotion_output(tc, f):
    ret = api.types.status.SUCCESS
    ep_vm_dict = vm_process_dbg_out(f)
    for moved_ep in tc.move_info:
        mac_addr = moved_ep.wl.mac_address
        if mac_addr in ep_vm_dict.keys():
            api.Logger.info("mac_addr {} moved per vm dbg cmd".format(mac_addr))
        else:
            api.Logger.info("mac_addr {} not found in vm dbg cmd".format(mac_addr))
            ret = api.types.status.FAILURE
    return ret 
 
def verify_dbg_vmotion(tc, node):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd_cookie = "hal debug vmotion"
    api.Trigger_AddNaplesCommand(req, node, "/nic/bin/halctl debug vmotion")
    tc.cmd_cookies.append(cmd_cookie)
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    cookie_idx = 0
    for cmd in tc.resp.commands:
        #api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
        #api.PrintCommandResults(cmd)
        if tc.cmd_cookies[cookie_idx].find("hal debug vmotion") != -1 and cmd.stdout == '':
           api.Logger.info("hal show session returned no sessions")
           return None
        else:
           ret = process_dbg_vmotion_output(tc, cmd.stdout)
           return ret


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
        clientCmd = tc.fuz_exec[client.workload_name]  + " -conns " + str(num_sessions) + " -duration " + str(__fuz_run_time) + " -attempts 1 -read-timeout 20 -talk " + server.ip_address + ":" + str(port)

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

def update_move_info(tc, workloads, factor_l2seg, new_node):
    wire_encap   = []
    for wl in workloads:
        found = False
        #import pdb; pdb.set_trace()
        if tc.num_moves <= 2:
            if (factor_l2seg and wl.uplink_vlan in wire_encap):
                continue
        for old_wl_moveinfo in tc.move_info:
            if old_wl_moveinfo.wl.workload_name == wl.workload_name:
                found = True
                move_info = old_wl_moveinfo
                break
        if not found:
            move_info     = MoveInfo()
            move_info.wl  = wl
            wire_encap.append(wl.uplink_vlan)
        move_info.new_node = new_node
        move_info.old_node = wl.node_name 
        if not found:
            tc.move_info.append(move_info)
        if (len(tc.move_info) == tc.num_moves):
            break
    api.Logger.info("Num of move_info elements are {}".format(len(tc.move_info)))

def create_move_info(tc, dsc_to_dsc):
    factor_l2seg = True
    new_node = ''
    old_node = ''
    api.Logger.info("In create_move_info for dsc_to_dsc {}".format(dsc_to_dsc))
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


def find_new_node_to_move_to(tc, wl):
    nodes = api.GetNaplesHostnames()
    # remove the node being used to pick wl for traffic
    # pick the next node in the naples nodes
    naples_nodes = nodes[:]
    naples_nodes.remove(wl.node_name)
    assert(len(naples_nodes) >= 1)
    return naples_nodes[0]

def get_memtrack(cmd, allocid):
    yaml_out = yaml.load_all(cmd.stdout, Loader=yaml.FullLoader)
    print(type(yaml_out))
    for data in yaml_out:
        if data is not None:
            if allocid == data['spec']['allocid']:
                stats = data['stats']
                return ({'allocid': allocid, 'allocs': stats['numallocs'], 'frees': stats['numfrees']})
    return {}

def move_back_vms(tc):
    vm_threads = [] 
    wls        = []
    for wl_info in tc.move_info:
        api.Logger.info("moving wl {} from node {} to node {}".format(wl_info.wl.workload_name, wl_info.new_node, wl_info.old_node))
        wls.append(wl_info.wl)

    vm_thread = threading.Thread(target=triggerVmotions,
                                 args=(tc, wls, wl_info.old_node, ))
    vm_threads.append(vm_thread)
    vm_thread.start()

    for wl_info in tc.move_info: 
        create_ep_info(tc, wl_info.wl, wl_info.old_node, "START", wl_info.new_node)
    
    # wait for vmotion thread to complete, meaning vmotion is done on vcenter
    for vm_thread in vm_threads:
        vm_thread.join()
    time.sleep(5)
    for wl_info in tc.move_info:
       if (api.IsNaplesNode(wl_info.new_node)):
            delete_ep_info(tc, wl_info.wl, wl_info.new_node)

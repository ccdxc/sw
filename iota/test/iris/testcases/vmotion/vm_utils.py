#! /usr/bin/python3

import time
import copy
import ipaddress
import threading
import iota.test.iris.config.netagent.api as agent_api
import iota.harness.api as api
import yaml

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
    api.Trigger_AddNaplesCommand(req, wl.node_name, "/nic/bin/halctl show session --dstip %s --yaml" % (wl.ip_address))
    tc.cmd_cookies.append(cmd_cookie)
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    cookie_idx = 0
    for cmd in tc.resp.commands:
        #api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
        #api.PrintCommandResults(cmd)
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
    #import pdb; pdb.set_trace()
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


def do_vmotion(tc, wl, new_node):
    vm_threads = []
    api.Logger.info("moving wl {} from node {} to node {}".format(wl.workload_name, wl.node_name, new_node))
    vm_thread = threading.Thread(target=triggerVmotion,args=(tc, wl, new_node, ))
    vm_threads.append(vm_thread)
    vm_thread.start()
    create_ep_info(tc, wl, new_node, "START", wl.node_name)
    # wait for vmotion thread to complete, meaning vmotion is done on vcenter
    for vm_thread in vm_threads:
        vm_thread.join()
    return api.types.status.SUCCESS

def create_ep_info(tc, wl, new_node, migr_state, old_node):
    # get a naples handle to move to
    ep_filter = "meta.name=" + wl.workload_name + ";"
    if not hasattr(tc, 'dsc_conn_type'):
       api.Logger.info(" seeing dsc_conn_type to oob")
       tc.dsc_conn_type = 'oob'  
    objects = agent_api.QueryConfigs("Endpoint", filter=ep_filter)
    assert(len(objects) == 1)
    object                          = copy.deepcopy(objects[0])
    # delete endpoint being moved on new host, TEMP
    agent_api.DeleteConfigObjects([object], [new_node], True)

    object.spec.node_uuid           = tc.uuidMap[new_node]
    object.spec.migration           = migr_state 
    if (api.IsNaplesNode(old_node)):
        object.status.node_uuid         = tc.uuidMap[old_node]
        if (tc.dsc_conn_type == "oob"):
            object.spec.homing_host_address = api.GetNicMgmtIP(old_node)
        else:
            object.spec.homing_host_address = api.GetBondIp(old_node)
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
        object.spec.timeouts.session_idle = "240s"
        object.spec.timeouts.tcp = "300s"
        object.spec.timeouts.udp = "300s"
        object.spec.timeouts.icmp = "120s"
        object.spec.timeouts.tcp_half_close = "120s"
        object.spec.timeouts.tcp_close = "120s"
        object.spec.timeouts.tcp_connection_setup = "60s"
        object.spec.timeouts.tcp_drop = "180s"
        object.spec.timeouts.udp_drop = "60s"
        object.spec.timeouts.icmp_drop = "300s"

    #Now push the update as we modified.
    agent_api.UpdateConfigObjects(store_profile_objects)
    return api.types.status.SUCCESS


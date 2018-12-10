#! /usr/bin/python3

import iota.harness.api as api
import os
from iota.test.iris.testcases.security.conntrack.session_info import *
from iota.test.iris.testcases.security.conntrack.conntrack_utils import *

def Setup(tc):
    api.Logger.info("Setup.")
    
    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Trigger.")
    pairs = api.GetLocalWorkloadPairs()
    resp_flow = getattr(tc.args, "resp_flow", 0)
    tc.cmd_cookies = {}
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    server,client  = pairs[0]
    
    dir_path = os.path.dirname(os.path.realpath(__file__))
    fullpath = dir_path + '/' + "scapy_3way.py"
    resp = api.CopyToWorkload(server.node_name, server.workload_name, [fullpath]) 
    resp = api.CopyToWorkload(client.node_name, client.workload_name, [fullpath]) 
    
    cmd_cookie = start_nc_server(server, "1237")
    add_command(req, tc, 'server', server, cmd_cookie, True)
 
    #start session
    cmd_cookie = "./scapy_3way.py"
    api.Trigger_AddCommand(req, client.node_name, client.workload_name, cmd_cookie,False)
       
    cmd_cookie = "/nic/bin/halctl show session --dstport 1237 --dstip {} --yaml".format(server.ip_address)
    add_command(req, tc, 'show before', client, cmd_cookie, naples=True)
    
    '''  
    iseq_num, iack_num, iwindo_sz, iwinscale, rseq_num, rack_num, rwindo_sz, rwinscale = get_conntrackinfo(cmd)

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    if resp_flow:
        new_seq_num = rseq_num + iwindo_sz * (2 ** iwinscale)
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        #left out of window - retransmit
        cmd_cookie = "hping3 -c 1 -s 1237 -p 52255 -M {}  -L {} --ack --tcp-timestamp {} -d 10".format(wrap_around(rseq_num, 0), rack_num, client.ip_address)    
        add_command(req, tc, 'fail ping', server, cmd_cookie)
    else:
        new_seq_num = iseq_num + rwindo_sz * (2 ** rwinscale)
        #left out of window - retransmit
        cmd_cookie = "hping3 -c 1 -s 52255 -p 1237 -M {}  -L {} --ack --tcp-timestamp {} -d 10".format(wrap_around(iseq_num, 0), iack_num, server.ip_address)   
        add_command(req, tc,"fail ping", client, cmd_cookie)

    cmd_cookie = "sleep 3 && /nic/bin/halctl show session --dstport 1237 --dstip {} --yaml".format(server.ip_address)
    add_command(req, tc, 'show after', client, cmd_cookie, naples=True)

    
    cmd_cookie = "/nic/bin/halctl clear session"
    add_command(req, tc, 'clear', client, cmd_cookie, naples=True)
    '''

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS   
 
def Verify(tc): 
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        '''
        if tc.cmd_cookies['show after'] == cmd.command:     
            if not cmd.stdout:
                #until we can manipulated close timeout to smaller value
                return api.types.status.SUCCESS
            print(cmd.stdout)
            yaml_out = get_yaml(cmd)
            if tc.resp:
                flow = get_respflow(yaml_out)
            else:   
                flow = get_initflow(yaml_out)    
            conn_info = get_conntrack_info(flow)
            excep =  get_exceptions(conn_info)
            if (excep['tcpfullretransmit'] == 'false'):
                return api.types.status.FAILURE 
        '''
    #print(tc.resp)
    return api.types.status.SUCCESS

def Teardown(tc):
    api.Logger.info("Teardown.")
    return api.types.status.SUCCESS

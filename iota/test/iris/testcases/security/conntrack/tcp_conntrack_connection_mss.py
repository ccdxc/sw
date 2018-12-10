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
    first_resp = api.Trigger(req)
    cmd = first_resp.commands[-1]
    api.PrintCommandResults(cmd)
    
    iseq_num, iack_num, iwindo_sz, iwinscale, rseq_num, rack_num, rwindo_sz, rwinscale = get_conntrackinfo(cmd)
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd_cookie = 'tcpdump -nni {} "src host {} and dst host {} and src port 1237"'.format(client.interface, server.ip_address, client.ip_address)
    print(cmd_cookie)
    add_command(req, tc, 'tcpdump', client, cmd_cookie, True)
    if resp_flow:
        new_seq_num = rseq_num + iwindo_sz * (2 ** iwinscale)
        #left out of window - retransmit
        cmd_cookie = "hping3 -c 1 -s 1237 -p 52255 -M {}  -L {} --ack  {} -d 900".format(wrap_around(rseq_num, 0), rack_num, client.ip_address)    
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

    sec_resp = api.Trigger(req)

    term_first_resp = api.Trigger_TerminateAllCommands(first_resp)
    term_sec_resp = api.Trigger_TerminateAllCommands(sec_resp)
   
    tc.resp = api.Trigger_AggregateCommandsResponse(first_resp, sec_resp) 
    tc.resp = api.Trigger_AggregateCommandsResponse(term_first_resp, term_sec_resp)
    tc.resp = term_sec_resp
    return api.types.status.SUCCESS   
 
def Verify(tc):
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if tc.cmd_cookies['tcpdump'] == cmd.command:     
            print(cmd.stdout)
            if cmd.stdout.find('689') != -1:
                return api.types.status.SUCCESS
            else:
                return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Teardown(tc):
    api.Logger.info("Teardown.")
    return api.types.status.SUCCESS

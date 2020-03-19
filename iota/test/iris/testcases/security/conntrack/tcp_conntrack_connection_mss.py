#! /usr/bin/python3

import pdb
import iota.harness.api as api
import os
from iota.test.iris.testcases.security.conntrack.session_info import *
from iota.test.iris.testcases.security.conntrack.conntrack_utils import *
from iota.test.iris.utils import vmotion_utils

def Setup(tc):
    api.Logger.info("Setup.")
    
    if tc.iterators.kind == "remote":
        pairs = api.GetRemoteWorkloadPairs()
        if not pairs:
            api.Logger.info("no remtote eps")
            return api.types.status.SUCCESS
    else:
        pairs = api.GetLocalWorkloadPairs()

    tc.resp_flow = getattr(tc.args, "resp_flow", 0)
    tc.cmd_cookies = {}
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if pairs[0][0].IsNaples():
        tc.client,tc.server = pairs[0]
    else:
        tc.server,tc.client = pairs[0]
    
    dir_path = os.path.dirname(os.path.realpath(__file__))
    fullpath = dir_path + '/' + "scapy_3way.py"
    resp = api.CopyToWorkload(tc.server.node_name, tc.server.workload_name, [fullpath]) 
    resp = api.CopyToWorkload(tc.client.node_name, tc.client.workload_name, [fullpath]) 
    
    cmd_cookie = start_nc_server(tc.server, "1237")
    add_command(req, tc, 'server', tc.server, cmd_cookie, True)
 
    #start session
    cmd_cookie = "./scapy_3way.py"
    api.Trigger_AddCommand(req, tc.client.node_name, tc.client.workload_name, cmd_cookie,False)
       
    cmd_cookie = "/nic/bin/halctl show session --dstport 1237 --dstip {} --yaml".format(tc.server.ip_address)
    add_command(req, tc, 'show before', tc.client, cmd_cookie, naples=True)
    tc.first_resp = api.Trigger(req)
    cmd = tc.first_resp.commands[-1]
    api.PrintCommandResults(cmd)
    
    tc.pre_ctrckinf = get_conntrackinfo(cmd)

    if getattr(tc.args, 'vmotion_enable', False):
        vmotion_utils.PrepareWorkloadVMotion(tc, [tc.server])

    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Trigger.")
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd_cookie = 'tcpdump -nni {} "src host {} and dst host {} and src port 1237"'.format(tc.client.interface, tc.server.ip_address, tc.client.ip_address)
    print(cmd_cookie)
    add_command(req, tc, 'tcpdump', tc.client, cmd_cookie, True)
    if tc.resp_flow:
        new_seq_num = tc.pre_ctrckinf.r_tcpseqnum + tc.pre_ctrckinf.i_tcpwinsz * (2 ** tc.pre_ctrckinf.i_tcpwinscale)
        #left out of window - retransmit
        cmd_cookie = "hping3 -c 1 -s 1237 -p 52255 -M {}  -L {} --ack  {} -d 900".format(
                wrap_around(tc.pre_ctrckinf.r_tcpseqnum, 0), tc.pre_ctrckinf.r_tcpacknum, tc.client.ip_address)    
        add_command(req, tc, 'fail ping', tc.server, cmd_cookie)
    else:
        new_seq_num = tc.pre_ctrckinf.i_tcpseqnum + tc.pre_ctrckinf.r_tcpwinsz * (2 ** tc.pre_ctrckinf.r_tcpwinscale)
        #left out of window - retransmit
        cmd_cookie = "hping3 -c 1 -s 52255 -p 1237 -M {}  -L {} --ack --tcp-timestamp {} -d 10".format(
                wrap_around(tc.pre_ctrckinf.i_tcpseqnum, 0), tc.pre_ctrckinf.i_tcpacknum, tc.server.ip_address)   
        add_command(req, tc,"fail ping", tc.client, cmd_cookie)

    cmd_cookie = "sleep 3 && /nic/bin/halctl show session --dstport 1237 --dstip {} --yaml".format(tc.server.ip_address)
    add_command(req, tc, 'show after', tc.client, cmd_cookie, naples=True)

    cmd_cookie = "/nic/bin/halctl clear session"
    add_command(req, tc, 'clear', tc.client, cmd_cookie, naples=True)
   
    if tc.server.IsNaples(): 
        cmd_cookie = "/nic/bin/halctl clear session"
        add_command(req, tc, 'clear', tc.server, cmd_cookie, naples=True)

    sec_resp = api.Trigger(req)

    term_first_resp = api.Trigger_TerminateAllCommands(tc.first_resp)
    term_sec_resp = api.Trigger_TerminateAllCommands(sec_resp)
   
    tc.resp = api.Trigger_AggregateCommandsResponse(tc.first_resp, sec_resp) 
    tc.resp = api.Trigger_AggregateCommandsResponse(term_first_resp, term_sec_resp)
    tc.resp = term_sec_resp

    return api.types.status.SUCCESS   
 
def Verify(tc):
    if getattr(tc.args, 'vmotion_enable', False):
        vmotion_utils.PrepareWorkloadRestore(tc)

    if tc.resp == None:
        return api.types.status.SUCCESS
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

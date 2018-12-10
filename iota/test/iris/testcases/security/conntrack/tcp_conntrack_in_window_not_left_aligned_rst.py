#! /usr/bin/python3

import iota.harness.api as api
from iota.test.iris.testcases.security.conntrack.session_info import *
from iota.test.iris.testcases.security.conntrack.conntrack_utils import *


def Setup(tc):
    api.Logger.info("Setup.")
    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Trigger.")
    pairs = api.GetLocalWorkloadPairs()
    resp_flow = getattr(tc.args, "resp_flow", 0)
    tc.resp_flow = resp_flow
    tc.cmd_cookies = {}
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    server, client = pairs[0]
    #for w1,w2 in pairs:
    cmd_cookie = start_nc_server(server, "1237")
    add_command(req, tc, 'server', server, cmd_cookie, True) 

    cmd_cookie = start_nc_client(server, "52255", "1237")
    add_command(req, tc, 'client', client, cmd_cookie, True)
       
    cmd_cookie = "/nic/bin/halctl show session --dstport 1237 --dstip {} --yaml".format(server.ip_address)
    add_command(req, tc, 'show before', client, cmd_cookie, naples=True)

    trig_resp1 = api.Trigger(req)
    cmd = trig_resp1.commands[-1] 
    api.PrintCommandResults(cmd)
    iseq_num, iack_num, iwindo_sz, iwinscale, rseq_num, rack_num, rwindo_sz, rwinscale = get_conntrackinfo(cmd)

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    #within the window - outoforder
    if resp_flow:
        cmd_cookie = "hping3 -c 1 -s 1237 -p 52255 -M {}  -L {} --rst --ack --tcp-timestamp {}".format(wrap_around(rseq_num,100), rack_num, client.ip_address)    
        add_command(req, tc, 'fail ping', server, cmd_cookie)
    else:
        cmd_cookie = "hping3 -c 1 -s 52255 -p 1237 -M {}  -L {} --rst --ack --tcp-timestamp {}".format(wrap_around(iseq_num,100), iack_num, server.ip_address)   
        add_command(req, tc, 'fail ping', client, cmd_cookie) 

    cmd_cookie = "sleep 3 && /nic/bin/halctl show session --dstport 1237 --dstip {} --yaml".format(server.ip_address)
    add_command(req, tc, 'show after', client, cmd_cookie, naples=True)


    cmd_cookie = "/nic/bin/halctl clear session"
    add_command(req, tc, 'clear', client, cmd_cookie, naples=True)
    
    trig_resp = api.Trigger(req)
    term_resp1 = api.Trigger_TerminateAllCommands(trig_resp1)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp1)
    
    return api.types.status.SUCCESS    
        
def Verify(tc):
    api.Logger.info("Verify.")
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if tc.cmd_cookies['show after'] == cmd.command:    
            if not cmd.stdout:
                print("none stdout")
                return api.types.status.SUCCESS
            print(cmd.stdout)
            
            yaml_out = get_yaml(cmd)
            if tc.resp_flow:
                flow = get_respflow(yaml_out)
            else:
                flow = get_initflow(yaml_out)
            state = get_tcpstate(flow)
            if int(state) == TcpState.ESTABLISHED:
                return api.types.status.SUCCESS
            else:
                return api.types.status.FAILURE
        
    return api.types.status.SUCCESS

def Teardown(tc):
    api.Logger.info("Teardown.")
    return api.types.status.SUCCESS

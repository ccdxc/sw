#! /usr/bin/python3

import iota.harness.api as api
from iota.test.iris.testcases.security.conntrack.session_info import *
from scapy import *
import pdb

def Setup(tc):
    api.Logger.info("Setup.")
    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Trigger.")
    pairs = api.GetLocalWorkloadPairs()
    tc.cmd_cookies1 = []
    tc.cmd_cookies2 = []
    tc.cmd_cookies3 = []
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    #for w1,w2 in pairs:
    server,client  = pairs[0]
    naples = server
    if not server.IsNaples():
       if not client.IsNaples():
          naples = client
          return api.types.status.SUCCESS
       else:
          client, server = pairs[0]

    #Step 1: Start TCP Server
    api.Trigger_AddCommand(req, server.node_name, server.workload_name, "nc -l 22535", background=True)
    tc.cmd_cookies1.append("start server")

    #Step 2: Start TCP Client
    api.Trigger_AddCommand(req, client.node_name, client.workload_name, 
                        "nc {} 22535 -p 52535".format(server.ip_address), background=True)
    tc.cmd_cookies1.append("start client")

    #Step 3: Get the session out from naples
    api.Trigger_AddNaplesCommand(req, naples.node_name, 
                "/nic/bin/halctl show session --dstport 22535 --dstip {} --yaml".format(server.ip_address))
    trig_resp1 = api.Trigger(req)
    cmd = trig_resp1.commands[-1]
    for command in trig_resp1.commands:
        api.PrintCommandResults(command)
    iseq_num, iack_num, iwindosz, iwinscale, rseq_num, rack_num, rwindo_sz, rwinscale = get_conntrackinfo(cmd)
    tc.cmd_cookies1.append("show session detail")

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    #Step 4: Start TCPDUMP in background at the client
    api.Trigger_AddCommand(req, client.node_name, client.workload_name, "tcpdump -i {} > out.txt".format(client.interface), background=True)
    tc.cmd_cookies2.append("tcpdump client");

    api.Trigger_AddCommand(req, server.node_name, server.workload_name, "tcpdump -i {} > out.txt".format(server.interface), background=True)
    tc.cmd_cookies2.append("tcpdump server");

    #Step 5: Send data on one side
    api.Trigger_AddCommand(req, client.node_name, client.workload_name, 
               "hping3 -c 1 -s 52535 -p 22535 -M {}  -L {} --ack --tcp-timestamp {} -d 10 ".format(iseq_num+1, iack_num, server.ip_address))
    tc.cmd_cookies2.append("Send data on initiator flow")

    #Step 6: Check if session is up in FIN_RCVD state
    api.Trigger_AddNaplesCommand(req, naples.node_name, "/nic/bin/halctl show session --dstport 22535 --dstip {} --srcip {} | grep ESTABLISHED".format(server.ip_address, client.ip_address))
    tc.cmd_cookies2.append("Before timeout");

    #Sleep for connection setup timeout
    #Get it from the config
    timeout = 45 
    cmd_cookie = "sleep"
    api.Trigger_AddNaplesCommand(req, naples.node_name, "sleep %s" % timeout, timeout=300)
    tc.cmd_cookies2.append("sleep")

    #Step 7: Validate if session is exist
    api.Trigger_AddNaplesCommand(req, naples.node_name, "/nic/bin/halctl show session --dstport 22535 --dstip {} --srcip {} | grep ESTABLISHED".format(server.ip_address, client.ip_address))
    tc.cmd_cookies2.append("After session timeout")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    ##Terminate the tcpdump to get the redirected output and grep for the
    ##packet we are looking for
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

     #Step 7: Check TCPDUMP to make sure we sent the packet
    api.Trigger_AddCommand(req, server.node_name, server.workload_name, "grep \".22535 > {}.52535\" out.txt | grep \"\[.\]\" | grep \"length 0\"".format(client.ip_address))
    tc.cmd_cookies3.append("Check sent tickle")

    #Sleep for connection setup timeout
    #Get it from the config
    timeout = 45 
    cmd_cookie = "sleep"
    api.Trigger_AddNaplesCommand(req, naples.node_name, "sleep %s" % timeout, timeout=300)
    tc.cmd_cookies3.append("sleep")

    #Step 6: Check if session is up after tickle
    api.Trigger_AddNaplesCommand(req, naples.node_name, "/nic/bin/halctl show session --dstport 22535 --dstip {} --srcip {} | grep ESTABLISHED".format(server.ip_address, client.ip_address))
    tc.cmd_cookies3.append("After tickle");

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.tcpdump_resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS
        
def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
  
    #Verify Half close timeout & session state
    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
        if tc.cmd_cookies2[cookie_idx].find("After session timeout") != -1:
            if cmd.stdout == -1:
                result = api.types.status.FAILURE
        cookie_idx += 1

    #Verify TCP DUMP responses
    cookie_idx = 0
    for cmd in tc.tcpdump_resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
        if tc.cmd_cookies3[cookie_idx].find("After tickle") != -1:
            if cmd.stdout == -1:
                result = api.types.status.FAILURE
        elif tc.cmd_cookies3[cookie_idx].find("Check sent tickle") != -1:
            if cmd.stdout == -1:
                result = api.types.status.FAILURE
        cookie_idx += 1
    return result        

def Teardown(tc):
    api.Logger.info("Teardown.")
    return api.types.status.SUCCESS

#! /usr/bin/python3

import iota.harness.api as api
from iota.test.iris.testcases.security.conntrack.session_info import *
from iota.test.iris.testcases.aging.aging_utils import *
import pdb

def Setup(tc):
    api.Logger.info("Setup.")
    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Trigger.")
    pairs = api.GetLocalWorkloadPairs(naples=True)
    tc.cmd_cookies1 = []
    tc.cmd_cookies2 = []
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

    #Step 0: Update the timeout in the config object
    update_timeout("tcp-connection-setup", tc.iterators.timeout) 

    #profilereq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    #api.Trigger_AddNaplesCommand(profilereq, naples.node_name, "/nic/bin/halctl show nwsec profile --id 11")
    #profcommandresp = api.Trigger(profilereq)
    #cmd = profcommandresp.commands[-1]
    #for command in profcommandresp.commands:
    #    api.PrintCommandResults(command)
    #timeout = get_haltimeout("tcp-connection-setup", cmd)
    #tc.config_update_fail = 0
    #if (timeout != timetoseconds(tc.iterators.timeout)):
    #    tc.config_update_fail = 1
    timeout = timetoseconds(tc.iterators.timeout)
    
    #Step 1: Start TCP Server
    server_port = api.AllocateTcpPort()
    api.Trigger_AddCommand(req, server.node_name, server.workload_name, "nc -l %s -i 600s"%(server_port), background=True)
    tc.cmd_cookies1.append("start server");

    #Step 2: Start TCPDUMP in background
    api.Trigger_AddCommand(req, server.node_name, server.workload_name, "tcpdump -i {} -nn > out.txt".format(server.interface), background=True)
    tc.cmd_cookies1.append("tcpdump");

    #Step 3: Start Hping with SYN set
    client_port = api.AllocateTcpPort()
    api.Trigger_AddCommand(req, client.node_name, client.workload_name, "hping3 -c 1 -s {} -p {} -S -M 1000 {}".format(client_port, server_port, server.ip_address))
    tc.cmd_cookies1.append("Send SYN");

    #Step 4: Check if session is up in SYN_RCVD state
    api.Trigger_AddNaplesCommand(req, naples.node_name, "/nic/bin/halctl show session --dstport {}  --dstip {} --srcip {} | grep SYN".format(server_port, server.ip_address, client.ip_address))
    tc.cmd_cookies1.append("Before timeout");

    #Sleep for connection setup timeout
    ######TBD -- uncomment this once agent update fix is in!!!
    #timeout = timetoseconds(tc.iterators.timeout)
    cmd_cookie = "sleep"
    api.Trigger_AddNaplesCommand(req, naples.node_name, "sleep %s" % timeout, timeout=300)
    tc.cmd_cookies1.append("sleep")

    #Step 5: Validate if session is gone. Note that we could have session in INIT state in this case as the server would retransmit. 
    #Idea is to make sure we have removed the session that was in SYN state
    api.Trigger_AddNaplesCommand(req, naples.node_name, "/nic/bin/halctl show session --dstport {} --dstip {} --srcip {} | grep SYN".format(server_port, server.ip_address, client.ip_address))
    tc.cmd_cookies1.append("After timeout")

    #Step 6: Send an ACK now from the same port
    api.Trigger_AddCommand(req, client.node_name, client.workload_name, "hping3 -c 1 -s {} -p {} -A -M 1000 {}".format(client_port, server_port, server.ip_address))
    tc.cmd_cookies1.append("Send ACK");
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    #Step 7: Check TCPDUMP on the other side to make sure we dropped the packet
    api.Trigger_AddCommand(req, server.node_name, server.workload_name, "grep \"{} >\" out.txt | grep \"\[\.\]\"".format(client_port, server_port, server.ip_address))
    tc.cmd_cookies2.append("Check ACK Received");

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.tcpdump_resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS
        
def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    #if tc.config_update_fail == 1:
    #    return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            #This is expected so dont set failure for this case
            if (tc.cmd_cookies1[cookie_idx].find("After timeout") != -1) or \
               (tc.cmd_cookies1[cookie_idx].find("Send ACK") != -1) or \
               (tc.cmd_cookies1[cookie_idx].find("Check ACK Received") != -1):
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        elif tc.cmd_cookies1[cookie_idx].find("Before timeout") != -1:
           #Session were not established ?
           if cmd.stdout.find("SYN_ACK_RCVD") == -1:
               result = api.types.status.FAILURE
        elif tc.cmd_cookies1[cookie_idx].find("After timeout") != -1:
           #Check if sessions were aged
           if cmd.stdout != '':
               result = api.types.status.FAILURE
        cookie_idx += 1

    cookie_idx = 0
    for cmd in tc.tcpdump_resp.commands:
        api.PrintCommandResults(cmd)
        if tc.cmd_cookies1[cookie_idx].find("Check ACK Received") != -1:
           if cmd.stdout != '':
               result = api.types.status.FAILURE

    return result        

def Teardown(tc):
    api.Logger.info("Teardown.")
    return api.types.status.SUCCESS

#! /usr/bin/python3

import iota.harness.api as api
from iota.test.iris.testcases.security.conntrack.session_info import *
from iota.test.iris.testcases.aging.aging_utils import *
from scapy import *
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

    #TBD remove this once agent side profile update is implemented
    timeout = get_timeout("tcp-close")

    #Step 0: Update the timeout in the config object
    update_timeout("tcp-close", tc.iterators.timeout)
    
    #profilereq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    #api.Trigger_AddNaplesCommand(profilereq, naples.node_name, "/nic/bin/halctl show nwsec profile --id 11")
    #profcommandresp = api.Trigger(profilereq)
    #cmd = profcommandresp.commands[-1]
    #for command in profcommandresp.commands:
    #    api.PrintCommandResults(command)
    #timeout = get_haltimeout("tcp-close", cmd)
    #tc.config_update_fail = 0
    #if (timeout != timetoseconds(tc.iterators.timeout)):
    #    tc.config_update_fail = 1
    timeout = timetoseconds(tc.iterators.timeout)

    #Step 1: Start TCP Server
    server_port = api.AllocateTcpPort()
    api.Trigger_AddCommand(req, server.node_name, server.workload_name, "nc -l %s"%(server_port), background=True)
    tc.cmd_cookies1.append("start server")

    #Step 2: Start TCP Client
    client_port = api.AllocateTcpPort()
    api.Trigger_AddCommand(req, client.node_name, client.workload_name, 
                        "nc {} {} -p {}".format(server.ip_address, server_port, client_port), background=True)
    tc.cmd_cookies1.append("start client")

    #Step 3: Get the session out from naples
    api.Trigger_AddNaplesCommand(req, naples.node_name, 
                "/nic/bin/halctl show session --dstport {} --dstip {} --yaml".format(server_port, server.ip_address))
    trig_resp1 = api.Trigger(req)
    cmd = trig_resp1.commands[-1]
    for command in trig_resp1.commands:
        api.PrintCommandResults(command)
    tc.ctrckinf = get_conntrackinfo(cmd)
    tc.cmd_cookies1.append("show session detail")

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    #Step 4: Start TCPDUMP in background at the client
    api.Trigger_AddCommand(req, client.node_name, client.workload_name, "tcpdump -i {} > out.txt".format(client.interface), background=True)
    tc.cmd_cookies2.append("tcpdump client");

    api.Trigger_AddCommand(req, server.node_name, server.workload_name, "tcpdump -i {} > out.txt".format(server.interface), background=True)
    tc.cmd_cookies2.append("tcpdump server");

    #Step 5: Cook up a FIN and send
    api.Trigger_AddCommand(req, client.node_name, client.workload_name, 
               "hping3 -c 1 -s {} -p {} -M {}  -L {} --ack --tcp-timestamp {} -d 0 -F".format(client_port, server_port, tc.ctrckinf.i_tcpseqnum+1, 
                                                                                              tc.ctrckinf.i_tcpacknum, server.ip_address))
    tc.cmd_cookies2.append("Send FIN")

    #Step 6: Send FIN ACK now from other side after timeout
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
               "hping3 -c 1 -s {} -p {} -M {}  -L {} --ack --tcp-timestamp {} -d 0 -F".format(server_port, client_port, tc.ctrckinf.r_tcpseqnum+1, 
                                                                                              tc.ctrckinf.r_tcpacknum, client.ip_address))
    tc.cmd_cookies2.append("Send FIN ACK")

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
               "hping3 -c 1 -s {} -p {} -M {}  -L {} --ack --tcp-timestamp {} -d 0".format(server_port, client_port, tc.ctrckinf.i_tcpseqnum, 
                                                                                           tc.ctrckinf.i_tcpacknum+1, server.ip_address))
    tc.cmd_cookies2.append("Send ACK")

    #Step 6: Check if session is up in FIN_RCVD state
    api.Trigger_AddNaplesCommand(req, naples.node_name, "/nic/bin/halctl show session --dstport {} --dstip {} --srcip {} | grep BIDIR_FIN".format(server_port, server.ip_address, client.ip_address))
    tc.cmd_cookies2.append("Before timeout");

    #Sleep for connection setup timeout
    ######TBD -- uncomment this once agent update fix is in!!!
    #timeout = timetoseconds(tc.iterators.timeout)
    cmd_cookie = "sleep"
    api.Trigger_AddNaplesCommand(req, naples.node_name, "sleep %s" % timeout, timeout=300)
    tc.cmd_cookies2.append("sleep")

    #Step 7: Validate if session is gone. Note that we could have session in INIT state in this case as the server would retransmit. 
    #Idea is to make sure we have removed the session that was in FIN_RCVD state
    api.Trigger_AddNaplesCommand(req, naples.node_name, "/nic/bin/halctl show session --dstport {} --dstip {} --srcip {} | grep BIDIR_FIN".format(server_port, server.ip_address, client.ip_address))
    tc.cmd_cookies2.append("After timeout")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    return api.types.status.SUCCESS
        
def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    #if tc.config_update_fail == 1:
    #    return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    #Verify Half close timeout & session state
    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            #This is expected so dont set failure for this case
            if (tc.cmd_cookies2[cookie_idx].find("After timeout") != -1 and cmd.stdout == ''):
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        elif tc.cmd_cookies2[cookie_idx].find("Before timeout") != -1:
           #Session were not established ?
           if cmd.stdout.find("BIDIR_FIN") == -1:
               result = api.types.status.FAILURE
        elif tc.cmd_cookies2[cookie_idx].find("After timeout") != -1:
           #Check if sessions are aged and new session is not created
           if cmd.stdout != '':
               result = api.types.status.FAILURE
        cookie_idx += 1

    return result        

def Teardown(tc):
    api.Logger.info("Teardown.")
    return api.types.status.SUCCESS

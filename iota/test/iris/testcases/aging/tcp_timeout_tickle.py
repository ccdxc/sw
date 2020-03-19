#! /usr/bin/python3

import iota.harness.api as api
from iota.test.iris.testcases.security.conntrack.session_info import *
from scapy import *
from iota.test.iris.testcases.aging.aging_utils import *
import pdb

GRACE_TIME = 5
def Setup(tc):
    api.Logger.info("Setup.")
    update_timeout('tcp-connection-setup', "10s")
    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Trigger.")
    if tc.args.type == 'local_only':
        pairs = api.GetLocalWorkloadPairs()
    else:
        pairs = api.GetRemoteWorkloadPairs()
    tc.cmd_cookies1 = []
    tc.cmd_cookies2 = []
    tc.cmd_cookies3 = []
    req1 = api.Trigger_CreateExecuteCommandsRequest(serial = True)

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
    update_timeout("tcp-timeout", tc.iterators.timeout)

    #profilereq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    #api.Trigger_AddNaplesCommand(profilereq, naples.node_name, "/nic/bin/halctl show nwsec profile --id 11")
    #profcommandresp = api.Trigger(profilereq)
    #cmd = profcommandresp.commands[-1]
    #for command in profcommandresp.commands:
    #    api.PrintCommandResults(command)
    #profcommandtermresp = api.Trigger_TerminateAllCommands(profcommandresp)
    #profcommandaggresp = api.Trigger_AggregateCommandsResponse(profcommandresp, profcommandtermresp)
    #timeout = get_haltimeout("tcp-timeout", cmd)
    #tc.config_update_fail = 0
    #if (timeout != timetoseconds(tc.iterators.timeout)):
    #    tc.config_update_fail = 1

    timeout = timetoseconds(tc.iterators.timeout)

    #Step 1: Start TCPDUMP in background at the client
    #api.Trigger_AddCommand(req1, client.node_name, client.workload_name, "tcpdump -i {} > out.txt".format(client.interface), background=True)
    #tc.cmd_cookies1.append("tcpdump client");

    #api.Trigger_AddCommand(req1, server.node_name, server.workload_name, "tcpdump -i {} > out.txt".format(server.interface), background=True)
    #tc.cmd_cookies1.append("tcpdump server");

    #Step 2: Start TCP Server
    server_port = api.AllocateTcpPort() 
    api.Trigger_AddCommand(req1, server.node_name, server.workload_name, "nc -l %s"%(server_port), background=True)
    tc.cmd_cookies1.append("start server")

    #Step 3: Start TCP Client
    client_port = api.AllocateTcpPort()
    api.Trigger_AddCommand(req1, client.node_name, client.workload_name, 
                        "nc {} {} -p {}".format(server.ip_address, server_port, client_port), background=True)
    tc.cmd_cookies1.append("start client")


    #Step 4: Get the session out from naples
    api.Trigger_AddNaplesCommand(req1, naples.node_name, 
                "/nic/bin/halctl show session --dstport {} --dstip {} --yaml".format(server_port, server.ip_address))
    trig_resp1 = api.Trigger(req1)
    cmd = trig_resp1.commands[-1]
    for command in trig_resp1.commands:
        api.PrintCommandResults(command)
    tc.ctrckinf = get_conntrackinfo(cmd)
    sess_hdl = get_sess_handle(cmd)


    req2 = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    #Keep the connection idle for 15 secs before timeout
    cmd_cookie = "sleep"
    api.Trigger_AddNaplesCommand(req2, naples.node_name, "sleep %s"% ((timeout-15)), timeout=300)
    tc.cmd_cookies3.append("sleep")

    #Step 5: Send data on one side
    api.Trigger_AddCommand(req2, client.node_name, client.workload_name, 
               "hping3 -c 1 -s {} -p {} -M {}  -L {} --ack --tcp-timestamp {} -d 10 ".format(client_port, server_port, tc.ctrckinf.i_tcpseqnum, 
                                                                                             tc.ctrckinf.i_tcpacknum, server.ip_address))
    tc.cmd_cookies2.append("Send data on initiator flow")

    #Step 6: Validate if session is exist
    api.Trigger_AddNaplesCommand(req2, naples.node_name, "/nic/bin/halctl show session --handle {}".format(sess_hdl))
    tc.cmd_cookies2.append("show session")

    #Sleep for 
    ######TBD -- uncomment this once agent update fix is in!!!
    cmd_cookie = "sleep"
    api.Trigger_AddNaplesCommand(req2, naples.node_name, "sleep %s"%(timeout-GRACE_TIME), timeout=300)
    tc.cmd_cookies2.append("sleep")

    #Step 6: Validate if session is exist
    api.Trigger_AddNaplesCommand(req2, naples.node_name, "/nic/bin/halctl show session --handle {}".format(sess_hdl))
    tc.cmd_cookies2.append("After session timeout")

    #Step 7: Validate if tickle was sent
    api.Trigger_AddNaplesCommand(req2, naples.node_name, "/nic/bin/halctl show session --handle {} --yaml".format(sess_hdl))
    tc.cmd_cookies2.append("Validate tickle")

    trig_resp2 = api.Trigger(req2)
    cmd = trig_resp2.commands[-1]
    for command in trig_resp2.commands:
        api.PrintCommandResults(command)
    tc.itickles, tc.iresets, tc.rtickles, tc.rresets = get_tickleinfo(cmd)
    term_resp2 = api.Trigger_TerminateAllCommands(trig_resp2)
    tc.resp2 = api.Trigger_AggregateCommandsResponse(trig_resp2, term_resp2)

    # Terminate client, server & tcpdump
    term_resp1 = api.Trigger_TerminateAllCommands(trig_resp1)
    tc.resp1 = api.Trigger_AggregateCommandsResponse(trig_resp1, term_resp1)
   
    if cmd.stdout != '' and (tc.itickles == 0 and tc.rtickles == 0):
        req3 = api.Trigger_CreateExecuteCommandsRequest(serial = True)

        api.Trigger_AddNaplesCommand(req3, naples.node_name, "sleep 5", timeout=300)
        tc.cmd_cookies3.append("sleep")
       
        api.Trigger_AddNaplesCommand(req3, naples.node_name, "/nic/bin/halctl show session --handle {} --yaml".format(sess_hdl))
        tc.cmd_cookies3.append("Re-validate tickle")

        trig_resp3 = api.Trigger(req3)
        cmd = trig_resp3.commands[-1]
        api.PrintCommandResults(cmd)
        tc.itickles, tc.iresets, tc.rtickles, tc.rresets = get_tickleinfo(cmd)
        term_resp3 = api.Trigger_TerminateAllCommands(trig_resp3)
        tc.tcpdump_resp = api.Trigger_AggregateCommandsResponse(trig_resp3, term_resp3)

    return api.types.status.SUCCESS
        
def Verify(tc):
    if tc.resp1 is None:
        return api.types.status.FAILURE

    #if tc.config_update_fail == 1:
    #    return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    #Verify Half close timeout & session state
 
    cookie_idx = 0
    for cmd in tc.resp1.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE

    for cmd in tc.resp2.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE

    if tc.itickles == 0 and tc.rtickles == 0:
       result = api.types.status.FAILURE

    return result        

def Teardown(tc):
    api.Logger.info("Teardown.")
    return api.types.status.SUCCESS

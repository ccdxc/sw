#! /usr/bin/python3
from iota.test.iris.testcases.alg.dns.dns_utils import *

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    tc.workload_pairs = api.GetLocalWorkloadPairs()
    tc.cmd_cookies = []
    tc.resp = []
    trig_resp = [None]*len(tc.workload_pairs)
    term_resp = [None]*len(tc.workload_pairs)

    #req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                       "/nic/bin/halctl show system memory slab | grep ftp")
    #tc.cmd_cookies.append("Memory stats before")
    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                       "/nic/bin/halctl clear session --alg ftp")
    #tc.cmd_cookies.append("Clear session")
    #trig_resp = api.Trigger(req)
    #term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    #tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    resp = 0
    for pair in tc.workload_pairs:
        server = pair[0]
        client = pair[1]

        naples = server
        if not server.IsNaples():
            naples = client
            if not client.IsNaples():
                continue

        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
        api.Logger.info("Starting DNS test from %s" % (tc.cmd_descr))

        SetupDNSServer(server)

        api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                            "echo \"nameserver %s\" | tee -a /etc/resolv.conf"%(server.ip_address))
        tc.cmd_cookies.append("Setup resolv conf")
 
        api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "nslookup test3.example.com")
        tc.cmd_cookies.append("Query DNS server") 

        # Add Naples command validation
        #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
        #                       "/nic/bin/halctl show session --alg dns | grep UDP")
        #tc.cmd_cookies.append("show session")

        trig_resp[resp] = api.Trigger(req)
        term_resp[resp] = api.Trigger_TerminateAllCommands(trig_resp[resp])
        tc.resp.append(api.Trigger_AggregateCommandsResponse(trig_resp[resp], term_resp[resp]))
        resp += 1 

    #req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                       "/nic/bin/halctl show system memory slab | grep dns")
    #tc.cmd_cookies.append("Memory stats After")
    #trig_resp = api.Trigger(req)
    #term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    #tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    api.Logger.info("Results for %s" % (tc.cmd_descr))
    cookie_idx = 0
    for resp in tc.resp:
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
                 if tc.cmd_cookies[cookie_idx].find("Show session") != -1 and cmd.exit_code == 0:
                     result = api.types.status.SUCCESS
                 else:
                     result = api.types.status.FAILURE
            if tc.cmd_cookies[cookie_idx].find("Show session") != -1 and \
               cmd.stdout != '':
               result = api.types.status.FAILURE
            cookie_idx += 1

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

#! /usr/bin/python3
from iota.test.iris.testcases.alg.dns.dns_utils import *
from iota.test.iris.testcases.alg.alg_utils import *
import re
import os

def Setup(tc):
    update_app('dns', '30s', 'drop_large_domain_packets', 'False')
    update_sgpolicy('dns')
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetLocalWorkloadPairs()
    server = pairs[0][0]
    client = pairs[0][1]
    tc.cmd_cookies = []

    server,client  = pairs[0]
    naples = server
    if not server.IsNaples():
       if not client.IsNaples():
          naples = client
          return api.types.status.SUCCESS
       else:
          client, server = pairs[0]

    SetupDNSClient(client, server, qtype="LARGE_DOMAIN_NAME")

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
    api.Logger.info("Starting DNS test from %s" % (tc.cmd_descr))

    api.Trigger_AddNaplesCommand(req, naples.node_name,
                "/nic/bin/halctl clear session")
    tc.cmd_cookies.append("clear session")

    SetupDNSServer(server)

    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "tcpdump -i {} > out.txt".format(server.interface), background=True)
    tc.cmd_cookies.append("tcpdump")

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "sudo echo \'nameserver %s\' | sudo tee -a /etc/resolv.conf"%(server.ip_address))
    tc.cmd_cookies.append("Setup resolv conf")
 
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "sh -c 'cd dnsdir && chmod +x dnsscapy.py && ./dnsscapy.py'")
    tc.cmd_cookies.append("Query DNS server") 

    ## Add Naples command validation
    api.Trigger_AddNaplesCommand(req, naples.node_name,
                "/nic/bin/halctl show session --alg dns --yaml")
    tc.cmd_cookies.append("Show session")
 
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
 
    GetTcpdumpData(server)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    tcpdump = dir_path + '/' + "out.txt"
    found = False
    for line in open(tcpdump, 'r'):
        if re.search(LARGE_DOMAIN_NAME, line):
            found = True

    os.remove(tcpdump)
    if found == False:
       result = api.types.status.FAILURE

    result = api.types.status.SUCCESS
    api.Logger.info("Results for %s" % (tc.cmd_descr))
    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
        cookie_idx += 1

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

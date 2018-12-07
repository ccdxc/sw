#! /usr/bin/python3
import os
import time
import iota.harness.api as api

def Setup(tc):
    return api.types.status.SUCCESS

def SetupDNSServer(server):
    node = server.node_name
    workload = server.workload_name
    dir_path = os.path.dirname(os.path.realpath(__file__))
    zonefile = dir_path + '/' + "example.com.zone"
    api.Logger.info("fullpath %s" % (zonefile))
    resp = api.CopyToWorkload(node, workload, [zonefile], 'dnsdir')
    if resp is None:
       return None

    named_conf = dir_path + '/' + "named.conf"
    resp = api.CopyToWorkload(node, workload, [named_conf], 'dnsdir')
    if resp is None:
       return None
    
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    
    api.Trigger_AddCommand(req, node, workload,
                           "yes | cp dnsdir/named.conf /etc/")
    api.Trigger_AddCommand(req, node, workload,
                           "ex -s -c \'%s/192.168.100.102/%s/g|x\' /etc/named.conf"%("%s", server.ip_address))
    api.Trigger_AddCommand(req, node, workload,
                           "yes | cp dnsdir/example.com.zone /var/named/")
    api.Trigger_AddCommand(req, node, workload,
                           "systemctl start named")
    api.Trigger_AddCommand(req, node, workload,
                           "systemctl enable named")
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetLocalWorkloadPairs()
    w1 = pairs[0][0]
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

    ## Add Naples command validation
    api.Trigger_AddNaplesCommand(req, naples.node_name,
                "/nic/bin/halctl show session --dstport 53 --dstip {}".format(server.ip_address))
    tc.cmd_cookies.append("Find session")
 
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    api.Logger.info("Results for %s" % (tc.cmd_descr))
    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
             if tc.cmd_cookies[cookie_idx].find("Find session") != -1 and cmd.exit_code == 0:
                 result = api.types.status.SUCCESS
             else:
                 result = api.types.status.FAILURE
        #Add a stricter check for session being gone
        cookie_idx += 1

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

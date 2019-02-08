#! /usr/bin/python3
from iota.test.iris.testcases.alg.dns.dns_utils import *
from iota.test.iris.testcases.alg.alg_utils import *

def Setup(tc):
    update_sgpolicy('dns')
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetLocalWorkloadPairs()
    tc.cmd_cookies = []
    tc.memleak = 0
    server = pairs[0][0]
    client = pairs[0][1]

    naples = server
    if not server.IsNaples():
       naples = client
    if not client.IsNaples():
       return  api.types.status.SUCCESS

    memreq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddNaplesCommand(memreq, naples.node_name,
                              "/nic/bin/halctl clear session")
    api.Trigger_AddNaplesCommand(memreq, naples.node_name,
                      "/nic/bin/halctl show system memory slab --yaml")
    mem_trig_resp = api.Trigger(memreq)
    cmd = mem_trig_resp.commands[-1]
    for command in mem_trig_resp.commands:
        api.PrintCommandResults(command)
    meminfo = get_meminfo(cmd, 'dns')
    for info in meminfo:
       if (info['inuse'] != 0 or info['allocs'] != info['frees']):
           tc.memleak = 1
    mem_term_resp = api.Trigger_TerminateAllCommands(mem_trig_resp)
    mem_resp = api.Trigger_AggregateCommandsResponse(mem_trig_resp, mem_term_resp)

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
    api.Logger.info("Starting DNS test from %s" % (tc.cmd_descr))

    for cnt in range(tc.args.count):
        api.Trigger_AddNaplesCommand(req, naples.node_name,
                               "/nic/bin/halctl clear session")
        tc.cmd_cookies.append("clear session")

        SetupDNSServer(server)

        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

        api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "sudo systemctl start named")
        tc.cmd_cookies.append("Start Named")

        api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "sudo systemctl enable named")
        tc.cmd_cookies.append("Enable Named")

        api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "sudo rm /etc/resolv.conf")
        tc.cmd_cookies.append("Remove resolv.conf")

        api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "sudo echo \'nameserver %s\' | sudo tee -a /etc/resolv.conf"%(server.ip_address))
        tc.cmd_cookies.append("Setup resolv conf")

        api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "nslookup test3.example.com")
        tc.cmd_cookies.append("Query DNS server")


        # Add Naples command validation
        api.Trigger_AddNaplesCommand(req, naples.node_name,
                               "/nic/bin/halctl show session --alg dns")
        tc.cmd_cookies.append("show session")

    api.Trigger_AddNaplesCommand(req, naples.node_name,
                         "/nic/bin/halctl show system memory slab --yaml")
    tc.cmd_cookies.append("show memory slab")

    trig_resp = api.Trigger(req)
    cmd = trig_resp.commands[-1]
    meminfo_after = get_meminfo(cmd, 'dns')
    for idx in range(0, len(meminfo)):
       if (meminfo[idx]['inuse'] != meminfo_after[idx]['inuse'] or \
           meminfo_after[idx]['allocs'] != meminfo_after[idx]['frees']):
           tc.memleak = 1
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    if tc.memleak == 1:
        api.Logger.info("MEMORY LEAK DETECTED")
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    api.Logger.info("Results for %s" % (tc.cmd_descr))
    cookie_idx = 0
    for cmd in tc.resp.commands:
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

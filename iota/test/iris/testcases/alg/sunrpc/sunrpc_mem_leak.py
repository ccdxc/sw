#! /usr/bin/python3
import os
import time
import iota.harness.api as api
from iota.test.iris.testcases.alg.sunrpc.sunrpc_utils import *
from iota.test.iris.testcases.alg.alg_utils import *
import pdb

def Setup(tc):
    update_sgpolicy('sunrpc_tcp')
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetLocalWorkloadPairs()
    server = pairs[0][0]
    client = pairs[0][1]
    tc.cmd_cookies = []
    tc.memleak = 0

    naples = server
    if not server.IsNaples():
       naples = client
       if not client.IsNaples():
          return api.types.status.SUCCESS

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
    api.Logger.info("Starting SUNRPC test from %s" % (tc.cmd_descr))

    memreq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddNaplesCommand(memreq, naples.node_name,
                               "sleep 100", timeout=True)
    api.Trigger_AddNaplesCommand(memreq, naples.node_name,
                              "/nic/bin/halctl clear session")
    api.Trigger_AddNaplesCommand(memreq, naples.node_name,
                      "/nic/bin/halctl show system memory slab --yaml")
    mem_trig_resp = api.Trigger(memreq)
    cmd = mem_trig_resp.commands[-1]
    for command in mem_trig_resp.commands:
        api.PrintCommandResults(command)
    meminfo = get_meminfo(cmd, 'rpc')
    for info in meminfo:
       if (info['inuse'] != 0 or info['allocs'] != info['frees']):
           tc.memleak = 1
    mem_term_resp = api.Trigger_TerminateAllCommands(mem_trig_resp)
    mem_resp = api.Trigger_AggregateCommandsResponse(mem_trig_resp, mem_term_resp)

    for cnt in range(tc.args.count):
        api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                            "rpcinfo -s %s"%(server.ip_address))
        tc.cmd_cookies.append("Run rpcinfo")

        # Add Naples command validation
        api.Trigger_AddNaplesCommand(req, naples.node_name,
                               "/nic/bin/halctl show session --alg sun_rpc")
        tc.cmd_cookies.append("show session")

        api.Trigger_AddNaplesCommand(req, naples.node_name,
                           "/nic/bin/halctl show nwsec flow-gate")
        tc.cmd_cookies.append("show security flow-gate")

    # Get the timeout from the config
    api.Trigger_AddNaplesCommand(req, naples.node_name, "sleep 120", timeout=300)
    tc.cmd_cookies.append("sleep")

    api.Trigger_AddNaplesCommand(req, naples.node_name,
                           "/nic/bin/halctl show nwsec flow-gate | grep SUN_RPC")
    tc.cmd_cookies.append("After flow-gate ageout")

    api.Trigger_AddNaplesCommand(req, naples.node_name,
                               "/nic/bin/halctl show session --alg sun_rpc")
    tc.cmd_cookies.append("show session")

    api.Trigger_AddNaplesCommand(req, naples.node_name,
                         "/nic/bin/halctl show system memory mtrack")
    tc.cmd_cookies.append("show memory mtrack")

    api.Trigger_AddNaplesCommand(req, naples.node_name,
                         "/nic/bin/halctl show system memory slab --yaml")
    tc.cmd_cookies.append("show memory slab")

    trig_resp = api.Trigger(req)
    cmd = trig_resp.commands[-1]
    meminfo_after = get_meminfo(cmd, 'rpc')
    for idx in range(0, len(meminfo)):
       if (meminfo[idx]['inuse'] != meminfo_after[idx]['inuse'] or \
           meminfo_after[idx]['allocs'] != meminfo_after[idx]['frees']):
           tc.memleak = 1

    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    CleanupNFSServer(server, client)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    if tc.memleak == 1:
        for cmd in tc.resp.commands:
            api.PrintCommandResults(cmd)
        api.Logger.info("MEMORY LEAK DETECTED")
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    api.Logger.info("Results for %s" % (tc.cmd_descr))
    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            if (tc.cmd_cookies[cookie_idx].find("Before") != -1 or \
                tc.cmd_cookies[cookie_idx].find("After") != -1 or \
                tc.cmd_cookies[cookie_idx].find("After flow-gate ageout") != -1):
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        if tc.cmd_cookies[cookie_idx].find("show session") != -1 and \
           cmd.stdout == '':
           result = api.types.status.FAILURE
        if tc.cmd_cookies[cookie_idx].find("show security flow-gate") != -1 and \
           cmd.stdout == '':
           result = api.types.status.FAILURE
        if tc.cmd_cookies[cookie_idx].find("After flow-gate ageout") != -1 and \
           cmd.stdout != '':
           result = api.types.status.FAILURE
        cookie_idx += 1

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

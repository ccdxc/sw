#! /usr/bin/python3
import os
import time
import iota.harness.api as api
from iota.test.iris.testcases.alg.alg_utils import *

def Setup(tc):
    update_sgpolicy('rtsp')
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

    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
    api.Logger.info("Starting RTSP test from %s" % (tc.cmd_descr))

    dir_path = os.path.dirname(os.path.realpath(__file__))
    fullpath = dir_path + '/' + "small.vob"
    api.Logger.info("fullpath %s" % (fullpath))
    resp = api.CopyToWorkload(server.node_name, server.workload_name, [fullpath], 'rtspdir')
    if resp is None:
       return api.types.status.FAILURE

    memreq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddNaplesCommand(memreq, naples.node_name,
                              "/nic/bin/halctl clear session")
    api.Trigger_AddNaplesCommand(memreq, naples.node_name,
                      "/nic/bin/halctl show system memory slab --yaml")
    mem_trig_resp = api.Trigger(memreq)
    cmd = mem_trig_resp.commands[-1]
    for command in mem_trig_resp.commands:
        api.PrintCommandResults(command)
    meminfo = get_meminfo(cmd, 'rtsp')
    for info in meminfo:
       if (info['inuse'] != 0 or info['allocs'] != info['frees']):
           tc.memleak = 1
    mem_term_resp = api.Trigger_TerminateAllCommands(mem_trig_resp)
    mem_resp = api.Trigger_AggregateCommandsResponse(mem_trig_resp, mem_term_resp)

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    server_cmd = "cd rtspdir && live555MediaServer"
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           server_cmd, background = True)
    tc.cmd_cookies.append("Run RTSP server")

    for cnt in range(tc.args.count):
        api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "openRTSP rtsp://%s/small.vob" % server.ip_address)
        tc.cmd_cookies.append("Run RTSP client")

        ## Add Naples command validation
        api.Trigger_AddNaplesCommand(req, naples.node_name,
                                "/nic/bin/halctl show session --alg rtsp")
        tc.cmd_cookies.append("show session RTSP established")

        api.Trigger_AddNaplesCommand(req, naples.node_name,
                            "/nic/bin/halctl show nwsec flow-gate | grep RTSP")
        tc.cmd_cookies.append("show flow-gate") 

    api.Trigger_AddNaplesCommand(req, naples.node_name,
                               "sleep 250", timeout=300)
    tc.cmd_cookies.append("sleep")

    api.Trigger_AddNaplesCommand(req, naples.node_name,
                                "/nic/bin/halctl show session --alg rtsp")
    tc.cmd_cookies.append("show session RTSP established")
 
    api.Trigger_AddNaplesCommand(req, naples.node_name,
                         "/nic/bin/halctl show system memory slab --yaml")
    tc.cmd_cookies.append("show memory slab")

    trig_resp = api.Trigger(req)
    cmd = trig_resp.commands[-1]
    meminfo_after = get_meminfo(cmd, 'rtsp')
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
    cookie_idx = 0
    api.Logger.info("Results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            if tc.cmd_cookies[cookie_idx].find("Before") != -1 or \
               tc.cmd_cookies[cookie_idx].find("show flow-gate") != -1:
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        if tc.cmd_cookies[cookie_idx].find("show session") != -1 and \
           cmd.stdout == '':
           result = api.types.status.FAILURE
        cookie_idx += 1       
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

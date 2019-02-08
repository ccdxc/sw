#! /usr/bin/python3
import os
import time
from iota.test.iris.testcases.alg.ftp.ftp_utils import *
from iota.test.iris.testcases.alg.alg_utils import *

def Setup(tc):
    update_sgpolicy('ftp')
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetLocalWorkloadPairs()
    server = pairs[0][0]
    client = pairs[0][1]
    tc.cmd_cookies = []
    tc.controlageout_fail = 0
    naples = server
    if not server.IsNaples():
       naples = client
       if not client.IsNaples():
           return api.types.status.SUCCESS

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
    api.Logger.info("Starting FTP test from %s" % (tc.cmd_descr))

    update_app('ftp', '30s')
    SetupFTPServer(server.node_name, server.workload_name)
    SetupFTPServer(client.node_name, client.workload_name)
    SetupFTPClient(client.node_name, client.workload_name)

    api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
                           "/nic/bin/halctl clear session")
    tc.cmd_cookies.append("clear session")
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "lftp -u admin,linuxpassword %s -e 'repeat -c 20 -d 10 ls; exit'"%(server.ip_address), timeout=300)
    tc.cmd_cookies.append("Run FTP repeat")

    ## Add Naples command validation
    api.Trigger_AddNaplesCommand(req, naples.node_name,
                           "/nic/bin/halctl show session --alg ftp")
    tc.cmd_cookies.append("show session")
    api.Trigger_AddNaplesCommand(req, naples.node_name, 
                           "/nic/bin/halctl show nwsec flow-gate")
    tc.cmd_cookies.append("show flow-gate")
    api.Trigger_AddNaplesCommand(req, naples.node_name, 
                           "/nic/bin/halctl show session --yaml")
    tc.cmd_cookies.append("show yaml")

    trig_resp = api.Trigger(req)
    control_seen = 0
    cmd = trig_resp.commands[-1]
    alginfo = get_alginfo(cmd, APP_SVC_FTP)
    if len(alginfo) == 0: tc.controlageout_fail = 1
    for info in alginfo:
       if info['ftpinfo']['iscontrol'] == True:
          control_seen = 1
    if control_seen == 0: tc.controlageout_fail = 1
 
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    if tc.controlageout_fail == 1:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    api.Logger.info("Results for %s" % (tc.cmd_descr))
    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            if tc.cmd_cookies[cookie_idx].find("show security flow gate") != -1:
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        if ((tc.cmd_cookies[cookie_idx].find("show session") != -1) and \
            (cmd.stdout == '')):
            result = api.types.status.FAILURE
        cookie_idx += 1

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

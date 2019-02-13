#! /usr/bin/python3
import os
import time
import iota.harness.api as api
from iota.test.iris.testcases.alg.alg_utils import *
import pdb

def Setup(tc):
    update_app('rtsp_nonstandard_port', '10s')
    update_sgpolicy('rtsp_nonstandard_port') 
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
    api.Logger.info("Starting RTSP test from %s" % (tc.cmd_descr))

    dir_path = os.path.dirname(os.path.realpath(__file__))
    fullpath = dir_path + '/' + "small.vob"
    api.Logger.info("fullpath %s" % (fullpath))
    resp = api.CopyToWorkload(server.node_name, server.workload_name, [fullpath], 'rtspdir')
    if resp is None:
       return api.types.status.FAILURE

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "ls -al | grep video")
    tc.cmd_cookies.append("Before RTSP")

    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "ip route del 224.0.0.0/4")
    tc.cmd_cookies.append("ip route del server")

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "ip route del 224.0.0.0/4")
    tc.cmd_cookies.append("ip route del client")

    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "ip route add 224.0.0.0/4 dev %s"%(server.interface))
    tc.cmd_cookies.append("ip route server")

    server_cmd = "cd rtspdir && vobStreamer -p 2004 small.vob"
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           server_cmd, background = True, timeout=300)
    tc.cmd_cookies.append("Run RTSP server")
   
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "ip route add 224.0.0.0/4 dev %s"%(client.interface))
    tc.cmd_cookies.append("ip route client")
 
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "openRTSP rtsp://%s:2004/vobStream" % (server.ip_address), timeout=300)
    tc.cmd_cookies.append("Run RTSP client")

    ## Add Naples command validation
    api.Trigger_AddNaplesCommand(req, naples.node_name,
                                "/nic/bin/halctl show session --alg rtsp --yaml")
    tc.cmd_cookies.append("show session RTSP established")
    api.Trigger_AddNaplesCommand(req, naples.node_name,
                              "sleep 30", timeout=300)
    tc.cmd_cookies.append("sleep")
    api.Trigger_AddNaplesCommand(req, naples.node_name,
                           "/nic/bin/halctl show session --alg rtsp --yaml")
    tc.cmd_cookies.append("show yaml")

    trig_resp = api.Trigger(req)
    control_seen = 0 
    data_seen = 0
    cmd = trig_resp.commands[-1]
    api.PrintCommandResults(cmd)
    alginfo = get_alginfo(cmd, APP_SVC_RTSP)
    for info in alginfo:
       if info['rtspinfo']['iscontrol'] == True:
          control_seen = 1
       if info['rtspinfo']['iscontrol'] == False:
          data_seen = 1
    if control_seen == 0 and data_seen == 1: tc.controlageout_fail = 1

    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    if tc.controlageout_fail == 1:
        api.Logger.info("CONTROLE AGED OUT")
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0
    api.Logger.info("Results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            if tc.cmd_cookies[cookie_idx].find("Run RTSP client") != -1 or \
               tc.cmd_cookies[cookie_idx].find("ip route server") != -1:
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        if (tc.cmd_cookies[cookie_idx].find("show session RTSP") != -1 or \
            tc.cmd_cookies[cookie_idx].find("show flow-gate") != -1) and \
            cmd.stdout == '':
            result = api.types.status.FAILURE
        if (tc.cmd_cookies[cookie_idx].find("check age out") != -1 and \
            cmd.stdout != ''):
            result = api.types.status.FAILURE 
        cookie_idx += 1       
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

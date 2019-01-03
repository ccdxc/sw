#! /usr/bin/python3
import os
import time
import iota.harness.api as api
from iota.test.iris.testcases.alg.alg_utils import *
import pdb

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetLocalWorkloadPairs()
    server = pairs[0][0]
    client = pairs[0][1]
    tc.cmd_cookies = []

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
                           "ip route add 224.0.0.0/4 dev %s"%(server.interface))
    tc.cmd_cookies.append("ip route server")

    server_cmd = "cd rtspdir && vobStreamer small.vob"
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           server_cmd, background = True)
    tc.cmd_cookies.append("Run RTSP server")
   
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "ip route add 224.0.0.0/4 dev %s"%(client.interface))
    tc.cmd_cookies.append("ip route client")
 
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "openRTSP rtsp://%s/vobStream" % (server.ip_address))
    tc.cmd_cookies.append("Run RTSP client")

    ## Add Naples command validation
    #api.Trigger_AddNaplesCommand(req, naples.node_name,
    #                            "/nic/bin/halctl show session --alg rtsp | grep ESTABLISHED")
    #tc.cmd_cookies.append("show session RTSP established")
    #api.Trigger_AddNaplesCommand(req, naples.node_name,
    #                        "/nic/bin/halctl show nwsec flow-gate | grep RTSP")
    #tc.cmd_cookies.append("show flow-gate") 

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "ls -al | grep video")
    tc.cmd_cookies.append("After RTSP")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0
    api.Logger.info("Results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            if tc.cmd_cookies[cookie_idx].find("Run RTSP client") != -1:
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        if (tc.cmd_cookies[cookie_idx].find("show session FTP") != -1 or \
            tc.cmd_cookies[cookie_idx].find("show flow-gate") != -1) and \
            cmd.stdout == '':
            result = api.types.status.FAILURE
        cookie_idx += 1       
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

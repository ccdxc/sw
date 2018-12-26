#! /usr/bin/python3
import os
import time
import iota.harness.api as api
from iota.test.iris.testcases.alg.alg_utils import *
import pdb

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    triplet = GetThreeWorkloads()
    server = triplet[0][0]
    client1 = triplet[0][1]
    client2 = triplet[0][2]
    tc.cmd_cookies = []

    naples = server
    if not server.IsNaples():
       naples = client1
       if not client1.IsNaples():
          naples = client2
          if not client2.IsNaples():
             return api.types.status.SUCCESS

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (server.workload_name, server.ip_address, client1.workload_name, client1.ip_address)
    api.Logger.info("Starting RTSP test from %s" % (tc.cmd_descr))

    dir_path = os.path.dirname(os.path.realpath(__file__))
    fullpath = dir_path + '/' + "small.vob"
    api.Logger.info("fullpath %s" % (fullpath))
    resp = api.CopyToWorkload(server.node_name, server.workload_name, [fullpath], 'rtspdir')
    if resp is None:
       return api.types.status.FAILURE

    api.Trigger_AddCommand(req, client1.node_name, client1.workload_name,
                           "ls -al | grep video")
    tc.cmd_cookies.append("Before RTSP")

    server_cmd = "cd rtspdir && vobStreamer -p 2000 small.vob"
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           server_cmd, background = True)
    tc.cmd_cookies.append("Run RTSP server")
   
    api.Trigger_AddCommand(req, client1.node_name, client1.workload_name,
                           "openRTSP rtsp://%s:2000/vobStream" % server.ip_address)
    tc.cmd_cookies.append("Run RTSP client1")

    ## Add Naples command validation
    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                            "/nic/bin/halctl show session --alg rtsp | grep ESTABLISHED")
    #tc.cmd_cookies.append("show session RTSP established")
    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                        "/nic/bin/halctl show nwsec flow-gate | grep RTSP")
    #tc.cmd_cookies.append("show flow-gate") 

    api.Trigger_AddCommand(req, client1.node_name, client1.workload_name,
                           "ls -al | grep video")
    tc.cmd_cookies.append("After RTSP")

    trig_resp = api.Trigger(req)

    dest = trig_resp.commands[2].stdout.find("Transport: RTP/AVP;multicast;destination=")
    dest += 41
    ip = trig_resp.commands[2].stdout[dest:dest+35]
    end = ip.find(";")

    req2 = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddCommand(req2, client2.node_name, client2.workload_name,
                           "ip route add 224.0.0.0/4 dev %s"%(client2.interface), background = True)
    tc.cmd_cookies.append("ip route client2")

    api.Trigger_AddCommand(req2, client2.node_name, client2.workload_name,
                           "sed -i 's/geteuid/getppid/' /usr/bin/vlc && cvlc -vvv rtp://%s:8888 --start-time=00 --run-time=5" % ip[0:end])
    tc.cmd_cookies.append("Run RTP client2")

    trig_resp2 = api.Trigger(req2)
    term_resp2 = api.Trigger_TerminateAllCommands(trig_resp2)
    tc.resp2 = api.Trigger_AggregateCommandsResponse(trig_resp2, term_resp2)

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
            if (tc.cmd_cookies[cookie_idx].find("Run RTSP client") != -1 or \
                tc.cmd_cookies[cookie_idx].find("Before RTSP") != -1):
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        if (tc.cmd_cookies[cookie_idx].find("show session FTP") != -1 or \
            tc.cmd_cookies[cookie_idx].find("show flow-gate") != -1) and \
            cmd.stdout == '':
            result = api.types.status.FAILURE
        cookie_idx += 1       
    for cmd in tc.resp2.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            if tc.cmd_cookies[cookie_idx].find("Run RTP client") != -1:
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

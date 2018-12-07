#! /usr/bin/python3
import os
import time
import iota.harness.api as api

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetLocalWorkloadPairs()
    w1 = pairs[0][0]
    w2 = pairs[0][1]
    tc.cmd_cookies = []

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
    api.Logger.info("Starting RTSP test from %s" % (tc.cmd_descr))

    dir_path = os.path.dirname(os.path.realpath(__file__))
    fullpath = dir_path + '/' + "small.vob"
    api.Logger.info("fullpath %s" % (fullpath))
    resp = api.CopyToWorkload(w1.node_name, w1.workload_name, [fullpath], 'rtspdir')
    if resp is None:
       return api.types.status.FAILURE

    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "ls -al | grep video")
    tc.cmd_cookies.append("Before RTSP")

    server_cmd = "cd rtspdir && live555MediaServer"
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           server_cmd, background = True)
    tc.cmd_cookies.append("Run RTSP server")
    
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "openRTSP rtsp://%s/small.vob" % w1.ip_address)
    tc.cmd_cookies.append("Run RTSP client")
 
    #Add Naples command

    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
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
            if tc.cmd_cookies[cookie_idx].find("Before") != -1:
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        cookie_idx += 1       
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

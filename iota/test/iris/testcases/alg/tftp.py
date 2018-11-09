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

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
    api.Logger.info("Starting TFTP test from %s" % (tc.cmd_descr))

    dir_path = os.path.dirname(os.path.realpath(__file__))
    fullpath = dir_path + '/' + "tftp_file.txt"
    api.Logger.info("fullpath %s" % (fullpath))
    #api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, "mkdir tftpdir")
    resp = api.CopyToWorkload(w1.node_name, w1.workload_name, [fullpath], 'tftpdir')
    if resp is None:
       return api.types.status.FAILURE

    server_cmd = "/bin/tftpy_server.py -i %s -r tftpdir" % (w1.ip_address)

    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           server_cmd, background = True)
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "/bin/tftpy_client.py -H %s -D tftp_file.txt" % w1.ip_address)
 
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "ls -al tftp_file.txt")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    api.Logger.info("Results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

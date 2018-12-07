#! /usr/bin/python3
import os
import time
import iota.harness.api as api

def Setup(tc):
    return api.types.status.SUCCESS

def SetupNFSServer(server, client):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    node = server.node_name
    workload = server.workload_name
    api.Trigger_AddCommand(req, node, workload, "chkconfig nfs on")
    api.Trigger_AddCommand(req, node, workload, "service rpcbind start")
    api.Trigger_AddCommand(req, node, workload, "service nfs start")
    api.Trigger_AddCommand(req, node, workload, "mkdir /home/sunrpcmntdir")
    api.Trigger_AddCommand(req, node, workload, 
        "echo \"/home/sunrpcmntdir %s(rw,sync,no_subtree_check,no_root_squash)\" | tee -a /etc/exports"%(client.ip_address))
    api.Trigger_AddCommand(req, node, workload, "exportfs -ra")
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetLocalWorkloadPairs()
    w1 = pairs[0][0]
    w2 = pairs[0][1]
    tc.cmd_cookies = []

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
    api.Logger.info("Starting SUNRPC test from %s" % (tc.cmd_descr))

    SetupNFSServer(w1, w2)
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "ls -al /home/sunrpcmntdir | grep sunrpc_file.txt")
    tc.cmd_cookies.append("Before rpc")

    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "mkdir -p sunrpcmntdir && mount %s:/home/sunrpcmntdir sunrpcmntdir"%(w1.ip_address))
    tc.cmd_cookies.append("Create mount point")
    
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "echo \"hello world\" | tee -a sunrpcmntdir/sunrpc_file.txt")
    tc.cmd_cookies.append("Create file")

    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "cat /home/sunrpcmntdir/sunrpc_file.txt | grep \"hello world\"")
    tc.cmd_cookies.append("After rpc")

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
            if tc.cmd_cookies[cookie_idx].find("Before") != -1:
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        cookie_idx += 1

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

#! /usr/bin/python3
import os
import time
import iota.harness.api as api
from iota.test.iris.testcases.alg.alg_utils import *
from iota.test.iris.testcases.alg.sunrpc.sunrpc_utils import *
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
    api.Logger.info("Starting SUNRPC test from %s" % (tc.cmd_descr))

    SetupNFSServer(server, client1)

    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "sh -c 'ls -al /home/sunrpcmntdir | grep sunrpc_file.txt'")
    tc.cmd_cookies.append("Before rpc")

    api.Trigger_AddCommand(req, client1.node_name, client1.workload_name,
                           "sh -c 'mkdir -p sunrpcmntdir && mount %s:/home/sunrpcmntdir sunrpcmntdir' "%(server.ip_address))
    tc.cmd_cookies.append("Create mount point")
    
    api.Trigger_AddCommand(req, client1.node_name, client1.workload_name,
                           "sh -c 'echo \'hello world\' | tee -a sunrpcmntdir/sunrpc_file.txt' ")
    tc.cmd_cookies.append("Create file")

    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "sh -c 'cat /home/sunrpcmntdir/sunrpc_file.txt | grep \'hello world\' ' ")
    tc.cmd_cookies.append("After rpc")

    # Add Naples command validation
    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                       "halctl show session --alg sunrpc")
    #tc.cmd_cookies.append("show session")
    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                       "halctl show security flow-gate | grep SUNRPC")
    #tc.cmd_cookies.append("show security flow-gate")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    dport = 0

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Trigger_AddCommand(req, client2.node_name, client2.workload_name,
                           "hping3 -c 1 -s 45535 -p {} -d 0 -S {}".format(dport, "192.168.100.200"))
    tc.cmd_cookies.append("ping unknown host")

    # Get the timeout from the config
    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                       "halctl show session --src %s"%(10.10.10.10))
    #tc.cmd_cookies.append("show session different source")

    api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
                           "sleep 100")
    tc.cmd_cookies.append("sleep")

    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                       "halctl show security flow-gate | grep SUNRPC")
    #tc.cmd_cookies.append("After flow-gate ageout")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    CleanupNFSServer(server, client1)

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
            if tc.cmd_cookies[cookie_idx].find("Before") != -1 or \
               tc.cmd_cookies[cookie_idx].find("ping unknown host") != -1:
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        if tc.cmd_cookies[cookie_idx].find("show session") != -1 and \
           cmd.stdout == '':
           result = api.types.status.FAILURE
        if tc.cmd_cookies[cookie_idx].find("show session different source") != -1 and \
           cmd.stdout != '':
           result = api.types.status.FAILURE
        #if tc.cmd_cookies[cookie_idx].find("show security flow-gate") != -1 and \
        #   cmd.stdout == '':
        #   result = api.types.status.FAILURE
        if tc.cmd_cookies[cookie_idx].find("After flow-gate ageout") != -1 and \
           cmd.stdout != '':
           result = api.types.status.FAILURE
        if tc.cmd_cookies[cookie_idx].find("ping unknown host") != -1 and \
           cmd.exit_code == 0:
           result = api.types.status.FAILURE
        cookie_idx += 1

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

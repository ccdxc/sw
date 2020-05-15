#! /usr/bin/python3
import os
import time
import threading
import iota.harness.api as api
from iota.test.iris.testcases.alg.sunrpc.sunrpc_utils import *
from iota.test.iris.testcases.alg.alg_utils import *
import iota.test.iris.testcases.vmotion.vm_utils as vm_utils 
import pdb

def Setup(tc):
    tc.move_info = []
    tc.server    = ''
    tc.client    = ''
    tc.uuidMap  = api.GetNaplesNodeUuidMap()
    tc.num_moves = int(getattr(tc.args, "num_moves", 1))
    tc.node_to_move    = getattr(tc.args,"move","client")
    tc.local_or_remote = getattr(tc.args,"type","local")
    update_sgpolicy('sunrpc_tcp', True)
    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.local_or_remote == "local":
        pairs = api.GetLocalWorkloadPairs()
    else:
        pairs = api.GetRemoteWorkloadPairs()
    if not len(pairs):
        return api.types.status.FAILURE
    server = pairs[0][0]
    client = pairs[0][1]
    tc.server = server
    tc.client = client
    if tc.node_to_move == "client":
        api.Logger.info("Moving client %s" %(client.workload_name))
        tc.vm_node = client
    else:
        api.Logger.info("Moving server %s" %(client.workload_name))
        tc.vm_node = server

    tc.cmd_cookies = []

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
    api.Logger.info("Starting SUNRPC test from %s" % (tc.cmd_descr))

    # this setups control session for alg-sunrpc
    SetupNFSServer(server, client)

    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "sh -c 'ls -al /home/sunrpcmntdir | grep sunrpc_file.txt'")
    tc.cmd_cookies.append("Before rpc")

    # this setups data session for alg-sunrpc
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "sudo sh -c 'mkdir -p /home/sunrpcdir && mount %s:/home/sunrpcmntdir /home/sunrpcdir' "%(server.ip_address))
    tc.cmd_cookies.append("Create mount point")

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "sudo chmod 777 /home/sunrpcdir")
    tc.cmd_cookies.append("add permission")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)


    #trigger a vmotion at this point
    new_node = vm_utils.find_new_node_to_move_to(tc, tc.vm_node)
    vm_utils.update_move_info(tc,[tc.vm_node],False,new_node)
    vm_utils.do_vmotion(tc, True)

    tc.cmd_cookies = []
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    # make sure a file change in client reflects on server 
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "mv sunrpcdir/sunrpc_file.txt /home/sunrpcdir/")
    tc.cmd_cookies.append("Create file")

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "ls -al /home/sunrpcdir")
    tc.cmd_cookies.append("verify file")

    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "ls -al /home/sunrpcmntdir/")
    tc.cmd_cookies.append("After rpc")

    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "sh -c 'cat /home/sunrpcmntdir/sunrpc_file.txt'")
    tc.cmd_cookies.append("After rpc")

    # Add Naples command validation
    api.Trigger_AddNaplesCommand(req, tc.vm_node.node_name, 
                           "/nic/bin/halctl show session --alg sun_rpc")
    tc.cmd_cookies.append("show session")

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
        if tc.cmd_cookies[cookie_idx].find("show session") != -1 and \
           cmd.stdout == '':
           result = api.types.status.FAILURE
        cookie_idx += 1

    return result

def Teardown(tc):
    if tc.GetStatus() != api.types.status.SUCCESS:
        api.Logger.info("verify failed, returning without teardown")
        return tc.GetStatus()

    CleanupNFSServer(tc.server, tc.client)
    vm_utils.move_back_vms(tc)
    return api.types.status.SUCCESS

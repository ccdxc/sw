#! /usr/bin/python3
import os
import time
import threading
import iota.harness.api as api
from iota.test.iris.testcases.alg.ftp.ftp_utils import *
from iota.test.iris.testcases.alg.alg_utils import *
import iota.test.iris.testcases.vmotion.vm_utils as vm_utils 
import pdb

TCP_TICKLE_GAP = 15
NUM_TICKLES = 5

dir_path = os.path.dirname(os.path.realpath(__file__))

def Setup(tc):
    tc.move_info       = []
    tc.uuidMap         = api.GetNaplesNodeUuidMap()
    tc.num_moves       = int(getattr(tc.args, "num_moves", 1))
    tc.node_to_move    = getattr(tc.args,"","client")
    tc.local_or_remote = getattr(tc.args,"","local")
    update_app('ftp', '120s')
    update_sgpolicy('ftp', True)
    return api.types.status.SUCCESS

def SetupFTPClient(node, workload, server, mode):
    dir_path = os.path.dirname(os.path.realpath(__file__))
    ftpcommand = dir_path + '/' + "ftp.sh" 
    f = open(ftpcommand, "w")
    f.write("#!/bin/bash\n")
    f.write("ftp -inv %s <<EOF\n"%(server.ip_address))
    f.write("user admin linuxpassword\n")
    f.write("cd /home/admin/ftp\n")
    f.write("pwd\n")
    f.write("dir\n")
    if (mode == 'active'):
        f.write("pass\n")
    f.write("ls\n")
    f.write("sleep 60\n")
    f.write("ls\n")
    f.write("sleep 120\n")
    f.write("mget ftp_server.txt\n")
    f.close()

    resp = api.CopyToWorkload(node, workload, [ftpcommand], 'ftpdir')
    if resp is None:
       return None

    return api.types.status.SUCCESS 

def Cleanup(server, client):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "rm -rf ftpdir")
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "rm -rf ftpdir")
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "rm -rf /home/admin/ftp")
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "rm -rf /home/admin/ftp")
    ftpfile = os.path.dirname(os.path.realpath(__file__)) + '/' + "ftp.sh"
    os.remove(ftpfile)
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
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

    if tc.node_to_move == "client":
        tc.vm_node = client
    else:
        tc.vm_node = server

    tc.cmd_cookies = []
    serverReq = None
    clientReq = None

    serverReq = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    clientReq = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
    api.Logger.info("Starting FTP test from %s" % (tc.cmd_descr))

    SetupFTPServer(server.node_name, server.workload_name)
    SetupFTPClient(client.node_name, client.workload_name, server, tc.iterators.mode)

    api.Trigger_AddCommand(serverReq, server.node_name, server.workload_name, "fallocate -l 1G /home/admin/ftp/ftp_server.txt")
    tc.cmd_cookies.append("Before file transfer")
    api.Trigger_AddCommand(clientReq, client.node_name, client.workload_name,
                           "cd ftpdir && chmod +x ftp.sh && ./ftp.sh", timeout=400)
    tc.cmd_cookies.append("Run FTP")

    api.Trigger_AddNaplesCommand(serverReq, tc.vm_node.node_name, "sleep 1", timeout=1)
    tc.cmd_cookies.append("Before Sleep")

    api.Trigger_AddNaplesCommand(serverReq, tc.vm_node.node_name, "/nic/bin/halctl show session --alg ftp")
    tc.cmd_cookies.append("Before vMotion - show session")

    # Trigger the commands
    tc.server_resp = api.Trigger(serverReq)
    tc.client_resp = api.Trigger(clientReq)
    tc.resp        = api.Trigger_AggregateCommandsResponse(tc.server_resp, tc.client_resp)

    # Trigger vMotion
    new_node = vm_utils.find_new_node_to_move_to(tc, tc.vm_node)
    vm_utils.update_move_info(tc,[tc.vm_node],False,new_node)
    vm_utils.do_vmotion(tc, True)

    vm_utils.update_node_info(tc, tc.server_resp)
    vm_utils.update_node_info(tc, tc.client_resp)

    # After vMotion - Wait for Commands to end
    tc.client_resp = api.Trigger_WaitForAllCommands(tc.client_resp)
    api.Trigger_TerminateAllCommands(tc.server_resp)

    # After vMotion - Show sessions dump after vMotion
    tc.cmd_cookies = []
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    ## Add Naples command validation
    api.Trigger_AddNaplesCommand(req, tc.vm_node.node_name, "/nic/bin/halctl show session --alg ftp")
    tc.cmd_cookies.append("show session FTP established")
 
    # Ensure full file is transferred by checking size 
    api.Trigger_AddCommand(req, client.node_name, client.workload_name, "ls -lrth ftpdir/")
    tc.cmd_cookies.append("After get1")
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "sh -c 'du -kh ftpdir/ftp_server.txt | grep 1.0G'")
    tc.cmd_cookies.append("After get2")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp   = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    Cleanup(server, client)

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
                api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
                result = api.types.status.FAILURE
       
        cookie_idx += 1

    return result

def Teardown(tc):
    if tc.GetStatus() != api.types.status.SUCCESS:
        api.Logger.info("verify failed, returning without teardown")
        return tc.GetStatus()

    vm_utils.move_back_vms(tc)
    return api.types.status.SUCCESS

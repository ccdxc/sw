#! /usr/bin/python3
import os
import time
from iota.test.iris.testcases.alg.ftp.ftp_utils import *

def Setup(tc):
    return api.types.status.SUCCESS

def SetupFTPClient(node, workload, server):
    dir_path = os.path.dirname(os.path.realpath(__file__))
    ftpdata = dir_path + '/' + "ftp_client.txt"
    api.Logger.info("fullpath %s" % (ftpdata))
    resp = api.CopyToWorkload(node, workload, [ftpdata], 'ftpdir')
    if resp is None:
       return None

    ftpcommand = dir_path + '/' + "ftp.sh" 
    f = open(ftpcommand, "w")
    f.write("#!/bin/bash\n")
    f.write("ftp -inv %s <<EOF\n"%(server.ip_address))
    f.write("user admin linuxpassword\n")
    f.write("cd /home/admin/ftp\n")
    f.write("pwd\n")
    f.write("dir\n")
    f.write("mget ftp_server.txt\n")
    f.write("mput ftp_client.txt\n")
    f.close()

    resp = api.CopyToWorkload(node, workload, [ftpcommand], 'ftpdir')
    if resp is None:
       return None

    return api.types.status.SUCCESS 

def Cleanup(server, client):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "rm -rf ftpdir/ftp_*")
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "rm -rf ftpdir/ftp_*")
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
    pairs = api.GetLocalWorkloadPairs()
    server = pairs[0][0]
    client = pairs[0][1]
    tc.cmd_cookies = []
    naples = server
    if not server.IsNaples():
       naples = client
       if not client.IsNaples():
          return api.types.status.SUCCESS
       else:
          client, server = pairs[0]

    tc.cmd_descr = "%s(%s) --> %s(%s)" %\
                (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
    api.Logger.info("Starting clear & show ftp stress test from %s" % (tc.cmd_descr))

    #req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                       "/nic/bin/halctl show system memory slab | grep ftp") 
    #tc.cmd_cookies.append("Memory stats before")
    #trig_resp = api.Trigger(req)
    #term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    #tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    for cnt in range(tc.args.count):
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

        SetupFTPServer(server.node_name, server.workload_name)
        SetupFTPClient(client.node_name, client.workload_name, server)

        api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                              "cd ftpdir && chmod +x ftp.sh && ./ftp.sh")
        tc.cmd_cookies.append("Run FTP")

        # Add Naples command validation
        #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
        #                       "/nic/bin/halctl show session --alg ftp | grep ESTABLISHED")
        #tc.cmd_cookies.append("show session")
        #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
        #                       "/nic/bin/halctl clear session --alg ftp")
        #tc.cmd_cookies.append("clear session alg")
 
        trig_resp = api.Trigger(req)
        term_resp = api.Trigger_TerminateAllCommands(trig_resp)
        tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

        Cleanup(server, client)

    #req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                       "/nic/bin/halctl show system memory slab | grep ftp")
    #tc.cmd_cookies.append("Memory stats After")
    #trig_resp = api.Trigger(req)
    #term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    #tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

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
                result = api.types.status.FAILURE
        #Add memory validation
        cookie_idx += 1

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

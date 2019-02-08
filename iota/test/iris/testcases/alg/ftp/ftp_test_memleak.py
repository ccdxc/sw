#! /usr/bin/python3
import os
import time
from iota.test.iris.testcases.alg.ftp.ftp_utils import *
from iota.test.iris.testcases.alg.alg_utils import *
import pdb

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
    pairs = api.GetLocalWorkloadPairs()
    server = pairs[0][0]
    client = pairs[0][1]
    tc.cmd_cookies = []
    tc.memleak = 0
    naples = server
    if not server.IsNaples():
       naples = client
       if not client.IsNaples():
          return api.types.status.SUCCESS
       else:
          client, server = pairs[0]

    update_sgpolicy('ftp')

    tc.cmd_descr = "%s(%s) --> %s(%s)" %\
                (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
    api.Logger.info("Starting ftp memleak test from %s" % (tc.cmd_descr))

    memreq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddNaplesCommand(memreq, naples.node_name,
                              "/nic/bin/halctl clear session")
    api.Trigger_AddNaplesCommand(memreq, naples.node_name,
                      "/nic/bin/halctl show system memory slab --yaml")
    mem_trig_resp = api.Trigger(memreq)
    cmd = mem_trig_resp.commands[-1]
    for command in mem_trig_resp.commands:
        api.PrintCommandResults(command)
    meminfo = get_meminfo(cmd, 'ftp')
    for info in meminfo:
       if (info['inuse'] != 0 or info['allocs'] != info['frees']):
           tc.memleak = 1
    mem_term_resp = api.Trigger_TerminateAllCommands(mem_trig_resp)
    mem_resp = api.Trigger_AggregateCommandsResponse(mem_trig_resp, mem_term_resp)

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    SetupFTPServer(server.node_name, server.workload_name)
    SetupFTPClient(client.node_name, client.workload_name, server)

    for cnt in range(tc.args.count):
        api.Trigger_AddNaplesCommand(req, naples.node_name,
                               "/nic/bin/halctl clear session --alg ftp")
        tc.cmd_cookies.append("clear session alg")

        api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                              "cd ftpdir && chmod +x ftp.sh && ./ftp.sh")
        tc.cmd_cookies.append("Run FTP")

        # Add Naples command validation
        api.Trigger_AddNaplesCommand(req, naples.node_name, 
                               "/nic/bin/halctl show session --alg ftp")
        tc.cmd_cookies.append("show session")


    api.Trigger_AddNaplesCommand(req, naples.node_name,
                               "/nic/bin/halctl clear session --alg ftp")
    tc.cmd_cookies.append("clear session alg")
 
    api.Trigger_AddNaplesCommand(req, naples.node_name,
                         "/nic/bin/halctl show system memory slab --yaml")
    tc.cmd_cookies.append("show memory slab")

    trig_resp = api.Trigger(req)
    cmd = trig_resp.commands[-1]
    meminfo_after = get_meminfo(cmd, 'ftp')
    for idx in range(0, len(meminfo_after)):
       if (meminfo[idx]['inuse'] != meminfo_after[idx]['inuse'] or \
           meminfo_after[idx]['allocs'] != meminfo_after[idx]['frees']):
           tc.memleak = 1
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    Cleanup(server, client)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    if tc.memleak == 1:
       api.Logger.info("Memleak failure detected")
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

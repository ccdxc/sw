#! /usr/bin/python3
import time
from iota.test.iris.testcases.alg.ftp.ftp_utils import *

def Setup(tc):
    return api.types.status.SUCCESS

def SetupFTPClient(node, workload):
    dir_path = os.path.dirname(os.path.realpath(__file__))
    ftpdata = dir_path + '/' + "ftp_client.txt"
    api.Logger.info("fullpath %s" % (ftpdata))
    resp = api.CopyToWorkload(node, workload, [ftpdata], 'ftpdir')
    if resp is None:
       return None

    ftpcommand = dir_path + '/' + ".lftprc" 
    f = open(ftpcommand, "w")
    f.write("set ftp:prefer-epsv true\n")
    f.close()

    resp = api.CopyToWorkload(node, workload, [ftpcommand], '')
    if resp is None:
       return None

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddCommand(req, node, workload, "cp ftpdir/.lftprc ~/")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)

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
    ftpfile = os.path.dirname(os.path.realpath(__file__)) + '/' + ".lftprc"
    os.remove(ftpfile)
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetLocalWorkloadPairs()
    server = pairs[0][0]
    client = pairs[0][1]
    tc.cmd_cookies = []

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
    api.Logger.info("Starting FTP test from %s" % (tc.cmd_descr))

    SetupFTPServer(server.node_name, server.workload_name)
    SetupFTPClient(client.node_name, client.workload_name)

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "cat ftpdir/ftp_server.txt | grep \"I am FTP server\"")
    tc.cmd_cookies.append("Before get on client")

    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "cat /home/admin/ftp_client.txt | grep \"I am FTP client\"")
    tc.cmd_cookies.append("Before put on server")

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "cd ftpdir && lftp -u admin,linuxpassword %s -e 'get ftp/ftp_server.txt; exit'"%(server.ip_address))
    tc.cmd_cookies.append("Run FTP GET")

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "cd ftpdir && lftp -u admin,linuxpassword %s -e 'put ftp_client.txt; exit'"%(server.ip_address))
    tc.cmd_cookies.append("Run FTP PUT")

    ## Add Naples command validation
 
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "cat ftpdir/ftp_server.txt | grep \"I am FTP server\"")
    tc.cmd_cookies.append("After get on client")
    
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "cat /home/admin/ftp_client.txt | grep \"I am FTP client\"")
    tc.cmd_cookies.append("After put on server")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

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
                result = api.types.status.FAILURE
        cookie_idx += 1

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

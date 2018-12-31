#! /usr/bin/python3
import os
import time
import pdb
from iota.test.iris.testcases.alg.ftp.ftp_utils import *

dir_path = os.path.dirname(os.path.realpath(__file__))

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    triplet = GetThreeWorkloads()
    server = triplet[0][0]
    client = triplet[0][1]
    mediator = triplet[0][2]
    tc.cmd_cookies = []

    naples = server
    if not server.IsNaples():
       naples = client
       if not client.IsNaples():
          naples = mediator
          if not mediator.IsNaples():
             return api.types.status.SUCCESS

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Mediator: %s(%s) <--> Client: %s(%s)" %\
                   (server.workload_name, server.ip_address, mediator.workload_name, mediator.ip_address, client.workload_name,
                    client.ip_address)
    api.Logger.info("Starting FTP test from %s" % (tc.cmd_descr))

    SetupFTPServer(server.node_name, server.workload_name)
    SetupFTPServer(client.node_name, client.workload_name)
    SetupFTPClient(client.node_name, client.workload_name)
    SetupFTPMediator(mediator.node_name, mediator.workload_name)

    api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
                           "/nic/bin/halctl clear session")
    tc.cmd_cookies.append("clear session")

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "cat /home/admin/ftp/ftp_server.txt | grep \"I am FTP server\"")
    tc.cmd_cookies.append("Before get on client")

    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "cat /home/admin/ftp/ftp_client.txt | grep \"I am FTP client\"")
    tc.cmd_cookies.append("Before put on server")

    api.Trigger_AddCommand(req, mediator.node_name, mediator.workload_name,
                           "lftp -u admin,linuxpassword -e 'get ftp://admin:linuxpassword@%s/ftp/ftp_server.txt  -o ftp://admin:linuxpassword@%s/ftp/; exit'"%(server.ip_address, client.ip_address))
    tc.cmd_cookies.append("Run FTP GET")

    api.Trigger_AddCommand(req, mediator.node_name, mediator.workload_name,
                           "lftp -u admin,linuxpassword -e 'put ftp://admin:linuxpassword@%s/ftp/ftp_client.txt  -o ftp://admin:linuxpassword@%s/ftp/; exit'"%(client.ip_address, server.ip_address))
    tc.cmd_cookies.append("Run FTP PUT")

    ## Add Naples command validation
    #sleep for tcp timeout
    #api.Trigger_AddCommand(req, naples.node_name, naples.workload_name,
    #                       "halctl show session --alg ftp")
    #tc.cmd_cookies.append("show session")
    #api.Trigger_AddCommand(req, naples.node_name, naples.workload_name,
    #                       "halctl show security flow-gate")
    #tc.cmd_cookies.append("show security flow-gate")
 
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "cat /home/admin/ftp/ftp_server.txt | grep \"I am FTP server\"")
    tc.cmd_cookies.append("After get on client")
    
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "cat /home/admin/ftp/ftp_client.txt | grep \"I am FTP client\"")
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
            if (tc.cmd_cookies[cookie_idx].find("Before") != -1 or \
               tc.cmd_cookies[cookie_idx].find("After") != -1):
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        if ((tc.cmd_cookies[cookie_idx].find("Before") != -1 or
             tc.cmd_cookies[cookie_idx].find("After") != -1) and
             cmd.exit_code == 0):
            result = api.types.status.FAILURE
        #Check if everything is cleaned up
        #if ((tc.cmd_cookies[cookie_idx].find("show security flow gate") != -1) and
        #    (cmd.stdout != '')):
        #    result = api.types.status.FAILURE
        cookie_idx += 1

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

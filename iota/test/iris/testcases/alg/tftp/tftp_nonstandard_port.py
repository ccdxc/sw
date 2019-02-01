#! /usr/bin/python3
import time
from iota.test.iris.testcases.alg.tftp.tftp_utils import *

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    pair = api.GetLocalWorkloadPairs()
    tc.cmd_cookies = []
    server = pair[0][0]
    client = pair[0][1]

    naples = server
    if not server.IsNaples():
       naples = client
       if not client.IsNaples():
          return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    SetupTFTPServer(server, non_standard_port=True)
    SetupTFTPClient(client)

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "sh -c 'cat tftpdir/tftp_server.txt | grep \'I am the server\' '")
    tc.cmd_cookies.append("Before file transfer client")

    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "sh -c 'cat /var/lib/tftpboot/tftp_client.txt | grep \'I am the client\' '")
    tc.cmd_cookies.append("Before file transfer server")

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                          "sh -c 'cd tftpdir && atftp  -p -l tftp_client.txt -r tftp_client.txt %s %s'" % (server.ip_address, TFTP_NONSTD_PORT))
    tc.cmd_cookies.append("TFTP put Server: %s(%s) <--> Client: %s(%s)" %\
                           (server.workload_name, server.ip_address, client.workload_name, client.ip_address))

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                         "sh -c 'cd tftpdir && tftp -v %s -c get tftp_server.txt'" % server.ip_address)
    tc.cmd_cookies.append("TFTP get Server: %s(%s) <--> Client: %s(%s)" %\
                           (server.workload_name, server.ip_address, client.workload_name, client.ip_address))

    ## Add Naples command validation
    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                            "/nic/bin/halctl show session --alg tftp | grep UDP")
    #tc.cmd_cookies.append("show session TFTP established")
    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                        "/nic/bin/halctl show nwsec flow-gate | grep TFTP")
    #tc.cmd_cookies.append("show flow-gate")
 
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                          "sh -c 'cat tftpdir/tftp_server.txt | grep \'I am the server\' ' ")
    tc.cmd_cookies.append("After get")

    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                       "sh -c 'cat /var/lib/tftpboot/tftp_client.txt | grep \'I am the client\' '")
    tc.cmd_cookies.append("After put")
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    Cleanup(server, client)

    return api.types.status.SUCCESS

def Verify(tc):
    result = api.types.status.SUCCESS
    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            if tc.cmd_cookies[cookie_idx].find("Before") != -1:
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        if (tc.cmd_cookies[cookie_idx].find("show session TFTP") != -1 or \
            tc.cmd_cookies[cookie_idx].find("show flow-gate") != -1) and \
            cmd.stdout == '':
            result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

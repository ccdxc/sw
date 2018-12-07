#! /usr/bin/python3
import os
import time
import iota.harness.api as api
import pdb

def Setup(tc):
    return api.types.status.SUCCESS

def SetupTFTPServer(server, dir_path):
    node = server.node_name
    workload = server.workload_name
    fullpath = dir_path + '/' + "tftp_server.txt"
    api.Logger.info("fullpath %s" % (fullpath))
    resp = api.CopyToWorkload(node, workload, [fullpath], 'tftpdir')
    if resp is None:
       return None

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddCommand(req, node, workload,
                           "cp tftpdir/tftp_server.txt /var/lib/tftpboot")  
    api.Trigger_AddCommand(req, node, workload,
                           "touch /var/lib/tftpboot/tftp_client.txt")
    api.Trigger_AddCommand(req, node, workload,
                           "chmod 666 /var/lib/tftpboot/tftp_client.txt")
    api.Trigger_AddCommand(req, node, workload,
                           "systemctl start tftp")
    api.Trigger_AddCommand(req, node, workload,
                           "systemctl enable tftp")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
    return api.types.status.SUCCESS

def SetupTFTPClient(client, dir_path):
    node = client.node_name
    workload = client.workload_name
    fullpath = dir_path + '/' + "tftp_client.txt"
    api.Logger.info("fullpath %s" % (fullpath))
    resp = api.CopyToWorkload(node, workload, [fullpath], 'tftpdir')
    if resp is None:
       return None
    return api.types.status.SUCCESS

def Cleanup(server, client):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "rm -rf /var/lib/tftpboot/*")
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "rm -rf tftpdir")
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "rm -rf tftpdir")
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    return api.types.status.SUCCESS
   
def Trigger(tc):
    tc.workload_pairs = api.GetLocalWorkloadPairs()
    tc.cmd_cookies = []
    tc.resp = []
    trig_resp = [None]*len(tc.workload_pairs)
    term_resp = [None]*len(tc.workload_pairs)

    resp = 0
    for pair in tc.workload_pairs:
       w1 = pair[0]
       w2 = pair[1]

       dir_path = os.path.dirname(os.path.realpath(__file__))
       req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

       SetupTFTPServer(w1, dir_path)
       SetupTFTPClient(w2, dir_path)

       api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                              "cat tftpdir/tftp_server.txt | grep \"I am the server\"")
       tc.cmd_cookies.append("Before file transfer client")

       api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "cat /var/lib/tftpboot/tftp_client.txt | grep \"I am the client\"")
       tc.cmd_cookies.append("Before file transfer server")

       api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "cd tftpdir && tftp -v %s -c put tftp_client.txt" % w1.ip_address)
       tc.cmd_cookies.append("TFTP put Server: %s(%s) <--> Client: %s(%s)" %\
                           (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address))

       api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "cd tftpdir && tftp -v %s -c get tftp_server.txt" % w1.ip_address)
       tc.cmd_cookies.append("TFTP get Server: %s(%s) <--> Client: %s(%s)" %\
                           (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address))

       #Add Naples command validation
 
       api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "cat tftpdir/tftp_server.txt | grep \"I am the server\"")
       tc.cmd_cookies.append("After get")

       api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "cat /var/lib/tftpboot/tftp_client.txt | grep \"I am the client\"")
       tc.cmd_cookies.append("After put")
       trig_resp[resp] = api.Trigger(req)
       term_resp[resp] = api.Trigger_TerminateAllCommands(trig_resp[resp])
       tc.resp.append(api.Trigger_AggregateCommandsResponse(trig_resp[resp], term_resp[resp]))
       Cleanup(w1, w2)
       resp += 1

    return api.types.status.SUCCESS

def Verify(tc):
    result = api.types.status.SUCCESS
    cookie_idx = 0
    for resp in tc.resp:
        for cmd in resp.commands:
            api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
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

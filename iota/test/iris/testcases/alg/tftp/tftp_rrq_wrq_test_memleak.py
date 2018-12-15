#! /usr/bin/python3
import os
from iota.test.iris.testcases.alg.tftp.tftp_utils import *

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    tc.workload_pairs = api.GetLocalWorkloadPairs()
    tc.cmd_cookies = []
    tc.resp = []
    trig_resp = [None]*len(tc.workload_pairs)
    term_resp = [None]*len(tc.workload_pairs)

    #req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                       "/nic/bin/halctl show system memory slab | grep ftp")
    #tc.cmd_cookies.append("Memory stats before")
    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                            "/nic/bin/halctl clear session --alg tftp")
    #tc.cmd_cookies.append("clear session")
    #trig_resp = api.Trigger(req)
    #term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    #tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    resp = 0
    for pair in tc.workload_pairs:
       w1 = pair[0]
       w2 = pair[1]

       naples = w1
       if not w1.IsNaples():
           naples = w2
           if not w2.IsNaples():
               continue

       req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

       SetupTFTPServer(w1)
       SetupTFTPClient(w2)

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

       ## Add Naples command validation
       #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
       #                            "/nic/bin/halctl show session --alg tftp | grep UDP")
       #tc.cmd_cookies.append("show session TFTP established")
       #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
       #                        "/nic/bin/halctl show nwsec flow-gate | grep TFTP")
       #tc.cmd_cookies.append("show flow-gate")
       #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
       #                            "/nic/bin/halctl clear session --alg tftp")
       #tc.cmd_cookies.append("clear session")
 
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

    #req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                       "/nic/bin/halctl show system memory slab | grep ftp")
    #tc.cmd_cookies.append("Memory stats After")
    #trig_resp = api.Trigger(req)
    #term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    #tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

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
            if (tc.cmd_cookies[cookie_idx].find("show session TFTP") != -1 or \
                tc.cmd_cookies[cookie_idx].find("show flow-gate") != -1 or \
                tc.cmd_cookies[cookie_idx].find("clear session") != -1) and \
                cmd.stdout == '':
                result = api.types.status.FAILURE
            #Add memleak validation
            cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

#! /usr/bin/python3
import os
import time
from iota.test.iris.testcases.alg.tftp.tftp_utils import *

def Setup(tc):
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

       naples = w1 
       if not w1.IsNaples():
           naples = w2
           if not w2.IsNaples():
               continue

       req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

       SetupTFTPServer(w1)
       SetupTFTPClient(w2)

       api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "sh -c 'cat /var/lib/tftpboot/tftp_client.txt | sudo grep \'I am the client\' ' ")
       tc.cmd_cookies.append("Before file transfer server")

       api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "sh -c 'cd tftpdir && sudo atftp  -p -l tftp_client.txt -r tftp_client.txt --trace %s'" % w1.ip_address)
       tc.cmd_cookies.append("TFTP put Server: %s(%s) <--> Client: %s(%s)" %\
                           (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address))

       ## Add Naples command validation
       #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
       #                            "/nic/bin/halctl show session --alg tftp | grep UDP")
       #tc.cmd_cookies.append("show session TFTP established")
       #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
       #                        "/nic/bin/halctl show nwsec flow-gate | grep TFTP")
       #tc.cmd_cookies.append("show flow-gate")
 
       api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "sh -c 'cat /var/lib/tftpboot/tftp_client.txt | sudo grep \'I am the client\' ' ")
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
            if ((tc.cmd_cookies[cookie_idx].find("Before") != -1 or \
                tc.cmd_cookies[cookie_idx].find("Before") != -1) and \
                (cmd.exit_code == 0)):
                result = api.types.status.FAILURE
            if ((tc.cmd_cookies[cookie_idx].find("show session TFTP") != -1 or \
                tc.cmd_cookies[cookie_idx].find("show flow-gate") != -1) and \
                (cmd.stdout == '')):
                result = api.types.status.FAILURE
            cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

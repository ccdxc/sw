#! /usr/bin/python3
import time
import iota.harness.api as api
import os
import re
import pdb

dir_path = os.path.dirname(os.path.realpath(__file__))

def Setup(tc):
    if tc.iterators.eptype == 'local':
        tc.workload_pairs = api.GetLocalWorkloadPairs()
    else:
        tc.workload_pairs = api.GetRemoteWorkloadPairs()
        
    return api.types.status.SUCCESS

def Trigger(tc):
    naples_list = []
    tc.cmd_cookies = []
    tc.fin_fail = 0

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    idx = 0
    for pairs in tc.workload_pairs:
        server = pairs[0]
        client = pairs[1]

        naples = server
        if not server.IsNaples():
           naples = client
           if not client.IsNaples():
              continue
         
        found = False
        for info in naples_list:
            if info[0] == naples.node_name:
               found = True
        if found == False:
           naples_list.append((naples.node_name, pairs))

        tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                      (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
        api.Logger.info("Starting Upgrade test from %s" % (tc.cmd_descr))

        #Step 1: Start TCPDUMP
        api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                  "tcpdump -i {} > out.txt".format(client.interface), background=True)
        tc.cmd_cookies.append("tcpdump on client")

        api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                  "tcpdump -i {} > out.txt".format(server.interface), background=True)
        tc.cmd_cookies.append("tcpdump on server")

        #Step 1: Start TCP Server
        server_port = api.AllocateTcpPort()
        api.Trigger_AddCommand(req, server.node_name, server.workload_name, "nc -l %s"%(server_port), background=True)
        tc.cmd_cookies.append("start server")

        #Step 2: Start TCP Client
        client_port = api.AllocateTcpPort()
        api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                       "nc {} {} -p {}".format(server.ip_address, server_port, client_port), background=True)
        tc.cmd_cookies.append("start client")
        idx = idx + 1

    for node in naples_list:
        api.Trigger_AddNaplesCommand(req, node[0],
                        "/nic/bin/halctl show session")
        tc.cmd_cookies.append("show session")
        api.Trigger_AddNaplesCommand(req, node[0], 
                        "/nic/bin/halctl debug test send-fin")
        tc.cmd_cookies.append("Send fin")
        api.Trigger_AddNaplesCommand(req, node[0],
                        "/nic/bin/halctl show session | grep FIN_RCVD")
        tc.cmd_cookies.append("show session FIN RCVD")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    for node in naples_list:
        found = False
        api.CopyFromWorkload(node[1][0].node_name, node[1][0].workload_name,
                             ['out.txt'], dir_path)
        api.Logger.info("Copy from {} {}".format(node[0], node[1][0]))
        tcpout = dir_path + '/out.txt'
        for line in open(tcpout, 'r'):
            if re.search("\[F\.\]", line):
               found = True
               break
        if node[0] == node[1][0].node_name and found == False:
           tc.fin_fail = 1

        os.remove(tcpout)
        found = False
        api.CopyFromWorkload(node[1][1].node_name, node[1][1].workload_name,
                             ['out.txt'], dir_path)
        api.Logger.info("Copy from {} {}".format(node[0], node[1][0]))
        tcpout = dir_path + '/out.txt'
        for line in open(tcpout, 'r'):
            if re.search("\[F\.\]", line):
               found = True
        if node[0] == node[1][1].node_name and found == False:
           tc.fin_fail = 1
        os.remove(tcpout)
 
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    if tc.fin_fail == 1:
        return api.types.status.FAILURE

    cookie_idx = 0
    result = api.types.status.SUCCESS
    api.Logger.info("TCP FIN Results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
        if tc.cmd_cookies[cookie_idx].find("FIN RCVD") != -1 and \
           cmd.stdout == '':
            result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

#! /usr/bin/python3
import os
import time
import iota.harness.api as api
from iota.test.iris.testcases.security.conntrack.session_info import *
from iota.test.iris.testcases.alg.msrpc.msrpc_utils import *
from iota.test.iris.testcases.alg.alg_utils import *
from scapy.all import *
from scapy.utils import rdpcap
from scapy.utils import wrpcap
import pdb

MSRPC_PORT = 135
dir_path = os.path.dirname(os.path.realpath(__file__))

def Setup(tc):
    #update_sgpolicy('msrpc')
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

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
    api.Logger.info("Starting MSRPC test from %s" % (tc.cmd_descr))

    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "tcpdump -nni %s > out.txt"%(server.interface), background=True)
    tc.cmd_cookies.append("tcpdump")

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "tcpdump -nni %s > out.txt"%(client.interface), background=True)
    tc.cmd_cookies.append("tcpdump")

    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "sudo nc -l %s"%(MSRPC_PORT), background=True)
    tc.cmd_cookies.append("netcat start-server")

    client_port = api.AllocateTcpPort()
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "sudo nc %s %s -p %s"%(server.ip_address, MSRPC_PORT, client_port), background=True)
    tc.cmd_cookies.append("netcat start-client")

    msrpcscript = dir_path + '/' + "msrpcscapy.py"
    resp = api.CopyToWorkload(server.node_name, server.workload_name, [msrpcscript], 'msrpcdir')
    if resp is None:
        return api.types.status.SUCCESS

    resp = api.CopyToWorkload(client.node_name, client.workload_name, [msrpcscript], 'msrpcdir')
    if resp is None:
        return api.types.status.SUCCESS

    api.Trigger_AddNaplesCommand(req, naples.node_name,
                                   "/nic/bin/halctl show session --dstport 135 --yaml")
    tc.cmd_cookies.append("show session yaml")
    sesssetup = api.Trigger(req)
    cmd = sesssetup.commands[-1]
    api.PrintCommandResults(cmd)
    tc.pre_ctrckinf = get_conntrackinfo(cmd)

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    msrpcpcap = dir_path + '/' + "msrpc_first_bind.pcap"
    pkts=rdpcap(msrpcpcap)
    clientidx = 0 
    serveridx = 0
    client_ack = tc.pre_ctrckinf.i_tcpacknum
    server_ack = tc.pre_ctrckinf.r_tcpacknum
    filename = None
    for pkt in pkts:
        node = client.node_name
        workload = client.workload_name
        if pkt[IP].src == "172.31.9.1":
           filename = ("msrpcscapy" + "%s" + ".pcap")%(clientidx)
           msrpcscapy = dir_path + '/' + filename
           pkt[Ether].src=client.mac_address
           pkt[IP].src=client.ip_address
           pkt[Ether].dst=server.mac_address
           pkt[IP].dst=server.ip_address 
           pkt[TCP].sport = client_port
           pkt[TCP].dport = 135
           if clientidx == 0:
              client_start_seq = pkt[TCP].seq
              client_start_ack = pkt[TCP].ack
           pkt[TCP].seq = tc.pre_ctrckinf.i_tcpseqnum + (pkt[TCP].seq - client_start_seq)
           pkt[TCP].ack = client_ack
           server_ack =  pkt[TCP].seq + len(pkt[TCP].payload) + 1
           clientidx += 1
        else:
           filename = ("msrpcscapy" + "%s" + ".pcap")%(serveridx)
           msrpcscapy = dir_path + '/' + filename
           pkt[Ether].src=server.mac_address
           pkt[IP].src=server.ip_address
           pkt[Ether].dst=client.mac_address
           pkt[IP].dst=client.ip_address
           node = server.node_name
           workload = server.workload_name
           pkt[TCP].dport = client_port
           pkt[TCP].sport = 135
           if serveridx == 0:
              server_start_seq = pkt[TCP].seq
              server_start_ack = pkt[TCP].ack 
           pkt[TCP].seq = tc.pre_ctrckinf.r_tcpseqnum + (pkt[TCP].seq - server_start_seq)
           pkt[TCP].ack = server_ack
           client_ack = pkt[TCP].seq + len(pkt[TCP].payload) + 1
           serveridx += 1
        del pkt[IP].chksum
        del pkt[TCP].chksum
        a = pkt.show(dump=True)
        print (a) 
        wrpcap(msrpcscapy, pkt)
        resp = api.CopyToWorkload(node, workload, [msrpcscapy], 'msrpcdir')
        if resp is None:
           continue 
        api.Trigger_AddCommand(req, node, workload,
                           "sh -c 'cd msrpcdir && chmod +x msrpcscapy.py && ./msrpcscapy.py %s'"%(filename))
        tc.cmd_cookies.append("running #%s on node %s workload %s"%(filename, node, workload)) 
        os.remove(msrpcscapy)

    api.Trigger_AddNaplesCommand(req, naples.node_name,
                                   "/nic/bin/halctl show session --dstport 135 --yaml")
    tc.cmd_cookies.append("show session yaml")
    firstbind = api.Trigger(req)
    cmd = firstbind.commands[-1]
    api.PrintCommandResults(cmd)
    tc.post_ctrckinf = get_conntrackinfo(cmd)

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    msrpcpcap = dir_path + '/' + "msrpc_second_bind.pcap"
    pkts=rdpcap(msrpcpcap)
    client_ack = tc.post_ctrckinf.i_tcpacknum
    server_ack = tc.post_ctrckinf.r_tcpacknum
    clientidx = 0
    serveridx = 0
    filename = None
    for pkt in pkts:
        node = client.node_name
        workload = client.workload_name
        if pkt[IP].src == "172.31.9.1":
           filename = ("msrpcscapy" + "%s" + ".pcap")%(clientidx)
           msrpcscapy = dir_path + '/' + filename
           pkt[Ether].src=client.mac_address
           pkt[IP].src=client.ip_address
           pkt[Ether].dst=server.mac_address
           pkt[IP].dst=server.ip_address
           pkt[TCP].sport = client_port
           pkt[TCP].dport = 135
           if clientidx == 0:
              client_start_seq = pkt[TCP].seq
              client_start_ack = pkt[TCP].ack
           pkt[TCP].seq = tc.post_ctrckinf.i_tcpseqnum + (pkt[TCP].seq - client_start_seq)
           pkt[TCP].ack = client_ack
           server_ack =  pkt[TCP].seq + len(pkt[TCP].payload) + 1
           clientidx += 1
        else:
           filename = ("msrpcscapy" + "%s" + ".pcap")%(serveridx)
           msrpcscapy = dir_path + '/' + filename
           pkt[Ether].src=server.mac_address
           pkt[IP].src=server.ip_address
           pkt[Ether].dst=client.mac_address
           pkt[IP].dst=client.ip_address
           node = server.node_name
           workload = server.workload_name
           pkt[TCP].dport = client_port
           pkt[TCP].sport = 135
           if serveridx == 0:
               server_start_seq = pkt[TCP].seq
               server_start_ack = pkt[TCP].ack
           pkt[TCP].seq = tc.post_ctrckinf.r_tcpseqnum + (pkt[TCP].seq - server_start_seq)
           pkt[TCP].ack = server_ack
           client_ack = pkt[TCP].seq + len(pkt[TCP].payload) + 1 
           serveridx += 1
        del pkt[IP].chksum
        del pkt[TCP].chksum
        a = pkt.show(dump=True)
        print (a)
        wrpcap(msrpcscapy, pkt)
        resp = api.CopyToWorkload(node, workload, [msrpcscapy], 'msrpcdir')
        if resp is None:
           continue
        api.Trigger_AddCommand(req, node, workload,
                           "sh -c 'cd msrpcdir && chmod +x msrpcscapy.py && ./msrpcscapy.py %s'"%(filename))
        tc.cmd_cookies.append("running #%s on node %s workload %s"%(filename, node, workload))
        os.remove(msrpcscapy)

    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "sudo nc -l 49134", background=True)
    tc.cmd_cookies.append("msrpc start-server")

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "sudo nc %s 49134 -p 59374"%(server.ip_address), background=True)
    tc.cmd_cookies.append("msrpc start-client")

    # Add Naples command validation
    #api.Trigger_AddNaplesCommand(req, naples.node_name,
    #                       "/nic/bin/halctl show security flow-gate | grep MSRPC")
    #tc.cmd_cookies.append("show security flow-gate")

    api.Trigger_AddNaplesCommand(req, naples.node_name,
                           "/nic/bin/halctl show session")
    tc.cmd_cookies.append("show session")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp2 = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    term_resp = api.Trigger_TerminateAllCommands(firstbind)
    tc.resp1 = api.Trigger_AggregateCommandsResponse(firstbind, term_resp)

    term_resp = api.Trigger_TerminateAllCommands(sesssetup)
    tc.resp = api.Trigger_AggregateCommandsResponse(sesssetup, term_resp)

    #GetTcpdumpData(client)
    #GetTcpdumpData(server)

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
        if tc.cmd_cookies[cookie_idx].find("show session") != -1 and \
           cmd.stdout == '':
           result = api.types.status.FAILURE
        #if tc.cmd_cookies[cookie_idx].find("show security flow-gate") != -1 and \
        #   cmd.stdout == '':
        #   result = api.types.status.FAILURE
        cookie_idx += 1

    if tc.resp2 is None:
       return api.types.status.FAILURE

    cookie_idx = 0
    for cmd in tc.resp2.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            if (tc.cmd_cookies[cookie_idx].find("Before") != -1 or \
                tc.cmd_cookies[cookie_idx].find("After") != -1):
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        if tc.cmd_cookies[cookie_idx].find("show session") != -1 and \
           cmd.stdout == '':
           result = api.types.status.FAILURE
        #if tc.cmd_cookies[cookie_idx].find("show security flow-gate") != -1 and \
        #   cmd.stdout == '':
        #   result = api.types.status.FAILURE
        cookie_idx += 1

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

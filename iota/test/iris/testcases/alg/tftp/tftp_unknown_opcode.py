#! /usr/bin/python3
import os
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

    SetupTFTPServer(server)
    SetupTFTPClient(client)

    ## Add Naples command validation
    api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
                                "/nic/bin/halctl clear session")
    tc.cmd_cookies.append("clear session")

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "yum install -y git", timeout=600)
    tc.cmd_cookies.append("Install git")
 
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "sh -c 'git clone https://github.com/secdev/scapy && cd scapy && python3 setup.py install'", timeout=600)
    tc.cmd_cookies.append("Install scapy")

    dir_path = os.path.dirname(os.path.realpath(__file__)) 
    tftpunknownop = dir_path + '/' + "tftp_unknown_op.py"
    f = open(tftpunknownop, "w")
    f.write("#! /usr/bin/python3\n")
    f.write("from scapy.all import *\n")
    f.write("ip=IP(src=\"%s\", dst=\"%s\")\n"%(client.ip_address,server.ip_address))
    f.write("tftppkt=UDP(sport=53433,dport=69)/TFTP(op=10)\n")
    f.write("send(ip/tftppkt)\n")
    f.close()

    resp = api.CopyToWorkload(client.node_name, client.workload_name, [tftpunknownop], 'tftpdir')
    if resp is None:
       return api.types.status.FAILURE 

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "sh -c 'cd tftpdir && chmod +x tftp_unknown_op.py && ./tftp_unknown_op.py'")
    tc.cmd_cookies.append("TFTP Uknown Opcode Server: %s(%s) <--> Client: %s(%s)" %\
                           (server.workload_name, server.ip_address, client.workload_name, client.ip_address))

    ## Add Naples command validation
    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                            "/nic/bin/halctl show session --alg tftp | grep UDP")
    #tc.cmd_cookies.append("show session TFTP")
    #api.Trigger_AddNaplesCommand(req, naples.node_name, naples.workload_name,
    #                        "/nic/bin/halctl show nwsec flow-gate | grep TFTP")
    #tc.cmd_cookies.append("show flow-gate")
 
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    Cleanup(server, client)

    os.remove(tftpunknownop) 
    return api.types.status.SUCCESS

def Verify(tc):
    result = api.types.status.SUCCESS
    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            if tc.cmd_cookies[cookie_idx].find("Install") != -1:
               result = api.types.status.SUCCESS
            else:
               result = api.types.status.FAILURE
        if (tc.cmd_cookies[cookie_idx].find("show session") != -1  or \
            tc.cmd_cookies[cookie_idx].find("show flow-gate") != -1) and \
            cmd.stdout != '':
            result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

#! /usr/bin/python3
import os
from iota.test.iris.testcases.alg.tftp.tftp_utils import *
from iota.test.iris.testcases.alg.alg_utils import *
import pdb

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    pair = api.GetLocalWorkloadPairs()
    tc.cmd_cookies = []
    server = pair[0][0]
    client = pair[0][1]
    tc.parseerror_fail = 0
    alginfo = []

    naples = server
    if not server.IsNaples():
        naples = client
        if not client.IsNaples():
            return api.types.status.FAILURE 

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    update_app('tftp', '45s')
    update_sgpolicy('tftp')
    SetupTFTPServer(server)
    SetupTFTPClient(client)

    ## Add Naples command validation
    api.Trigger_AddNaplesCommand(req, naples.node_name,
                                "/nic/bin/halctl clear session")
    tc.cmd_cookies.append("clear session")

    dir_path = os.path.dirname(os.path.realpath(__file__)) 
    tftpunknownop = dir_path + '/' + "tftp_unknown_op.py"
    f = open(tftpunknownop, "w")
    f.write("#! /usr/bin/python3\n")
    f.write("from scapy.all import *\n")
    f.write("ip=IP(src=\"%s\", dst=\"%s\")\n"%(client.ip_address,server.ip_address))
    f.write("tftppkt=UDP(sport=53433,dport=69)/TFTP(op=10)\n")
    f.write("sendp(Ether(src=\"%s\", dst=\"%s\")/ip/tftppkt, iface=\"%s\")\n"%(client.mac_address, server.mac_address, client.interface))
    f.close()

    resp = api.CopyToWorkload(client.node_name, client.workload_name, [tftpunknownop], 'tftpdir')
    if resp is None:
       return api.types.status.FAILURE 

    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "sh -c 'cd tftpdir && chmod +x tftp_unknown_op.py && ./tftp_unknown_op.py'")
    tc.cmd_cookies.append("TFTP Uknown Opcode Server: %s(%s) <--> Client: %s(%s)" %\
                           (server.workload_name, server.ip_address, client.workload_name, client.ip_address))

    ## Add Naples command validation
    api.Trigger_AddNaplesCommand(req, naples.node_name,
                                "/nic/bin/halctl show session --alg tftp --yaml")
    tc.cmd_cookies.append("show session TFTP")
 
    trig_resp = api.Trigger(req)
    cmd = trig_resp.commands[-1]
    alginfo = get_alginfo(cmd, APP_SVC_TFTP)
    api.PrintCommandResults(cmd)
    if len(alginfo) == 0: tc.parseerror_fail = 1
    for info in alginfo:
        if info['tftpinfo']['iscontrol'] == 'true' and info['tftpinfo']['parserror'] == 0:
            tc.parseerror_fail = 1 
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    Cleanup(server, client)

    os.remove(tftpunknownop) 
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.parseerror_fail == 1:
       api.Logger.info("Parse error failure detected") 
       return api.types.status.FAILURE

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
        if (tc.cmd_cookies[cookie_idx].find("show flow-gate") != -1) and \
           cmd.stdout != '':
           result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

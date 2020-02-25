#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
from iota.test.iris.testcases.aging.aging_utils import *
import pdb

def Setup(tc):
    if tc.args.type == 'local_only':
        tc.workload_pairs = api.GetLocalWorkloadPairs()
    else:
        tc.workload_pairs = api.GetRemoteWorkloadPairs()
    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_cookies = []

    for pair in tc.workload_pairs:
        server = pair[0]
        client = pair[1]
        naples = server
        if not server.IsNaples():
            naples = client
            if not client.IsNaples():
               continue

        cmd_cookie = "%s(%s) --> %s(%s)" %\
                     (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
        api.Logger.info("Starting UDP Single flow test from %s" % (cmd_cookie))

        timeout = get_timeout('udp-timeout')
        server_port = api.AllocateUdpPort()
        client_port = api.AllocateUdpPort()

        for idx in range(0, 5):
            api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                               "sudo hping3 -c 1 -s %s -p %s --udp %s -d 10" % (client_port, server_port, server.ip_address))
            tc.cmd_cookies.append("Send data from server to client")
 
        if server.IsNaples():
            cmd_cookie = "Before aging show session"
            api.Trigger_AddNaplesCommand(req, server.node_name, "/nic/bin/halctl show session --dstport {} --dstip {} --srcip {} | grep UDP".format(server_port, server.ip_address, client.ip_address))
            tc.cmd_cookies.append(cmd_cookie)         

            #Get it from the config
            cmd_cookie = "sleep"
            api.Trigger_AddNaplesCommand(req, server.node_name, "sleep %s"%(int(timeout)%5), timeout=300)
            tc.cmd_cookies.append(cmd_cookie)
        
            cmd_cookie = "After aging show session"
            api.Trigger_AddNaplesCommand(req, server.node_name, "/nic/bin/halctl show session --dstport {} --dstip {} --srcip {} | grep UDP".format(server_port, server.ip_address, client.ip_address))
            tc.cmd_cookies.append(cmd_cookie)

    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0

    for cmd in tc.resp.commands:
        api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0: 
            #This is expected so dont set failure for this case
            if tc.cmd_cookies[cookie_idx].find("After aging") != -1 or \
               tc.cmd_cookies[cookie_idx].find("Send data") != -1:
               result = api.types.status.SUCCESS
            else:
               result = api.types.status.FAILURE
        if tc.cmd_cookies[cookie_idx].find("clear session") != -1 and \
           cmd.stdout != '':
           result = api.types.status.FAILURE
        if tc.cmd_cookies[cookie_idx].find("Before aging") != -1 and \
           cmd.stdout == '':
           result = api.types.status.FAILURE
        if tc.cmd_cookies[cookie_idx].find("After aging") != -1 and \
           cmd.stdout == '':
           result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

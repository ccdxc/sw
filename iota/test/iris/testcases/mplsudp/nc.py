#! /usr/bin/python3
import pdb
import time
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.penctl.common as common
import iota.test.iris.config.mplsudp.tunnel as tunnel

def Setup(tc):
    tc.tunnels = tunnel.GetTunnels()
    return api.types.status.SUCCESS

def Trigger(tc):
    #Run all commands in serial
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_cookies = []
    count = 0
    for tunnel in tc.tunnels:
        w1 = tunnel.ltep
        w2 = tunnel.rtep

        cmd_cookie = "%s(%s) --> %s(%s)" %\
                     (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)

        if count % 2 == 0:
            # TCP test
            api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                                "nc -l %s 2000" % w2.ip_address, background=True)
            api.Logger.info("Listen on IP %s and TCP port 2000" % w2.ip_address)

            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                                "echo \"Pensando Systems\" | nc %s 2000" % w2.ip_address)
            api.Logger.info("Send TCP traffic from %s" % (cmd_cookie))
            tc.cmd_cookies.append("TCP test " + cmd_cookie)

        if count % 2 == 1:
            # UDP test
            api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                                "nc -l -u %s 2000" % w2.ip_address, background=True)
            api.Logger.info("Listen on IP %s and UDP port 2000" % w2.ip_address)

            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                                "echo \"Pensando Systems\" | nc -u %s 2000" % w2.ip_address)
            api.Logger.info("Send UDP traffic from %s" % (cmd_cookie))
            tc.cmd_cookies.append("UDP test " + cmd_cookie)

        count += 1
        if count == 16:
            break

    trig_resp = api.Trigger(req)
    time.sleep(10)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0

    for cmd in tc.resp.commands:
        if 'nc -l' in cmd.command:
            api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
            api.PrintCommandResults(cmd)
            if 'Pensando Systems' in cmd.stdout:
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

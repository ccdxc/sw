#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.config.mplsudp.tunnel as tunnel

class IperfTestContext:
    def __init__(self):
        self.req = None
        self.resp = None
        self.cmd_cookies = None

def Setup(tc):
    tc.tunnels = tunnel.GetTunnels()
    return api.types.status.SUCCESS

def Trigger(tc):
    tc.contexts = []
    ctxt = IperfTestContext()
    ctxt.req = api.Trigger_CreateAllParallelCommandsRequest()
    ctxt.cmd_cookies = []
    for tunnel in tc.tunnels:
        w1 = tunnel.ltep
        w2 = tunnel.rtep

        cmd_cookie = "Server: %s(%s) <--> Client: %s(%s)" %\
                       (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
        api.Logger.info("Starting Iperf test from %s" % (cmd_cookie))

        basecmd = 'iperf -p %d ' % api.AllocateTcpPort()
        if tc.iterators.proto == 'udp':
            basecmd = 'iperf -u -p %d ' % api.AllocateUdpPort()
        api.Trigger_AddCommand(ctxt.req, w1.node_name, w1.workload_name,
                               "%s -s -t 300" % basecmd, background = True)
        api.Trigger_AddCommand(ctxt.req, w2.node_name, w2.workload_name,
                               "%s -c %s" % (basecmd, w1.ip_address))

        ctxt.cmd_cookies.append(cmd_cookie)
        ctxt.cmd_cookies.append(cmd_cookie)
    trig_resp = api.Trigger(ctxt.req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    ctxt.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    tc.context = ctxt

    return api.types.status.SUCCESS

def Verify(tc):
    result = api.types.status.SUCCESS

    cookie_idx = 0
    if tc.context.resp is None:
        return api.types.status.FAILURE

    for cmd in tc.context.resp.commands:
        api.Logger.info("Iperf Results for %s" % (tc.context.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
        cookie_idx += 1

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

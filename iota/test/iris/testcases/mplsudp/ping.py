#! /usr/bin/python3
import pdb
import ipaddress
import iota.harness.api as api
import iota.test.iris.config.mplsudp.tunnel as tunnel

class PingTestContext:
    def __init__(self):
        self.req = None
        self.resp = None
        self.cmd_cookie = None

def Setup(tc):
    tc.tunnels = tunnel.GetTunnels()
    tc.resps = []
    return api.types.status.SUCCESS

def Trigger(tc):
    tc.context = None
    ctxt = PingTestContext()
    ctxt.req = api.Trigger_CreateAllParallelCommandsRequest()
    ctxt.cmd_cookies = []
    for tunnel in tc.tunnels:
        w1 = tunnel.ltep
        w2 = tunnel.rtep

        for sec_ipaddr in w1.sec_ip_addresses:
            if ipaddress.IPv4Address(sec_ipaddr).is_reserved:
                api.Logger.info("Workload1: ClassE IP = %s" % sec_ipaddr)
        for sec_ipaddr in w2.sec_ip_addresses:
            if ipaddress.IPv4Address(sec_ipaddr).is_reserved:
                api.Logger.info("Workload2: ClassE IP = %s" % sec_ipaddr)

        cmd_cookie = "%s(%s) --> %s(%s)" %\
                     (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
        api.Trigger_AddCommand(ctxt.req, w1.node_name, w1.workload_name,
                               "ping -i 0.2 -c 20 -s %d %s" % (64, w2.ip_address))
        
        api.Logger.info("Ping test from %s" % (cmd_cookie))
        ctxt.cmd_cookies.append(cmd_cookie)
    ctxt.resp = api.Trigger(ctxt.req)
    tc.context = ctxt

    return api.types.status.SUCCESS

def Verify(tc):

    result = api.types.status.SUCCESS

    cookie_idx = 0
    if tc.context.resp is None:
        return api.types.status.FAILURE

    for cmd in tc.context.resp.commands:
        api.Logger.info("Ping Results for %s" % (tc.context.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
           result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

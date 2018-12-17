#! /usr/bin/python3
import time
import iota.harness.api as api

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetRemoteWorkloadPairs()
    w1 = pairs[0][0]
    w2 = pairs[0][1]

    group = "239.1.1.1"
    maddr = "01:00:5e:01:01:01"

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" % \
                   (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
    api.Logger.info("Starting Multicast inbound Iperf test from %s" % (tc.cmd_descr))

    basecmd = "ip maddress add %s dev %s" % (maddr, w1.interface)
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           basecmd, background = True)
    basecmd = "ip maddress add %s dev %s" % (maddr, w2.interface)
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           basecmd, background = True)
    basecmd = "ip route add %s/32 dev %s" % (group, w1.interface)
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           basecmd, background = True)
    basecmd = "ip route add %s/32 dev %s" % (group, w2.interface)
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           basecmd, background = True)

    basecmd = 'iperf -p %d ' % api.AllocateTcpPort()
    if tc.iterators.proto == 'udp':
        basecmd = 'iperf -u -p %d ' % api.AllocateUdpPort()
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "%s -s -t 300 -B %s -i 1" % (basecmd, group), background = True)
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "%s -c %s -T 32 -t 3 -i 1" % (basecmd, group))

    basecmd = "ip maddress del %s dev %s" % (maddr, w1.interface)
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           basecmd, background = True)
    basecmd = "ip maddress del %s dev %s" % (maddr, w2.interface)
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           basecmd, background = True)
    basecmd = "ip route del %s/32 dev %s" % (group, w1.interface)
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           basecmd, background = True)
    basecmd = "ip route del %s/32 dev %s" % (group, w2.interface)
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           basecmd, background = True)

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    api.Logger.info("Multicast inbound Iperf Results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

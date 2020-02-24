#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.testcases.rdma.rdma_perftest as rdma_perf
import iota.test.iris.utils.naples_host as host

def Setup(tc):
    rdma_perf.Setup(tc)
    return api.types.status.SUCCESS


def Trigger(tc):

    #==============================================================
    # trigger the commands
    #==============================================================
    krping_req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    dmesg_req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    ping_req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    # load krping on both nodes
    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])
    for n in tc.nodes:
        if tc.os == host.OS_TYPE_LINUX:
            api.Trigger_AddHostCommand(krping_req, n,
                    "(lsmod | grep -w rdma_krping >/dev/null) || " +
                    "insmod {path}/krping/rdma_krping.ko"
                    .format(path=tc.iota_path))
        else:
            api.Trigger_AddHostCommand(krping_req, n,
                    "(kldstat | grep -w krping >/dev/null) || " +
                    "kldload {path}/krping/krping.ko"
                    .format(path=tc.iota_path))

    w1 = tc.w[0]
    w2 = tc.w[1]

    options = "port=9999,verbose,validate,size=65536,"

    # cmd for server
    if api.GetNodeOs(w1.node_name) == host.OS_TYPE_LINUX:
        krpfile = "/proc/krping"
    else:
        krpfile = "/dev/krping"

    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" % \
                   (w1.workload_name, w1.ip_address,      \
                    w2.workload_name, w2.ip_address)

    api.Logger.info("Starting krping_rdma test from %s" % (tc.cmd_descr))

    # If the client fails to connect or otherwise misbehaves, IOTA
    # can get stuck with a server waiting forever. This causes
    # module unload to fail and cascading failures after that.
    # Tell the server to only wait this long before giving up.
    server_options = options + "wait=30,"

    cmd = "sudo echo -n 'server,addr={addr},{opstr}' > {kfile}".format(
              addr = w1.ip_address,
              opstr = server_options,
              kfile = krpfile)
    api.Trigger_AddCommand(krping_req, w1.node_name, w1.workload_name, cmd,
                           background = True)

    # It takes a few seconds before the server starts listening.
    cmd = 'sleep 3'
    api.Trigger_AddCommand(krping_req, w1.node_name, w1.workload_name, cmd)

    cmd = "sudo echo -n 'client,addr={addr},{opstr}' > {kfile}".format(
              addr = w1.ip_address,
              opstr = options,
              kfile = krpfile)
    api.Trigger_AddCommand(krping_req, w2.node_name, w2.workload_name, cmd,
                           background = True)

    krping_trig_resp = api.Trigger(krping_req)

    #Run RDMA while krping is running
    rdma_perf.Trigger(tc)
    
    krping_term_resp = api.Trigger_TerminateAllCommands(krping_trig_resp)
    tc.krping_resp = api.Trigger_AggregateCommandsResponse(krping_trig_resp, krping_term_resp)

    # dmesg commands
    api.Trigger_AddCommand(dmesg_req, w1.node_name, w1.workload_name, "dmesg | tail -20 | grep rdma-ping-")
    api.Trigger_AddCommand(dmesg_req, w2.node_name, w2.workload_name, "dmesg | tail -20 | grep rdma-ping-")

    dmesg_trig_resp = api.Trigger(dmesg_req)
    dmesg_term_resp = api.Trigger_TerminateAllCommands(dmesg_trig_resp)
    tc.dmesg_resp = api.Trigger_AggregateCommandsResponse(dmesg_trig_resp, dmesg_term_resp)

    # ping to check if the pipeline is stuck
    api.Trigger_AddCommand(ping_req, w1.node_name, w1.workload_name, "ping -c 10 -t 10 " + w2.ip_address)
    api.Trigger_AddCommand(ping_req, w2.node_name, w2.workload_name, "ping -c 10 -t 10 " + w1.ip_address)

    ping_trig_resp = api.Trigger(ping_req)
    ping_term_resp = api.Trigger_TerminateAllCommands(ping_trig_resp)
    tc.ping_resp = api.Trigger_AggregateCommandsResponse(ping_trig_resp, ping_term_resp)
    return api.types.status.SUCCESS

def Verify(tc):
    
    result = api.types.status.SUCCESS
    
    #print krping output
    for cmd in tc.krping_resp.commands:
        api.PrintCommandResults(cmd)
    
    #print dmesg output
    for cmd in tc.dmesg_resp.commands:
        api.PrintCommandResults(cmd)

    #verify rdma perf test
    result = rdma_perf.Verify(tc)
    if result != api.types.status.SUCCESS:
        return result
    
    #print ping output and check if pipline is stuck
    for cmd in tc.ping_resp.commands:
        api.PrintCommandResults(cmd)
        ping_output = cmd.stdout.split("\n")
        
        if "round-trip" in ping_output[-2]:
            ping_result = ping_output[-3].split()
        else:
            ping_result = ping_output[-2].split()
            
        pkts_sent = int(ping_result[0])
        pkts_rcvd = int(ping_result[3])
        if pkts_sent != 10 or pkts_rcvd != 10:
            api.Logger.error("PING not successful")
            return api.types.status.FAILURE

    return result


def Teardown(tc):
    term_req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    # unload krping on both nodes
    for n in tc.nodes:
        if tc.os == host.OS_TYPE_LINUX:
            api.Trigger_AddHostCommand(term_req, n, "rmmod rdma_krping")
        else:
            api.Trigger_AddHostCommand(term_req, n, "kldunload krping")

    term_resp = api.Trigger(term_req)
    return api.types.status.SUCCESS

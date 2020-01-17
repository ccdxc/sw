#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.naples_host as host
import time

def Setup(tc):

    tc.desc = '''
    Test:   ibv_rc_pingpong
    Opcode: N/A
    Num QP: 1
    modes:  workload1 as server, workload2 as client
            workload2 as server, workload1 as client
    '''

    tc.iota_path = api.GetTestsuiteAttr("driver_path")

    pairs = api.GetRemoteWorkloadPairs()
    # get workloads from each node
    tc.w = []
    tc.w.append(pairs[0][0])
    tc.w.append(pairs[0][1])

    tc.devices = []
    tc.gid = []
    tc.ib_prefix = []
    for i in range(2):
        tc.devices.append(api.GetTestsuiteAttr(tc.w[i].ip_address+'_device'))
        tc.gid.append(api.GetTestsuiteAttr(tc.w[i].ip_address+'_gid'))
        if tc.w[i].IsNaples():
            tc.ib_prefix.append('cd ' + tc.iota_path + ' && ./run_rdma.sh ')
        else:
            tc.ib_prefix.append('')

    tc.stats_results = []

    return api.types.status.SUCCESS

def StatsCmds(tc, w, dev, counter):
    if w.IsNaples():
        cmds = {}
        if api.GetNodeOs(w.node_name) == host.OS_TYPE_LINUX:
            # set counter lifespan zero so it does not return an old value if the test runs quickly
            cmds['lifespan'] = 'echo 0 > /sys/class/infiniband/{dev}/ports/1/hw_counters/lifespan'
            cmds['counter'] = 'cat /sys/class/infiniband/{dev}/ports/1/hw_counters/{counter}'
        else:
            cmds['lifespan'] = 'sysctl sys.class.infiniband.{dev}.ports.1.hw_counters.lifespan=0'
            cmds['counter'] = 'sysctl sys.class.infiniband.{dev}.ports.1.hw_counters.{counter}'
        cmds['lifespan'] = cmds['lifespan'].format(dev=dev)
        cmds['counter'] = cmds['counter'].format(dev=dev, counter=counter)
        return cmds

def Trigger(tc):

    #==============================================================
    # trigger the commands
    #==============================================================
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    i = 0
    while (i < 2):
        j = (i + 1) % 2
        w1 = tc.w[i]
        w2 = tc.w[j]

        tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                       (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)

        api.Logger.info("Starting ibv_rc_pingpong test from %s" % (tc.cmd_descr))

        # stats dump before / after test
        w1_stats_cmds = StatsCmds(tc, w1, tc.devices[i], 'tx_rdma_ucast_pkts')
        w2_stats_cmds = StatsCmds(tc, w2, tc.devices[j], 'tx_rdma_ucast_pkts')
        w1_stats_results = []
        w2_stats_results = []

        if w1_stats_cmds is not None:
            api.Trigger_AddCommand(req,
                                   w1.node_name,
                                   w1.workload_name,
                                   w1_stats_cmds['lifespan'])
            cmd_ref = api.Trigger_AddCommand(req,
                                             w1.node_name,
                                             w1.workload_name,
                                             w1_stats_cmds['counter'])
            w1_stats_results.append(cmd_ref)

        if w2_stats_cmds is not None:
            api.Trigger_AddCommand(req,
                                   w2.node_name,
                                   w2.workload_name,
                                   w2_stats_cmds['lifespan'])
            cmd_ref = api.Trigger_AddCommand(req,
                                             w2.node_name,
                                             w2.workload_name,
                                             w2_stats_cmds['counter'])
            w2_stats_results.append(cmd_ref)

        # cmd for server
        cmd = "ibv_rc_pingpong -d " + tc.devices[i] + " -g " + tc.gid[i] + " -s 1024 -r 10 -n 10"
        api.Trigger_AddCommand(req, 
                               w1.node_name, 
                               w1.workload_name,
                               tc.ib_prefix[i] + cmd,
                               background = True)

        # On Naples-Mellanox setups, with Mellanox as server, it takes a few seconds before the server
        # starts listening. So sleep for a few seconds before trying to start the client
        cmd = 'sleep 2'
        api.Trigger_AddCommand(req, 
                               w1.node_name, 
                               w1.workload_name,
                               cmd)

        # cmd for client
        cmd = "ibv_rc_pingpong -d " + tc.devices[j] + " -g " + tc.gid[j] + " -s 1024 -r 10 -n 10 " + w1.ip_address
        api.Trigger_AddCommand(req, 
                               w2.node_name, 
                               w2.workload_name,
                               tc.ib_prefix[j] + cmd)

        # stats dump after test
        if w1_stats_cmds is not None:
            cmd_ref = api.Trigger_AddCommand(req,
                                             w1.node_name,
                                             w1.workload_name,
                                             w1_stats_cmds['counter'])
            w1_stats_results.append(cmd_ref)
            tc.stats_results.append(w1_stats_results)

        if w2_stats_cmds is not None:
            cmd_ref = api.Trigger_AddCommand(req,
                                             w2.node_name,
                                             w2.workload_name,
                                             w2_stats_cmds['counter'])
            w2_stats_results.append(cmd_ref)
            tc.stats_results.append(w2_stats_results)

        i = i + 1
    #end while

    # trigger the request
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    api.Logger.info("ibv_rc_pingpong results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE

    for cmd_refs in tc.stats_results:
        if cmd_refs[0].stdout == cmd_refs[1].stdout:
            api.Logger.info("counter value did not change %s -> %s" % (
                            cmd_refs[0].stdout, cmd_refs[1].stdout))
            result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

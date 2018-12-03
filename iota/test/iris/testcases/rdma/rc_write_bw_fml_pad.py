#! /usr/bin/python3
import iota.harness.api as api

def Setup(tc):
 
    tc.desc = '''
    Test  :   ib_write_bw
    Opcode:   Write First, Middle, Last
    Num QP:   1, 2
    Pad   :   Yes
    Inline:   No
    modes:    workload1 as server, workload2 as client
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
            tc.ib_prefix.append('cd ' + tc.iota_path + ' && ./run_rdma.sh  ')
        else:
            tc.ib_prefix.append('')

    return api.types.status.SUCCESS

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

        api.Logger.info("Starting ib_write_bw test from %s" % (tc.cmd_descr))

        # cmd for server
        cmd = "ib_write_bw -d " + tc.devices[i] + " -n 10 -F -x " + tc.gid[i] + " -m 4096 -s 12601 -q " + str(tc.iterators.num_qp) + " --report_gbits"
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
        cmd = "ib_write_bw -d " + tc.devices[j] + " -n 10 -F -x " + tc.gid[j] + " -m 4096 -s 12601 -q " + str(tc.iterators.num_qp) + " --report_gbits " + w1.ip_address
        api.Trigger_AddCommand(req, 
                               w2.node_name, 
                               w2.workload_name,
                               tc.ib_prefix[j] + cmd)

        i = i + 1
    # end while

    # trigger the request
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    api.Logger.info("ib_write_bw results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

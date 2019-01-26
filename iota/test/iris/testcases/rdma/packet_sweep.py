#! /usr/bin/python3
import iota.harness.api as api

def Setup(tc):
 
    tc.desc = '''
    Test  :   packet_sweep
    Opcode:   Send, Read, Write
    Num QP:   1, 2
    Transport: UD, RC
    MTU: Various sizes
    RDMA CM: Yes, No
    modes :   workload1 as server, workload2 as client
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

    if tc.iterators.rdma_cm == 'yes':
        cm_opt = " -R "
    else:
        cm_opt = " "

    if tc.iterators.transport == 'UD':
        transport_opt = " -c UD "
    else:
        transport_opt = " "

    if tc.iterators.server == 'yes':
        i = 0
        k = 1
    else :
        i = 1
        k = 2

    s_port = 12340
    e_port = s_port + tc.iterators.threads
    if (tc.iterators.threads > 1):
        client_bkg = True
    else:
        client_bkg = False

    while ((i < 2) and (i < k)):
        j = (i + 1) % 2
        w1 = tc.w[i]
        w2 = tc.w[j]

        tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                       (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)

        api.Logger.info("Starting ib_send_bw test from %s" % (tc.cmd_descr))

        # cmd for server
        for p in range(s_port, e_port):

            cmd = tc.iterators.command + " -d " + tc.devices[i] + " -n 10 -F -x " + tc.gid[i] + " -a -q " + str(tc.iterators.num_qp) + " -m " + str(tc.iterators.mtu) + cm_opt + transport_opt + " --report_gbits " + " -p {0}".format(p)
            api.Trigger_AddCommand(req, 
                                   w1.node_name, 
                                   w1.workload_name,
                                   tc.ib_prefix[i] + cmd,
                                   background=True, timeout=120)

        # On Naples-Mellanox setups, with Mellanox as server, it takes a few seconds before the server
        # starts listening. So sleep for a few seconds before trying to start the client
        cmd = 'sleep 2'
        api.Trigger_AddCommand(req,
                               w1.node_name,
                               w1.workload_name,
                               cmd)

        # cmd for client
        for p in range(s_port, e_port):
            cmd = tc.iterators.command + " -d " + tc.devices[j] + " -n 10 -F -x " + tc.gid[j] + " -a -q " + str(tc.iterators.num_qp) + " -m " + str(tc.iterators.mtu) + cm_opt + transport_opt + " --report_gbits " + w1.ip_address + " -p {0}".format(p)
            api.Trigger_AddCommand(req, 
                                   w2.node_name, 
                                   w2.workload_name,
                                   tc.ib_prefix[j] + cmd, 
                                   background=client_bkg, timeout=120)

        if client_bkg:
            # since the client is running in the background, sleep for 120 secs
            # to allow the test to complete before verifying the result
            cmd = 'sleep 120'
            api.Trigger_AddCommand(req,
                                   w1.node_name,
                                   w1.workload_name,
                                   cmd)

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

    api.Logger.info("packet_sweep results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE

        if api.Trigger_IsBackgroundCommand(cmd) and len(cmd.stderr) != 0:
            result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

#! /usr/bin/python3
import iota.harness.api as api

def Setup(tc):
 
    tc.desc = '''
    Test  :   multi_sge
    Opcode:   Send
    Num QP:   1, 2
    Transport: RC
    MTU: Various sizes
    RDMA CM: Yes, No
    modes :   workload1 as server, workload2 as client
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

    #==============================================================
    # init cmd options
    #==============================================================
    iter_opt      = ' -D 20 '
    misc_opt      = ' -F --report_gbits '
    cm_opt        = ''
    transport_opt = ''
    size_opt      = ' -s 10000 '
    mtu_opt       = ' -m 4096 '
    qp_opt        = ' -q 1 '
    numsges_opt   = ''
    tc.client_bkg = False
    s_port        = 12340
    e_port        = s_port + 1

    #==============================================================
    # update non-default cmd options
    #==============================================================
    if hasattr(tc.iterators, 'rdma_cm') and tc.iterators.rdma_cm == 'yes':
        cm_opt = ' -R '

    if hasattr(tc.iterators, 'transport') and tc.iterators.transport == 'UD':
        transport_opt = ' -c UD '

    if hasattr(tc.iterators, 'size'):
        size_opt  =  ' -s {size} '.format(size = tc.iterators.size)

    if hasattr(tc.iterators, 'mtu'):
        mtu_opt = ' -m {mtu} '.format(mtu = tc.iterators.mtu)

    if hasattr(tc.iterators, 'numsges'):
        numsges_opt  =  ' -W {numsges} '.format(numsges = tc.iterators.numsges)

    if hasattr(tc.iterators, 'num_qp'):
        num_qp = ' -q {numqp} '.format(numqp = str(tc.iterators.num_qp))

    if hasattr(tc.iterators, 'threads') and tc.iterators.threads > 1:
        tc.client_bkg = True
        e_port = s_port + tc.iterators.threads

    #==============================================================
    # run the cmds
    #==============================================================
    w1 = tc.w[0]
    w2 = tc.w[1]

    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                    (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)

    api.Logger.info("Starting ib_send_bw multi_sge test from %s" % (tc.cmd_descr))

    #==============================================================
    # cmd for server
    #==============================================================
    for p in range(s_port, e_port):

        port_opt  = ' -p {port} '.format(port = p)
        dev_opt   = ' -d {dev} '.format(dev = tc.devices[0])
        gid_opt   = ' -x {gid} '.format(gid = tc.gid[0])

        cmd       = tc.iterators.command
        cmd       += dev_opt + iter_opt + gid_opt 
        cmd       += size_opt + mtu_opt + qp_opt
        cmd       += cm_opt + transport_opt + misc_opt + port_opt
        # add numsges_opt only for Naples
        if w1.IsNaples():
            cmd   += numsges_opt

        api.Trigger_AddCommand(req, 
                               w1.node_name, 
                               w1.workload_name,
                               tc.ib_prefix[0] + cmd,
                               background=True, timeout=120)

        # On Naples-Mellanox setups, with Mellanox as server, it takes a few seconds before the server
        # starts listening. So sleep for a few seconds before trying to start the client
        cmd = 'sleep 2'
        api.Trigger_AddCommand(req,
                               w1.node_name,
                               w1.workload_name,
                               cmd)

    #==============================================================
    # cmd for client
    #==============================================================
    for p in range(s_port, e_port):

        port_opt  = ' -p {port} '.format(port = p)
        dev_opt   = ' -d {dev} '.format(dev = tc.devices[1])
        gid_opt   = ' -x {gid} '.format(gid = tc.gid[1])

        cmd       = tc.iterators.command
        cmd       += dev_opt + iter_opt + gid_opt 
        cmd       += size_opt + mtu_opt + qp_opt
        cmd       += cm_opt + transport_opt + misc_opt + port_opt
        # add numsges_opt only for Naples
        if w2.IsNaples():
            cmd   += numsges_opt
        # append server's ip_address 
        cmd       += w1.ip_address

        api.Trigger_AddCommand(req, 
                               w2.node_name, 
                               w2.workload_name,
                               tc.ib_prefix[1] + cmd, 
                               background=tc.client_bkg, timeout=125) #5 secs more than def test timeout=120

        if tc.client_bkg:
            # since the client is running in the background, sleep for 130 secs
            # to allow the test to complete before verifying the result
            #override default timeout to 135 slightly above the test duration 130 secs
            cmd = 'sleep 130'
            api.Trigger_AddCommand(req,
                                   w1.node_name,
                                   w1.workload_name,
                                   cmd, timeout = 135)

    #==============================================================
    # trigger the request
    #==============================================================
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    api.Logger.info("multi_sge results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE

        if tc.client_bkg and api.Trigger_IsBackgroundCommand(cmd) and len(cmd.stderr) != 0:
            result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS
